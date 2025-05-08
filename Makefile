CC = g++
CFLAGS = -Wall -std=c++17
LIBS = -lpthread -lcrypto -lssl

# Platform specific settings
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += -DLinux
    LIBS += -lX11 -lXext
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -DOSx
    LIBS += -framework ApplicationServices
endif
ifeq ($(OS),Windows_NT)
    CFLAGS += -D_WIN32
    LIBS += -lgdi32 -lgdiplus -lws2_32 -luserenv
endif

SERVER_SRC = $(wildcard server/src/*.cpp common/src/*.cpp)
CLIENT_SRC = $(wildcard client/src/*.cpp common/src/*.cpp)

SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)

all: deps setup server client

deps:
	@echo "Installing dependencies..."
	@if [ -f /etc/debian_version ]; then \
		apt-get update && apt-get install -y build-essential libssl-dev libx11-dev; \
	elif [ -f /etc/redhat-release ]; then \
		yum install -y gcc-c++ openssl-devel libX11-devel; \
	elif [ "$(OS)" = "Windows_NT" ]; then \
		echo "On Windows, please install MinGW-w64, OpenSSL, and Windows SDK manually."; \
	else \
		echo "Please install build tools, OpenSSL, and X11 development packages manually"; \
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

# Windows compilation using MinGW (run on Windows)
windows:
	@echo "Building for Windows using MinGW..."
	g++ -Wall -std=c++17 -D_WIN32 $(SERVER_SRC) -o bin/server.exe -lpthread -lcrypto -lssl -lws2_32
	g++ -Wall -std=c++17 -D_WIN32 $(CLIENT_SRC) -o bin/client.exe -lpthread -lcrypto -lssl -lws2_32 -lgdi32 -lgdiplus -luserenv

.PHONY: all clean deps setup server client windows 