#ifndef SECURE_SOCKET_H
#define SECURE_SOCKET_H

#include "TCPSocket.h"
#include "../defines.h"

namespace botnet {

class SecureSocket : public tcpSocket {
private:
	uint32_t encryption_key;

	std::string Encrypt(const void* message, size_t size) const;
	std::string Decrypt(const void* message, size_t size) const;
public:
	/*---C'tors---*/
	SecureSocket(int port, IPv ip_version, uint32_t encryption_key = botnet_defines::DEFAULT_ENCYPTION_KEY);
	SecureSocket(int port, uint32_t encryption_key = botnet_defines::DEFAULT_ENCYPTION_KEY);
	SecureSocket(const SecureSocket& copy);
	SecureSocket();

	virtual ~SecureSocket() = default;

	/**
	* Sends message encrypted using encryption_key,
	* sent message length is set as msg.length()
	* Return value: number of bytes sent, or -1 on error.
	*/
	virtual int Send(const std::string& msg) const;
	/**
	* Sends void* encrypted using encryption_key,
	* size is gotten from var "size"
	* Return value: number of bytes sent, or -1 on error.
	*/
	virtual int Send(const void* msg, size_t size) const;
	/**
	* Receives encrypted message and decrypts it using encryption_key.
	* Length specifies the largest amount of bytes willing to receive.
	* Decrypted message will be placed in msg.
	* Return value: number of bytes received, or 0 on closed connection, -1 on error, and -2 on timeout.
	*/
	virtual int Recv(std::string& msg, int length, double timeout_secs=-1.0) const;
	virtual int Recv(void* msg, int length, double timeout_secs=-1.0) const;
};

}

#endif