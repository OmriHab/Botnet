#ifndef SERVER_H
#define SERVER_H

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

#include "../Socket/Socket_Set.h"
#include "../Socket/SocketIncludes.h"
#include "../Socket/SecureSocket.h"


namespace botnet {

class server {
protected:
	typedef std::map<std::string, std::string> StringMap;

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
	SecureSocket MainSocket;
	/**
	* Maximum connections to have at any given time.
	* Any connection requested after max_connections
	* are connected will be denied and ignored.
	*/
	int max_connections;
	
	/**
	* Master set of all sockets, including the listening socket, MainSocket
	*/
	Socket_Set<SecureSocket> master_set;
	/** 
	* Connections management lock, lock whenever wishing to access master_set
	*/
	std::mutex master_set_lock;

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
	virtual void HandleMessage(const std::string& msg, const SecureSocket& socket);

	/**
	* Thread safe log, print if verbose is true. Uses out_stream_lock.
	* msg    - Message to write.
	* stream - Stream to write to. Default: Standard Output.
	*/
	void ThreadSafeLog(const std::string& msg, std::ostream& stream=std::cout);	
	/**
	* Thread safe log, print wether verbose is true or false. Uses out_stream_lock.
	* msg    - Message to write.
	* stream - Stream to write to. Default: Standard Output.
	*/
	void ThreadSafeLogPrintAlways(const std::string& msg, std::ostream& stream=std::cout);
	/**
	* Thread safe getline. Uses in_stream_lock.
	* stream - Stream to read from. Default: Standard input.
	*/
	std::string ThreadSafeGetLine(std::istream& stream=std::cin);


	/**
	* Out and in stream locks.
	*/
	std::mutex out_stream_lock;
	std::mutex in_stream_lock;

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