#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <stdexcept>
#include <string>

#include "Socket.h"


namespace http {

/* Socket not connected exception */
class SocketNotConnected : public std::runtime_error {
public:
	SocketNotConnected(const std::string& what) : std::runtime_error(what) {}
};

/**
* TCP socket class, inherits from socket.
* Connection based socket which uses the linux socket API.
*/
class tcpSocket : public Socket {
private:
	/* Flags saying if the socket is in connected mode or listening mode */
	bool connected;
	bool listening;

	/* SetSockId to use within accept to set tcpSocket to an existing socket */
	void SetSockId(int sock_id);

	/* Enables keep-alive for accepted socket */
	bool EnableKeepAlive();

public:
	/*--C'tors--*/
	tcpSocket(int port, IPv ip_version) : Socket::Socket(port, Socket::TCP, ip_version), connected(false), listening(false){ }
	tcpSocket(int port) : Socket::Socket(port, Socket::TCP, Socket::UNSPECIFIED), connected(false), listening(false){ }
	tcpSocket(const tcpSocket& copy);
	tcpSocket() : Socket::Socket(), connected(false), listening(false) { };
	
	virtual ~tcpSocket() = default;

	/**
	* Sets the socket on listen mode.
	* Listen mode waits for incoming connections and when
	*  received they stay on a queue until accepted.
	**
	* max_connections - maximum number of connections to get at a time.
	* Return value: success value
	*/
	bool Listen(int max_connections);
	/**
	* Accept pending connections from listening socket.
	**
	* new_socket - refrence to tcpSocket to have accepted socket placed in
	* Return value: success value
	*/
	bool Accept(tcpSocket& new_socket);

	bool isConnected() const;
	
	bool isListening() const;


	/**
	* Sends message, sent message length is set as msg.length()
	* Return value: number of bytes received, or -1 on error.
	*/
	int Send(const std::string& msg) const;
	/**
	* Length specifies the largest amount of bytes willing to receive
	* Message received will be placed in msg
	* Return value: number of bytes received, or 0 on closed connection, or -1 on error.
	*/
	int Recv(std::string& msg, int length) const;

	/**
	* Connect to a host address.
	* host - IP address or web address; e.g "192.168.1.1" or "www.google.com"
	* port - Port to connect to; e.g "443"
	* Return value: success value
	*/
	bool Connect(const std::string& host, const std::string& port);

};

}

#endif