#include <client.h>

int main (void)
{
	SetupHardware();
	
	for(;;){
		runNet();
	}
	
	return 0;
}

void SetupHardware(void)
{	
	LED_Init();
	
	_delay_ms(100);
	SPI_Init();
	
	_delay_ms(1000);
	NET_Init();
}

unsigned char Connect(unsigned char sock, unsigned char ip[4] ,unsigned char eth_protocol, unsigned int tcp_port) {
	unsigned char			retval;
	unsigned int			sockaddr;
	
	retval = W5100_FAIL;	
																																				// assume this doesn't work
	if (sock >= W5100_NUM_SOCKETS)  {
		return retval;																											// illegal socket value is bad!
	}
	
	sockaddr =  W5100_SKT_BASE(sock);																			// calc base addr for this socket

	if (W51_read(sockaddr+W5100_SR_OFFSET) == W5100_SKT_SR_CLOSED)				// Make sure we close the socket first
	{
		Close(sock);
	}

	// W51_write(sockaddr+W5100_MR_OFFSET ,0b01000000);														//  Enable Mac Filtering
	W51_write(sockaddr+W5100_MR_OFFSET ,eth_protocol);										//  set protocol for this socket
	
	W51_write(sockaddr+W5100_DIPR_OFFSET,     ip[0]);											//  set ip for server
	W51_write(sockaddr+W5100_DIPR_OFFSET + 1, ip[1]);											//  set ip for server
	W51_write(sockaddr+W5100_DIPR_OFFSET + 2, ip[2]);											//  set ip for server
	W51_write(sockaddr+W5100_DIPR_OFFSET + 3, ip[3]);											//  set ip for server
	
	W51_write(sockaddr+W5100_DPORT_OFFSET, ((tcp_port & 0xFF00) >> 8 ));	// set port for server (MSB)
	W51_write(sockaddr+W5100_DPORT_OFFSET + 1, (tcp_port & 0x00FF));			// set port for server (LSB)
	
	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_OPEN);	              // open the socket
	
	while (W51_read(sockaddr+W5100_CR_OFFSET))  ;													// loop until device reports socket is open (blocks!!)

	if (W51_read(sockaddr+W5100_SR_OFFSET) == W5100_SKT_SR_INIT){
		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_CONNECT);					// connect to server
		while (W51_read(sockaddr+W5100_CR_OFFSET))  ;												// loop until device reports socket is open (blocks!!)
		retval = sock;																											// if success, return socket number
	} else  Close(sock);																									// if failed, close socket immediately

	return  retval;
}

void  Close(unsigned char  sock)
{
	unsigned int			sockaddr;
	
	if (sock > W5100_NUM_SOCKETS)  {		
		return;																															// if illegal socket number, ignore request
	}
	sockaddr = W5100_SKT_BASE(sock);																			// calc base addr for this socket

	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_CLOSE);							// tell chip to close the socket
	while (W51_read(sockaddr+W5100_CR_OFFSET))  ;													// loop until socket is closed (blocks!!)
}

void  Disconnect(unsigned char  sock)
{
	unsigned int			sockaddr;
	
	if (sock > W5100_NUM_SOCKETS)  {
		return;																															// if illegal socket number, ignore request
	}
	sockaddr = W5100_SKT_BASE(sock);																			// calc base addr for this socket

	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_DISCON);							// disconnect the socket
	while (W51_read(sockaddr+W5100_CR_OFFSET))  ;													// loop until socket is closed (blocks!!)
}

unsigned char Send(unsigned char  sock, const unsigned char  *buf, unsigned int  buflen)
{
	unsigned int					ptr;
	unsigned int					offaddr;
	unsigned int					realaddr;
	unsigned int					txsize;
	unsigned int					timeout;
	unsigned int					sockaddr;

	if (buflen == 0 || sock >= W5100_NUM_SOCKETS)  return  W5100_FAIL;		// ignore illegal requests
	sockaddr = W5100_SKT_BASE(sock);																			// calc base addr for this socket
	// Make sure the TX Free Size Register is available
	txsize = W51_read(sockaddr+W5100_TX_FSR_OFFSET);											// make sure the TX free-size reg is available
	txsize = (((txsize & 0x00FF) << 8 ) + W51_read(sockaddr+W5100_TX_FSR_OFFSET + 1));

	timeout = 0;
	while (txsize < buflen)
	{
		_delay_ms(1);

		txsize = W51_read(sockaddr+W5100_TX_FSR_OFFSET);										// make sure the TX free-size reg is available
		txsize = (((txsize & 0x00FF) << 8 ) + W51_read(sockaddr+W5100_TX_FSR_OFFSET + 1));

		if (timeout++ > 2048) 																							// if max delay has passed...
		{
			#ifdef DBG
			printf("Send() - Timeout expired\n");
			#endif
			
			Disconnect(sock);																						// can't connect, close it down
			return  W5100_FAIL;																								// show failure
		}
	}

	// Read the TX Write Pointer
	ptr = W51_read(sockaddr+W5100_TX_WR_OFFSET);
	offaddr = (((ptr & 0x00FF) << 8 ) + W51_read(sockaddr+W5100_TX_WR_OFFSET + 1));

	while (buflen)
	{
		buflen--;
		realaddr = (W5100_TXBUFADDR + (0x0800 * sock)) + (offaddr & W5100_TX_BUF_MASK);        // calc W5100 physical buffer addr for this socket

		W51_write(realaddr, *buf);																				// send a byte of application data to TX buffer
		offaddr++;																												// next TX buffer addr
		buf++;																														// next input buffer addr
	}

	W51_write(sockaddr+W5100_TX_WR_OFFSET, (offaddr & 0xFF00) >> 8);		// send MSB of new write-pointer addr
	W51_write(sockaddr+W5100_TX_WR_OFFSET + 1, (offaddr & 0x00FF));			// send LSB

	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_SEND);							// start the send on its way
	while (W51_read(sockaddr+W5100_CR_OFFSET))  ;												// loop until socket starts the send (blocks!!)

	return  W5100_OK;
}

unsigned int Receive(unsigned char sock, unsigned char  *buf, unsigned int  buflen)
{
	unsigned int	ptr;
	unsigned int	cnt;
	unsigned int	offaddr;
	unsigned int	realaddr;
	unsigned int	sockaddr;
	uint8_t				data[24];
	
	if (sock >= W5100_NUM_SOCKETS)  return  W5100_FAIL;		// ignore illegal conditions

	if (buflen > (MAX_BUF - 2))  {
		buflen = MAX_BUF - 2;											// requests that exceed the max are truncated
	}


	sockaddr = W5100_SKT_BASE(sock);																									// calc base addr for this socket
	ptr = W51_read(sockaddr+W5100_RX_RD_OFFSET);																			// get the RX read pointer (MSB)
	offaddr = (((ptr & 0x00FF) << 8 ) + W51_read(sockaddr+W5100_RX_RD_OFFSET + 1));		// get LSB and calc offset addr
	
	//_delay_ms(1);
	
	cnt = 0;
	while (buflen)
	{
		buflen--;
		realaddr = (W5100_RXBUFADDR + (MAX_MEM * sock)) + (offaddr & W5100_RX_BUF_MASK);
		data[cnt] = W51_read(realaddr);
		
		cnt++;
		offaddr++;
	}
	
	Send(crntSocket, netBuffer, strlen((char *)netBuffer));
	
	#ifdef DBG
	printf("\n");
	#endif
	
	// Increase the S0_RX_RD value, so it point to the next receive
	W51_write(sockaddr+W5100_RX_RD_OFFSET, (offaddr & 0xFF00) >> 8);			// update RX read offset (MSB)
	W51_write(sockaddr+W5100_RX_RD_OFFSET + 1,(offaddr & 0x00FF));				// update LSB

	// Now Send the RECV command
	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_RECV);								// issue the receive command
	_delay_us(10);																												// wait for receive to start

	return  W5100_OK;
}

unsigned int ReceivedSize(unsigned char  sock)
{
	unsigned int					val;
	unsigned int					sockaddr;

	if (sock >= W5100_NUM_SOCKETS)  return  0;
	sockaddr = W5100_SKT_BASE(sock);						// calc base addr for this socket
	val = W51_read(sockaddr+W5100_RX_RSR_OFFSET) & 0xff;
	val = (val << 8) + W51_read(sockaddr+W5100_RX_RSR_OFFSET + 1);
	return  val;
}

void selectEthernet(void)
{
	/*
	*  Simple wrapper function for selecting the W5100 device.  This function
	*  allows the library code to invoke a target-specific function for enabling
	*  the W5100 chip.
	*/
	W51_ENABLE;
}

void deselectEthernet(void)
{
	/*
	*  Simple wrapper function for de-selecting the W5100 device.  This function
	*  allows the library code to invoke a target-specific function for disabling
	*  the W5100 chip.
	*/
	
	W51_DISABLE;
}

unsigned char xchgData(unsigned char  val)
{
	/*
	*  my_xchg      callback function; exchanges a byte with W5100 chip
	*/
	SPDR = val;
	while  (!(SPSR & (1<<SPIF)))  ;
	return  SPDR;
}

void resetEthernet(void)
{
	/*
	*  my_reset      callback function; force a hardware reset of the W5100 device
	*/
	SPI_DDR |= (1<<RESET_BIT);
	SPI_PORT |= (1<<RESET_BIT);
	
	SPI_PORT &= ~(1<<RESET_BIT);																		//  pull the line low
	_delay_ms(10);																									//  let the device reset
	SPI_PORT |= (1<<RESET_BIT);																			//  done with reset, pull the line high
	_delay_ms(100);																									//  let the chip wake up
}

void toggleLed(uint8_t led) {
	switch(led){
		case 0: LED1_TG; break;
		case 1: LED2_TG; break;
		case 2: LED3_TG; break;
		case 3: LED4_TG; break;
		case 4: LED5_TG; break;
		case 5: LED6_TG; break;
		case 6: LED7_TG; break;
		case 7: LED8_TG; break;
		default: break;
	}
}


void LED_Init(void) {
	LED1_SET;
	LED2_SET;
	LED3_SET;
	LED4_SET;
	LED5_SET;
	LED6_SET;
	LED7_SET;
	LED8_SET;
	
	for(uint8_t a = 0; a < MAX_LEDS; a++){
		toggleLed(a);
		_delay_ms(500);
	}
}

void SPI_Init(void){
	
	// Set MOSI (PORTB2),SCK (PORTB1) and PORTB0 (SS) as output, others as input
	SPI_DDR = (1<<SPI_MOSI)|(1<<SPI_SCLK)|(1<<SPI_CS);

	// CS pin is not active - Set to High
	SPI_PORT |= (1<<SPI_CS);

	// Enable SPI, Master Mode 0, set the clock rate fck/2
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);
	SPSR |= (1<<SPI2X);

	///////////////////////////////////////////////////////////////////////////////////////////////////
}

void NET_Init(void){
	crntSocket = 0;																// magic number! declare the socket number we will use (0-3)
	sockaddr = W5100_SKT_BASE(crntSocket);				// calc address of W5100 register set for this socket
	
	cb._select = &selectEthernet;									// callback for selecting the W5100
	cb._xchg = &xchgData;													// callback for exchanging data
	cb._deselect = &deselectEthernet;							// callback for de-selecting the W5100
	cb._reset = &resetEthernet;										// callback for hardware-reset of the W5100
	
	W51_register(&cb);														// register our target-specific W5100 routines with the W5100 library
	W51_init();																		// now initialize the W5100

	W51_config(&ipConfig);												// config the W5100 (MAC, TCP address, subnet, etc
}

void runNet(void){
	switch  (W51_read(sockaddr+W5100_SR_OFFSET))		// based on current status of socket...
	{
		case  W5100_SKT_SR_CLOSED:						// if socket is closed...
			if (Connect(crntSocket, srvrIp, W5100_SKT_MR_TCP, SRVR_PORT) == crntSocket)		// if successful opening a socket...
			{
				_delay_ms(1);
				} else {
			}
			break;
			
		case  W5100_SKT_SR_INIT: {
			break;
		}
		
		case  W5100_SKT_SR_ESTABLISHED:					// if socket connection is established...
			rsize = ReceivedSize(crntSocket);					// find out how many bytes
			if (rsize >= 1) {
				if (Receive(crntSocket, netBuffer, rsize) != W5100_OK)  {
					strcpy_P((char *)netBuffer, PSTR("Z"));
				}
			
				if (Send(crntSocket, netBuffer, strlen((char *)netBuffer)) == W5100_FAIL)  break;		// just throw out the packet for now
			}
			else											// no data yet...
			{
				_delay_us(10);
			}
			break;

		case  W5100_SKT_SR_FIN_WAIT:
		case  W5100_SKT_SR_CLOSING:
		case  W5100_SKT_SR_TIME_WAIT:
		case  W5100_SKT_SR_CLOSE_WAIT:
		case  W5100_SKT_SR_LAST_ACK: {
			Close(crntSocket);
			break;
		}
	}
}