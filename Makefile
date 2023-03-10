CC=g++ -std=c++11 -Wall -Wextra

PORT = 2000

IP_SERVER = 127.0.0.1

all: server subscriber

# Compileaza server.cpp
server: server.cpp

# Compileaza client.cpp
subscriber: subscriber.cpp

# Ruleaza serverul
run_server:
	./server ${PORT}

run_tcp_client: build	
	./subscriber $(CLIENT_ID) $(IP_SERVER) $(PORT)

run_udp_client:
	sudo python3 udp_client.py $(IP_SERVER) $(PORT)

clean:
	rm -f *.o server subscriber