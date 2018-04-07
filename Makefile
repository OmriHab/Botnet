GPP_FLAGS    = -std=c++11 -g -Wall
OBJECT_LIST  = main.o Server.o Socket.o TCPSocket.o
HEADER_FILES = Soket/Socket_Set.h
THREAD_LIB   = -lpthread


Botnet: $(OBJECT_LIST)
	g++ $(GPP_FLAGS) $(OBJECT_LIST) -o Botnet $(THREAD_LIB)
main.o: main.cpp
	g++ $(GPP_FLAGS) -c main.cpp
Server.o: Server/Server.cpp Server/Server.h
	g++ $(GPP_FLAGS) -c Server/Server.cpp
Socket.o: Socket/Socket.cpp Socket/Socket.h
	g++ $(GPP_FLAGS) -c Socket/Socket.cpp
TCPSocket.o: Socket/TCPSocket.cpp Socket/TCPSocket.h
	g++ $(GPP_FLAGS) -c Socket/TCPSocket.cpp
clean:
	rm $(OBJECT_LIST)
