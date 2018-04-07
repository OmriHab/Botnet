#include <iostream>
#include <stdexcept>
#include <string.h>

#include "SocketIncludes.h"
#include "Socket.h"


using namespace http;

Socket::Socket(int port, Type protocol, IPv ip_version) {
	/*--Vars--*/
	struct addrinfo hints, *results, *pResults;
	socklen_t addr_size;
	static const int MAX_PORT = 65535;

	/* Check for legal port number */
	if(port > MAX_PORT || port < 0) {
		throw std::runtime_error("Port must be in range of 0-" + std::to_string(MAX_PORT));
	}

	/* Load up hints struct with info */
	LoadHints(hints, protocol, ip_version);

	if (getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &results) == -1) {
		throw std::runtime_error("getaddrinfo error");
	}

	/*--Try to get a socket for all results--*/
	for (pResults = results; pResults != nullptr; pResults = pResults->ai_next) {
		this->sock_id = socket(pResults->ai_family, pResults->ai_socktype, pResults->ai_protocol);

		if (this->sock_id == -1) {
			continue;
		}
		
		// Save socket address data
		this->self_address = new struct sockaddr;
		this->copy_address(this->self_address, pResults->ai_addr);

		break;
	}

	// Free results
	freeaddrinfo(results);

	// Throw error if failed to create socket on desired port
	if (pResults == nullptr) {
		throw std::runtime_error("Failed to make socket on port " + std::to_string(port));
	}
}

/* Call first c'tor with IPv "UNSPECIFIED" */
Socket::Socket(int port, Type protocol) : Socket::Socket(port, protocol, UNSPECIFIED) { }

Socket::Socket(int sock_id) : sock_id(sock_id) {
	socklen_t self_address_length = sizeof(this->self_address);

	// Alocate memmory for the struct sockaddr
	this->self_address = new struct sockaddr;
	// Get self_address from socket id
	if (getsockname(sock_id, this->self_address, &self_address_length) == -1) {
		throw std::runtime_error("Socket::Socket(sock_id): error getting socket name " + std::string(strerror(errno)));
	}
}

Socket::Socket() : sock_id(0), self_address(nullptr) { }

Socket::Socket(const Socket& copy) {
	this->sock_id = copy.sock_id;
	// Allocate space for self_address and let copy_address fill it
	this->self_address = new struct sockaddr;
	copy_address(this->self_address, copy.self_address);
}

Socket::~Socket() {
	// Delete self_address
	if (this->self_address != nullptr) {
		delete this->self_address;
		this->self_address = nullptr;
	}
}

bool Socket::operator==(const Socket& other) const {
	// Compares sock_id
	return (this->sock_id == other.GetSockId());
}

bool Socket::operator!=(const Socket& other) const {
	// Compares sock_id
	return (this->sock_id != other.GetSockId());
}


void Socket::SetSockId(int sock_id) {
	this->sock_id = sock_id;

	socklen_t self_address_length = sizeof(*this->self_address);

	// Allocate memmory for the struct sockaddr
	this->self_address = new struct sockaddr; 
	// Get self_address from socket id
	if (getsockname(sock_id, this->self_address, &self_address_length) == -1) {
		throw std::runtime_error("Socket::SetSockId(sock_id): error getting socket name " + std::string(strerror(errno)));
	}

}


int Socket::GetSockId() const {
	return this->sock_id;
}

int Socket::GetPort() const {
	in_port_t port = 0;
	// Use static casting to get port
	if (this->self_address->sa_family == AF_INET) {
		port = ((struct sockaddr_in*)this->self_address)->sin_port;
	}
	else {
		port = ((struct sockaddr_in6*)this->self_address)->sin6_port;
	}

	return ntohs(port);
}

Socket::IPv Socket::GetIpVersion() const {
	switch (this->self_address->sa_family) {
		case AF_INET:
			return IPv4;
		case AF_INET6:
			return IPv6;
		case AF_UNSPEC:
			return UNSPECIFIED;
	}
	return UNSPECIFIED;
}

bool Socket::Bind() {
	int rv = bind(this->sock_id,
				  this->self_address,
				  sizeof(struct sockaddr));
	return (rv != -1);
}

void Socket::Close() const {
	close(this->sock_id);
}

bool Socket::CanRead() const {
	fd_set read_set;

	/* Set time as 0 to return immediately*/
	struct timeval return_immediately = { 0 };


	/* Set read set to hold this->sock_id */
	FD_ZERO(&read_set);
	FD_SET(this->GetSockId(), &read_set);

	int retval = select(this->GetSockId()+1, &read_set, nullptr, nullptr, &return_immediately);

	if (retval == -1) {
		throw std::runtime_error("Socket::CanRead(): error using select()");
	}
	// Return if able to read from socket
	return (retval != 0);
}

bool Socket::CanWrite() const {
	fd_set write_set;

	/* Set time as 0 to return immediately */
	struct timeval return_immediately = { 0 };


	/* Set write set to hold this->sock_id */
	FD_ZERO(&write_set);
	FD_SET(this->GetSockId(), &write_set);

	int retval = select(this->GetSockId()+1, nullptr, &write_set, nullptr, &return_immediately);

	if (retval == -1) {
		throw std::runtime_error("Socket::CanWrite(): error using select()");
	}

	// Return if able to write from socket
	return (retval != 0);

}

bool Socket::IsConnected() const {
	char dummy_char = 0;
	int tries  = 0;
	int retval = 0;
	// Peek at the message to see socket status by return value
	do {
		retval = recv(this->GetSockId(), &dummy_char, 1, MSG_DONTWAIT|MSG_PEEK);
		tries++;
		if (tries == 3) {
			break;
		}
	} while (errno != EINTR); // If the receive was interupted, try maximum 3 times

	return retval > 0;

}

void Socket::copy_address(sockaddr* destination, const sockaddr* source) {
	if (destination == nullptr || source == nullptr) {
		throw std::invalid_argument("Socket::copy_address(destination, source): Both arguments must be non-nullptr");
	}

	static const int DATA_SIZE = 14;

	destination->sa_family = source->sa_family;

	for (int i = 0; i < DATA_SIZE; ++i) {
		destination->sa_data[i] = source->sa_data[i];
	}
	// Copy data array from source to destination
	//std::copy(source->sa_data, source->sa_data + DATA_SIZE, destination->sa_data);
}


void Socket::LoadHints(struct addrinfo &hints, Type protocol, IPv ip_version) {
	/* Clean hints struct */
	memset(&hints, 0, sizeof(hints));
	
	/*--Set ip version--*/
	switch(ip_version){
		case IPv4:
			hints.ai_family = AF_INET;
			break;
		case IPv6:
			hints.ai_family = AF_INET6;
			break;
		case UNSPECIFIED:
			hints.ai_family = AF_UNSPEC;
			break;
		default:
			throw std::runtime_error("Socket IPv must be IPv4, IPv6 or UNSPECIFIED");
	}

	/*--Set protocol--*/
	switch(protocol){
		case TCP:
			hints.ai_socktype = SOCK_STREAM;
			break;
		case UDP:
			hints.ai_socktype = SOCK_DGRAM;
			break;
		default:
			throw std::runtime_error("Socket protocol must be TCP or UDP");
	}

	// Set IP address as ours
	hints.ai_flags = AI_PASSIVE;

}

