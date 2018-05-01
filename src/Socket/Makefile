GPP_FLAGS    = -std=c++11 -g
OBJECT_LIST  = TCPSocket.o Socket.o SecureSocket.o
HEADER_FILES = Socket_Set.h SocketIncludes.h SecureSocket.h

libSocket.a: $(OBJECT_LIST)
	ar rvs libSocket.a Socket.o TCPSocket.o SecureSocket.o
Socket.o: Socket.cpp Socket.h $(HEADER_FILES)
	g++ $(GPP_FLAGS) -c Socket.cpp
TCPSocket.o: TCPSocket.cpp TCPSocket.h $(HEADER_FILES)
	g++ $(GPP_FLAGS) -c TCPSocket.cpp
SecureSocket.o: SecureSocket.cpp SecureSocket.h
	g++ $(GPP_FLAGS) -c SecureSocket.cpp

clean:
	rm $(OBJECT_LIST)
