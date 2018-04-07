#ifndef SOCKET_SET_H
#define SOCKET_SET_H

#include <boost/static_assert.hpp>
#include <deque>
#include <map>
#include <iostream>
#include <sys/select.h>
#include <type_traits>
#include <sys/poll.h>
#include <string.h>

#include "TCPSocket.h"


namespace http {


template<class T=Socket>
class Socket_Set {
	BOOST_STATIC_ASSERT(std::is_base_of<Socket, T>::value);
public:
	enum Op { READ, WRITE, EXCEPT };
private:
	fd_set set;
	int max_fd;
	Op operation;
	std::map<int, T> socket_map;
public:

	Socket_Set(Op operation) : operation(operation), max_fd(0) { FD_ZERO(&set); }
	~Socket_Set() = default;
	
	Socket_Set<T>& operator=(const Socket_Set<T>& other);
	

	/**
	* AddSocket - Adds socket to set
	*/
	void AddSocket(const T& sock_to_add);
	/**
	* RemoveSocket - Removes socket from set
	*/
	void RemoveSocket(const T& sock_to_remove);
	/**
	* Select - Waits and check if there are sockets in the set that can be read/written from, 
	* deppending on the given Op passed in the c'tor.
	* Select waits seconds and useconds before timing out
	* Return value: queue holding all sockets ready to read/write or empty queue on timeout/error
	*/
	std::deque<T> Select(int seconds=0, int u_seconds=0);
	/**
	* GetAllSockets - Returns deque of all sockets in socket_map
	*/
	std::deque<T> GetAllSockets();
};

template<class T>
Socket_Set<T>& Socket_Set<T>::operator=(const Socket_Set<T>& other) {
	this->set        = other.set;
	this->max_fd     = other.max_fd;
	this->operation  = other.operation;
	this->socket_map = other.socket_map;

	return *this;
}

template<class T>
void Socket_Set<T>::AddSocket(const T& sock_to_add) {
	int sock_id = sock_to_add.GetSockId();
	FD_SET(sock_id, &this->set);
	this->max_fd = sock_id > this->max_fd ? sock_id : this->max_fd;

	this->socket_map[sock_id] = sock_to_add;
}

template<class T>
void Socket_Set<T>::RemoveSocket(const T& sock_to_remove) {
	int sock_id = sock_to_remove.GetSockId();
	FD_CLR(sock_id, &this->set);

	// Set new max
	int max = 0;
	for (int i = 0; i < this->max_fd; i++) {
		if (FD_ISSET(i, &this->set)) {
			max = i;
		}
	}

	this->max_fd = max;

	this->socket_map.erase(sock_id);
}

template<class T>
std::deque<T> Socket_Set<T>::Select(int seconds, int u_seconds) {
	
	// Copy original set to not change the original
	fd_set set_cpy;
	set_cpy = this->set;

	int ret_val = 0;

	if (seconds > 0) {
		timeval wait_time;
		wait_time.tv_sec  = seconds;
		wait_time.tv_usec = u_seconds;

		switch (this->operation) {
		case READ:
			ret_val = select(max_fd+1, &set_cpy, nullptr, nullptr, &wait_time);
			break;
		case WRITE:
			ret_val = select(max_fd+1, nullptr, &set_cpy, nullptr, &wait_time);
			break;
		case EXCEPT:
			ret_val = select(max_fd+1, nullptr, nullptr, &set_cpy, &wait_time);
			break;
		}
	}
	// If entered negative seconds, wait indefinetly
	else {
		switch (this->operation) {
			case READ:
				ret_val = select(max_fd+1, &set_cpy, nullptr, nullptr, nullptr);
				break;
			case WRITE:
				ret_val = select(max_fd+1, nullptr, &set_cpy, nullptr, nullptr);
				break;
			case EXCEPT:
				ret_val = select(max_fd+1, nullptr, nullptr, &set_cpy, nullptr);
				break;
		}
	}
	// If select failed, throw exception
	if (ret_val == -1) {
		throw std::runtime_error("Error on select: " + std::string(strerror(errno)));
	}

	std::deque<T> ret_queue;
	for (int socket_id = 0; socket_id <= this->max_fd; socket_id++) {
		// If socket_id is in set, add socket to ret_queue
		if (FD_ISSET(socket_id, &set_cpy)) {
			ret_queue.push_back(this->socket_map[socket_id]);
		}
	}
	return ret_queue;
}



template <class T>
std::deque<T> Socket_Set<T>::GetAllSockets() {
	std::deque<T> ret_queue;
	for (auto socket : socket_map) {
		ret_queue.push_back(socket.second);
	}

	return ret_queue;
}

}
#endif