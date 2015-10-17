/*
 * client.h
 *
 * Created: 10/16/2015 5:26:48 PM
 *  Author: Jairo Martinez
 */ 


#ifndef CLIENT_H_
#define CLIENT_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "EasyAVR128.h"
#include "w5100/w5100.h"

#define MAX_BUF		512				//  largest buffer we can read from chip 
#define SRVR_PORT	1880			// TCP port for HTTP 

#define MAX_MEM		0x0800		//  For 8K Buffer -- 0x1000 for 4K -- 0x0800 for 2K

/*
*  Define macros for selecting and de-selecting the W5100 device.
*/

#define  W51_ENABLE		SPI_PORT&=~(1<<SPI_CS)
#define  W51_DISABLE	SPI_PORT|=(1<<SPI_CS)

W5100_CALLBACKS		cb;

// Network Vars
unsigned char			netBuffer[MAX_BUF];
unsigned int			sockaddr;
unsigned char			crntSocket;
unsigned int			rsize;
unsigned char			srvrIp[4] = {172,16,76,107};

W5100_CFG					ipConfig =
{
	{0x00,0x08,0xDC,0x00,0x00,0x0A},		// mac_addr
	{172,16,76,90},											// ip_addr
	{255,255,255,0},										// sub_mask
	{172,16,76,1}												// gtw_addr
};

void							SetupHardware(void);
void							toggleLed(uint8_t led);
void							LED_Init(void);
void							SPI_Init(void);
void							NET_Init(void);
void							runNet(void);

void							selectEthernet(void);
void							deselectEthernet(void);
unsigned char			xchgData(unsigned char  val);
void							resetEthernet(void);

unsigned char			Connect(unsigned char sock, unsigned char ip[] ,unsigned char eth_protocol, unsigned int tcp_port);
void							Close(unsigned char  sock);
void							Disconnect(unsigned char  sock);
unsigned char			Send(unsigned char  sock, const unsigned char  *buf, unsigned int  buflen);
unsigned int			Receive(unsigned char  sock, unsigned char  *buf, unsigned int  buflen);
unsigned int			ReceivedSize(unsigned char  sock);

#endif /* CLIENT_H_ */