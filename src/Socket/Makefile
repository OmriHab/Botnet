GPP_FLAGS    = -std=c++11 -g
OBJECT_LIST  = TCPSocket.o Socket.o
HEADER_FILES = Socket_Set.h SocketIncludes.h

libSocket.a: $(OBJECT_LIST)
	ar rvs libSocket.a Socket.o TCPSocket.o
Socket.o: Socket.cpp Socket.h $(HEADER_FILES)
	g++ $(GPP_FLAGS) -c Socket.cpp
TCPSocket.o: TCPSocket.cpp TCPSocket.h $(HEADER_FILES)
	g++ $(GPP_FLAGS) -c TCPSocket.cpp

clean:
	rm $(OBJECT_LIST)
