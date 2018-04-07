#ifndef SERVER_H
#define SERVER_H

#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <string>

#include "../Socket/Socket_Set.h"
#include "../Socket/SocketIncludes.h"
#include "../Socket/TCPSocket.h"


namespace http {

class server {
private:
	/**
	* Verbose flag,
	* if true, print errors and log,
	* if false, dont print anything
	*/
	bool verbose;
	/**
	* Continue Serving flag,
	* when false, shut down server.
	* Flag true on start and set by GetInput().
	*/
	bool continue_serving;
	/**
	* Main listening socket.
	* Lifespan from call to server::Serve() until closing of server
	*/
	tcpSocket MainSocket;
	/**
	* Maximum connections to have at any given time.
	* Any connection requested after max_connections
	* are connected will be denied and ignored.
	*/
	int max_connections;

	/**
	* SetUpServer - Sets up server, gets MainSocket up and listening
	* Return value: success value
	*/
	bool SetUpServer();

	/**
	* Thread that handles connections.
	*/
	virtual void HandleConnections();
	/**
	* Handles messages received from sockets.
	* msg    - Message sent from socket.
	* socket - Socket who sent the message.
	*/
	virtual void HandleMessage(const std::string& msg, const tcpSocket& socket);

	/* Out stream write lock */
	std::mutex out_stream_lock;
	/* Connections management lock */
	std::mutex connections_lock;

protected:
	/**
	* Thread safe log, print if verbose is true.
	* msg    - Message to write.
	* stream - Stream to write to. Default: Standard Output.
	*/
	void ThreadSafeLog(const std::string& msg, std::ostream& stream=std::cout);	
	/**
	* Thread safe log, print wether verbose is true or false.
	* msg    - Message to write.
	* stream - Stream to write to. Default: Standard Output.
	*/
	void ThreadSafeLogPrintAlways(const std::string& msg, std::ostream& stream=std::cout);
	Socket_Set<tcpSocket> master_set;

public:
	/**
	* server C'tor
	* port            - port number to set server on
	* max_connections - maximim connections to handle at one time
	* verbose         - if true, print errors and log, if false, dont print anything
	*/
	server(int port, int max_connections, bool verbose = true);
	/* Server D'tor */
	virtual ~server();

	/**
	* Main function, infinite loop of serving.
	*/
	bool Serve();

};

}

#endif