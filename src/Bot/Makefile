GPP_FLAGS   = -std=c++11 -g -Wall
OBJECT_LIST = bot_main.o Bot.o TCPFunctions.o
SOCKET_LIB  = -LSocket -lSocket

## Build all objects
all: $(OBJECT_LIST)

bot_main.o: bot_main.cpp
	g++ $(GPP_FLAGS) -c bot_main.cpp
Bot.o: Bot.cpp Bot.h
	g++ $(GPP_FLAGS) -c Bot.cpp
TCPFunctions.o: TCPFunctions.cpp TCPFunctions.h
	g++ $(GPP_FLAGS) -c TCPFunctions.cpp

## Clean
clean:
	rm $(OBJECT_LIST)
