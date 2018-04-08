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
	*/
	typedef enum { PRINT_BOTS=1, PING, GET_INFO } Commands;
private:
	std::vector<std::string> GetCommandsList() const {
		static std::vector<std::string> commands = {"Print Bots", "Ping", "Get Info"};
		return commands;
	};
	typedef void (BotnetServer::*CommandFunction)();
	CommandFunction GetCommandFunction(Commands command) {
		static const std::vector<CommandFunction> func_vector = {
			&BotnetServer::PrintBots,
			&BotnetServer::Ping,
			&BotnetServer::GetInfo
		};

		// Return at place command-1 because "Commands" starts at 1 and not 0
		return func_vector[command-1];
	};

	void GetMessages();
	void GetConnections();
	virtual void HandleMessage(const std::string& msg, const tcpSocket& socket) override;

	/* CLI functions */
	std::string GetCLIPrompt() const;
	void BotnetCLI();


	/* Botnet commands void(void) functions */

	void PrintBots();
	void Ping();
	void GetInfo();


	/* Botnet commands helpers */

	std::string ReadIP();
	bool LegalIP(const std::string& IP_address) const;
	
	static const int ALL_BOTS = -1;
	/**
	* Reads bot id from user input and continues to read until entered a real number or '*',
	* which means all bots, in which case ALL_BOTS is returned.
	*/
	int ReadID();

};


}

#endif