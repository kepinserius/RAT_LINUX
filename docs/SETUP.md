# Educational RAT - Setup Instructions

This document provides instructions for setting up and building the educational RAT project.

## Prerequisites

- Linux/Unix operating system
- C++ compiler (GCC or Clang)
- OpenSSL development libraries
- Build tools (make, etc.)

### Installing Dependencies

#### Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential libssl-dev
```

#### Fedora/RHEL/CentOS:

```bash
sudo dnf install gcc-c++ openssl-devel make
# or
sudo yum install gcc-c++ openssl-devel make
```

## Building the Project

1. Clone the repository (if you're using version control):

```bash
git clone https://your-repo-url.git
cd educational-rat
```

2. Build the project using make:

```bash
make setup
make
```

This will build both the server and client components.

## Running the Server

```bash
./bin/server [options]
```

Options:

- `-p, --port PORT` - Specify the listening port (default: 8080)
- `-h, --help` - Show help message

## Running the Client

```bash
./bin/client [options]
```

Options:

- `-s, --server ADDR` - Specify the server address (default: 127.0.0.1)
- `-p, --port PORT` - Specify the server port (default: 8080)
- `-h, --help` - Show help message

## Server Usage

Once the server is running, you can use the following commands:

- `help` - Show available commands
- `list` - List connected clients
- `select <client_id>` - Select a client to work with
- `ping <client_id>` - Ping a client
- `shell <client_id> <command>` - Execute shell command on client
- `screenshot <client_id>` - Take a screenshot from client
- `webcam <client_id>` - Capture webcam image from client
- `files <client_id> <path>` - List files in directory on client
- `download <client_id> <remote_path> <local_path>` - Download file from client
- `upload <client_id> <local_path> <remote_path>` - Upload file to client
- `ps <client_id>` - List processes on client
- `kill <client_id> <pid>` - Kill process on client
- `keylogger <client_id> <start|stop|dump>` - Control keylogger
- `persist <client_id>` - Install persistence on client
- `unpersist <client_id>` - Remove persistence from client
- `broadcast <command>` - Execute command on all clients
- `exit` - Exit the program

## Note

Remember that this project is for educational purposes only. Use it responsibly and only on systems you own or have explicit permission to access.
