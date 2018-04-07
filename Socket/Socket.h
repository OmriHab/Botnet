#ifndef SOCKET_H
#define SOCKET_H

#include <netdb.h>

namespace http {

class Socket {
public:
	/**
	* enum IPv    - Specify which IP version to use
	* IPv4/IPv6   - use IP version 4/6
	* UNSPECIFIED - use IP version 4 or 6 as needed
	*/
	enum IPv { IPv4, IPv6, UNSPECIFIED };
	/**
	* enum Type - Specify which protocol to use
	* TCP/UDP   - use TCP/UDP
	*/
	enum Type { TCP, UDP };

	// Copy C'tor
	Socket(const Socket& copy);

	// D'tor
	virtual ~Socket();

	/**
	* Equals/Un-equal Comparer: Gets another socket and compares their sock_ids for equality
	* Return value: equal if both sock_ids are equal, unequal otherwise
	*/
	bool operator==(const Socket& other) const;
	bool operator!=(const Socket& other) const;

	/**
	* Bind - Associates the socket to it's port address.
	* Required by a socket wishing to listen for packets.
	* Return value: success value
	*/
	virtual bool Bind();

	/*---Getters---*/
	// Returns sock_id
	int GetSockId() const;
	// Returns port
	int GetPort() const;
	// Returns IP version
	IPv GetIpVersion() const;

	/**
	* CanRead - returns if socket is able to be read from
	* Return value: True if can read, false if can't read
	*/
	bool CanRead() const;
	/**
	* CanWrite - returns if socket is able to write to
	* Return value: True if can write, false if can't write
	*/
	bool CanWrite() const;

	/**
	* IsConnected - returns if socket is still connected to local host
	* Return value: True if is connected, false if isn't
	*/
	virtual bool IsConnected() const;

	void Close() const;
protected:
	/*---C'tors---*/
	/**
	* port       - port to set socket at
	* protocol   - TCP/UDP
	* ip_version - IPv4, IPv6 or UNSPECIFIED (Use both by needs)
	*/
	Socket(int port, Type protocol, IPv ip_version);
	/**
	* If IP version not specified assumes unspecified, using both IPv4 and IPv6
	*/
	Socket(int port, Type protocol);
	/**
	* Constructor receiving only socket id for a socket already initialized outside
	* Use ONLY when the socket given is ALREADY initialized
	*/
	Socket(int sock_id);
	/**
	* Default empty C'tor.
	* Use when wanting to use SetSockId after when you have an initialized socket already.
	*/
	Socket();
	/**
	* SetSockId to use to set socket to an existing socket id
	* Use ONLY when the socket given is ALREADY initialized
	*/
	void SetSockId(int sock_id);


private:
	int sock_id;
	struct sockaddr* self_address;

	/* Helper functions */
	/* Deep copy a sockaddr struct */
	void copy_address(sockaddr* destination, const sockaddr* source);
	/* Load struct hints with needed info about self */
	void LoadHints(struct addrinfo &hints, Type protocol, IPv ip_version);
};

}

#endif