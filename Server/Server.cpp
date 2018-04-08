#include <algorithm>
#include <deque>
#include <iostream>
#include <signal.h>
#include <stdexcept>
#include <string.h>
#include <termios.h>
#include <thread>
#include <limits.h>

#include "Server.h"


using namespace botnet;

server::server(int port, int max_connections, bool verbose)
	: verbose(verbose)
	, continue_serving(true)
	, MainSocket(port)
	, max_connections(max_connections)
	, master_set(Socket_Set<tcpSocket>::READ) { }

server::~server() {

	ThreadSafeLog("Shutting down...\n");
	ThreadSafeLog("Closing all open sockets...\n");
	// Close all sockets we opened and haven't closed, including the listener socket
	for (const auto& socket : this->master_set.GetAllSockets()) {
		ThreadSafeLog("Closing socket " + std::to_string(socket.GetSockId()) + "\n");
		socket.Close();
		ThreadSafeLog(std::to_string(socket.GetSockId()) + " closed\n");
	}
	ThreadSafeLog("All done\n");
}

bool server::Serve() {
	// Set up server
	try {
		if (!this->SetUpServer()) {
			// No need for thread safety if reached here
			std::cerr << "Server: Failed to set up server, aborting" << std::endl;
			return false;
		}
	}
	catch (const std::exception& e) {
		// No need for thread safety if reached here
		std::cerr << "Server: Failed to set up server " << std::string(e.what()) << std::endl;
	}
	/*** MainSocket binded and listening ***/

	this->HandleConnections();
	
	return true;
}

bool server::SetUpServer() {
	ThreadSafeLog("Binding socket...\n");
	if (!this->MainSocket.Bind()) {
		return false;
	}
	ThreadSafeLog("Listening with socket...\n");
	if (!this->MainSocket.Listen(max_connections)) {
		return false;
	}

	// Add MainSocket to master_set
	this->master_set.AddSocket(MainSocket);
	return true;
}


void server::HandleConnections() {
	std::string message_received;
	// Maximum recv of 4 kilobytes
	static const int KILOBYTE     = 1024;
	static const int MAX_MSG_SIZE = 4*KILOBYTE;

	std::deque<tcpSocket> readable;
	tcpSocket tmpSocket;

	// Continue serving until asked to shut down by user
	while (this->continue_serving) {
		try {
			// Wait to be able to read on one of the sockets
			readable = master_set.Select(-1);
		}
		catch (const std::exception& e) {
			ThreadSafeLog(std::string(e.what()) + "\n", std::cerr);
			continue;
		}
		
		for (auto& socket : readable) {
			// If socket is the listener, try to accept the new socket
			if (socket.isListening()) {
				// If succedded in accepting new socket, add to master set
				if (socket.Accept(tmpSocket)) {
					this->master_set.AddSocket(tmpSocket);
				}
			}

			else {
				try {
					int ret_val = socket.Recv(message_received, MAX_MSG_SIZE);
					// If receive failed, remove socket
					if (ret_val <= 0) {
						this->master_set.RemoveSocket(socket);
					}
					else {
						HandleMessage(message_received, socket);
					}
				}
				catch (const std::exception& e) {
					ThreadSafeLog(std::string(e.what()) + "\n", std::cerr);
					continue;
				}
			}
		}
	}
}

void server::HandleMessage(const std::string& msg, const tcpSocket& sender) {
	// Just print the message
	ThreadSafeLog(msg + "\n\n");	
}

void server::ThreadSafeLog(const std::string& msg, std::ostream& stream) {
	// If not verbose, exit
	if (!verbose) {
		return;
	}
	// Lock 
	std::lock_guard<std::mutex> lg(this->out_stream_lock);
	// Output
	stream << msg << std::flush;
}

void server::ThreadSafeLogPrintAlways(const std::string& msg, std::ostream& stream) {
	// Lock 
	std::lock_guard<std::mutex> lg(this->out_stream_lock);
	// Output
	stream << msg << std::flush;
}

std::string server::ThreadSafeGetLine(std::istream& stream) {
	std::string input;

	std::lock_guard<std::mutex> read_lock(out_stream_lock);
	std::getline(stream, input);
	return input;
}