GPP_FLAGS           = -std=c++11 -g -Wall
SERVER_OBJECT_LIST  = server_main.o Server.o BotnetServer.o
BOT_OBJECT_LIST     = bot_main.o bot.o
THREAD_LIB          = -lpthread
SOCKET_LIB          = -LSocket -lSocket


## Server
BotnetServer: $(SERVER_OBJECT_LIST)
	g++ $(GPP_FLAGS) $(SERVER_OBJECT_LIST) -o BotnetServer $(THREAD_LIB) $(SOCKET_LIB)
server_main.o: server_main.cpp
	g++ $(GPP_FLAGS) -c server_main.cpp
Server.o: Server/Server.cpp Server/Server.h
	g++ $(GPP_FLAGS) -c Server/Server.cpp
BotnetServer.o: Server/BotnetServer.cpp Server/BotnetServer.h
	g++ $(GPP_FLAGS) -c Server/BotnetServer.cpp

## Client
Bot: $(CLIENT_OBJECT_LIST)
	g++ $(GPP_FLAGS) $(CLIENT_OBJECT_LIST) -o Bot $(THREAD_LIB) $(SOCKET_LIB)
bot_main.o: bot_main.cpp
	g++ $(GPP_FLAGS) -c bot_main.cpp
Bot.o: Bot/Bot.cpp Bot/Bot.h
	g++ $(GPP_FLAGS) -c Bot/Bot.cpp

## Clean
clean:
	rm $(SERVER_OBJECT_LIST) $(CLIENT_OBJECT_LIST)

all: BotnetServer Bot