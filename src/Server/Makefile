GPP_FLAGS   = -std=c++11 -g -Wall
OBJECT_LIST = server_main.o BotnetServer.o Server.o

.PHONY: all
.PHONY: clean

## Build all objects
all: $(OBJECT_LIST)

server_main.o: server_main.cpp
	g++ $(GPP_FLAGS) -c server_main.cpp
BotnetServer.o: BotnetServer.cpp BotnetServer.h
	g++ $(GPP_FLAGS) -c BotnetServer.cpp
Server.o: Server.cpp Server.h
	g++ $(GPP_FLAGS) -c Server.cpp

## Clean
clean:
	rm $(OBJECT_LIST)
