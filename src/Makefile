GPP_FLAGS           = -std=c++11 -g -Wall
SERVER_OBJECT_LIST  = Server/server_main.o Server/Server.o Server/BotnetServer.o
BOT_OBJECT_LIST     = Bot/bot_main.o Bot/Bot.o Bot/TCPFunctions.o
THREAD_LIB          = -lpthread
SOCKET_LIB          = -LSocket -lSocket
SUBDIR              = Socket Bot Server

.PHONY: all
.PHONY: clean
.PHONY: Dependencies
.PHONY: BotnetServer
.PHONY: BotnetBot


all: Dependencies BotnetServer BotnetBot

## Server
BotnetServer:
	g++ $(GPP_FLAGS) $(SERVER_OBJECT_LIST) -o BotnetServer $(THREAD_LIB) $(SOCKET_LIB)


## Client
BotnetBot:
	g++ $(GPP_FLAGS) $(BOT_OBJECT_LIST) -o BotnetBot $(THREAD_LIB) $(SOCKET_LIB)

## Dependencies
Dependencies:
	for dir in $(SUBDIR); do \
		$(MAKE) -C $$dir; \
	done


## Clean
clean:
	for dir in $(SUBDIR); do \
		$(MAKE) -C $$dir clean; \
	done
	
	
	
	
	
	
	
	
	
	
	
	
