GPP_FLAGS    = -std=c++11 -g -Wall
OBJECT_LIST  = main.o Server.o Socket.o TCPSocket.o BotnetServer.o
THREAD_LIB   = -lpthread


BotnetServer: $(OBJECT_LIST)
	g++ $(GPP_FLAGS) $(OBJECT_LIST) -o BotnetServer $(THREAD_LIB)
main.o: main.cpp
	g++ $(GPP_FLAGS) -c main.cpp
Socket.o: Socket/Socket.cpp Socket/Socket.h
	g++ $(GPP_FLAGS) -c Socket/Socket.cpp
TCPSocket.o: Socket/TCPSocket.cpp Socket/TCPSocket.h
	g++ $(GPP_FLAGS) -c Socket/TCPSocket.cpp
Server.o: Server/Server.cpp Server/Server.h
	g++ $(GPP_FLAGS) -c Server/Server.cpp
BotnetServer.o: Server/BotnetServer.cpp Server/BotnetServer.h
	g++ $(GPP_FLAGS) -c Server/BotnetServer.cpp

## Clean
clean:
	rm $(OBJECT_LIST)
