#include <iostream>
#include <string.h>

#include "TCPSocket.h"
#include "SocketIncludes.h"


using namespace http;


tcpSocket::tcpSocket(const tcpSocket& copy)
	: Socket::Socket(copy)
	, connected(copy.connected)
	,listening(copy.listening)
	{ }

bool tcpSocket::Listen(int max_connections) {
	int ret_val = listen(this->GetSockId(), max_connections);
	if (ret_val == -1) {
		return false;
	}

	this->listening = true;
	return true;
}

void tcpSocket::SetSockId(int sock_id) {
	Socket::SetSockId(sock_id);
}

bool tcpSocket::Accept(tcpSocket& new_socket) {
	/*--Vars--*/
	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof(their_addr);
	int accepted_socket;

	// Accept socket
	accepted_socket = accept(this->GetSockId(), (struct sockaddr*)&their_addr, &addr_size);
		
	// If failed to accept
	if (accepted_socket == -1) {
		return false;
	}

	new_socket.SetSockId(accepted_socket);
	new_socket.connected = true;

	return this->EnableKeepAlive();
}

bool tcpSocket::isConnected() const {
	return this->connected;
}

bool tcpSocket::isListening() const {
	if (this->listening) {
		int val;
		socklen_t len = sizeof(val);
		if (getsockopt(this->GetSockId(), SOL_SOCKET, SO_ACCEPTCONN, &val, &len) == -1) {
		    return false;
		}
		return (val != 0);
	}
	return false;
}

int tcpSocket::Send(const std::string& msg) const {
	/* Socket must be connected before sending a message using send */
	if (!this->connected) {
		throw SocketNotConnected("tcpSocket::Send(msg): error, socket not connected");
	}
	static const int FLAGS = 0;
	
	int left_to_send        = msg.length();
	int bytes_sent          = 0;
	std::string msg_to_send = msg;

	while (left_to_send > 0){
		bytes_sent = send(this->GetSockId(), msg_to_send.c_str(), msg_to_send.length(), FLAGS);
		
		// On send malfunction return how much was sent until now
		if (bytes_sent == -1) {
			msg.length() - left_to_send;
		}

		left_to_send -= bytes_sent;
		msg_to_send   = msg_to_send.substr(bytes_sent);
	}
}

int tcpSocket::Recv(std::string& msg, int length) const {
	/* Socket must be connected before sending a message using send */
	if (!this->connected) {
		throw SocketNotConnected("tcpSocket::recv(msg, length): error, socket " + std::to_string(this->GetSockId()) + " not connected");
	}
	char* message = new char[length];
	static const int FLAGS = 0;
	int bytes_read = 0;;

	/* Receive the message and copy string to msg */
	try {
		bytes_read = recv(this->GetSockId(), message, length, FLAGS);
		if (bytes_read <= 0) {
			if (message != nullptr){
				delete[] message;
				message = nullptr;
			}
			return bytes_read;
	}

		// Size msg as bytes_read and copy message to it
		msg.resize(bytes_read);
		for (int i = 0; i < bytes_read; i++) {
			msg[i] = message[i];
		}
	}
	catch (const std::exception& e) {
		if (message != nullptr){
			delete[] message;
			message = nullptr;
		}
		throw;
	}

	return bytes_read;
}

bool tcpSocket::Connect(const std::string& host, const std::string& port) {
	struct addrinfo hints, *results, *pResults;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_UNSPEC;		// Support both IPv4 and IPv6
	hints.ai_socktype = SOCK_STREAM;	// Use TCP
	hints.ai_flags    = AI_PASSIVE;		// Enter my ip address

	int rv;

	if ((rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &results)) != 0) {
		throw std::runtime_error("tcpSocket::Connect(host,port): getaddrinfo error, " + std::string(strerror(rv)));
	}

	for (pResults = results; pResults != nullptr; pResults = pResults->ai_next) {
		// Try to connect on each address found
		if (connect(this->GetSockId(), pResults->ai_addr, pResults->ai_addrlen) == -1) {
			continue;
		}
		break;
	}

	// Free the results list
	freeaddrinfo(results);

	// If reached the end without breaking, nothing was found.
	if (pResults == nullptr) {
		throw std::runtime_error("tcpSocket::Connect(host,port): Unable to connect to " + host + ":" + port);
	}
}

bool tcpSocket::EnableKeepAlive() {
	int keep_alive = 1;		// Enable keep alive
	int idle       = 300;	// Number of idle seconds before sending a KA probe
	int interval   = 10;	// How often in seconds to resend an unacked KA probe
	int count      = 5;		// How many times to resend a KA probe if previous probe was unacked


	if (setsockopt(this->GetSockId(), SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive)) < 0) {
		return false;
	}

	/* Set the number of seconds the connection must be idle before sending a KA probe. */
	if (setsockopt(this->GetSockId(), IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle)) < 0) {
		return false;
	}

	/* Set how often in seconds to resend an unacked KA probe. */
	if (setsockopt(this->GetSockId(), IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0) {
		return false;
	}

	/* Set how many times to resend a KA probe if previous probe was unacked. */
	if (setsockopt(this->GetSockId(), IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count)) < 0) {
		return false;
	}
	return true;
}