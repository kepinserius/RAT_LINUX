CC = g++
CFLAGS = -Wall -std=c++11
LIBS = -lpthread -lcrypto -lssl

SERVER_SRC = $(wildcard server/src/*.cpp common/src/*.cpp)
CLIENT_SRC = $(wildcard client/src/*.cpp common/src/*.cpp)

SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)

all: deps server client

deps:
	@echo "Installing dependencies..."
	@if [ -f /etc/debian_version ]; then \
		apt-get update && apt-get install -y build-essential libssl-dev; \
	elif [ -f /etc/redhat-release ]; then \
		yum install -y gcc-c++ openssl-devel; \
	else \
		echo "Please install build tools and OpenSSL development packages manually"; \
	fi

server: $(SERVER_OBJ)
	@echo "Building server..."
	$(CC) $(CFLAGS) -o bin/server $^ $(LIBS)

client: $(CLIENT_OBJ)
	@echo "Building client..."
	$(CC) $(CFLAGS) -o bin/client $^ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f server/src/*.o client/src/*.o common/src/*.o bin/server bin/client

setup:
	mkdir -p bin

.PHONY: all clean deps setup server client 