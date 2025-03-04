// #include "Server.hpp"
// #include <iostream>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <cstring>
// #include <algorithm>

// Server::Server() : _running(false) {
// }

// Server::~Server() {
//     // Clean up clients
//     for (auto& pair : _clients) {
//         delete pair.second;
//     }
	
//     // Close sockets
//     for (int sock : _listenSockets) {
//         close(sock);
//     }
// }

// void Server::createServer() 
// {
	
// }

// bool Server::init(const std::string& configPath) {
//     // Load configuration
//     if (!_config.load(configPath)) {
//         std::cerr << "Failed to load configuration from " << configPath << std::endl;
//         return false;
//     }
	
//     // Set up server sockets
//     if (!setupServerSockets()) {
//         std::cerr << "Failed to set up server sockets" << std::endl;
//         return false;
//     }
	
//     return true;
// }

// bool Server::setupServerSockets() {
//     const std::vector<ServerConfig>& servers = _config.getServers();
	
//     for (const auto& server : servers) {
//         // Create socket
//         int sock = socket(AF_INET, SOCK_STREAM, 0);
//         if (sock < 0) {
//             std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
//             return false;
//         }
		
//         // Configure socket
//         if (!configureSocket(sock)) {
//             close(sock);
//             return false;
//         }
		
//         // Bind socket
//         struct sockaddr_in addr;
//         memset(&addr, 0, sizeof(addr));
//         addr.sin_family = AF_INET;
//         addr.sin_port = htons(server.port);
		
//         if (server.host == "0.0.0.0") {
//             addr.sin_addr.s_addr = INADDR_ANY;
//         } else {
//             inet_pton(AF_INET, server.host.c_str(), &addr.sin_addr.s_addr);
//         }
		
//         if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
//             std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
//             close(sock);
//             return false;
//         }
		
//         // Listen on socket
//         if (listen(sock, 128) < 0) {
//             std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
//             close(sock);
//             return false;
//         }
		
//         // Add to our list of listening sockets
//         _listenSockets.push_back(sock);
//         _socketToServer[sock] = const_cast<ServerConfig*>(&server);
		
//         std::cout << "Server listening on " << server.host << ":" << server.port << std::endl;
//     }
	
//     return !_listenSockets.empty();
// }

// bool Server::configureSocket(int sock) {
//     // Set socket options
//     int opt = 1;
//     if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
//         std::cerr << "Error setting SO_REUSEADDR: " << strerror(errno) << std::endl;
//         return false;
//     }
	
//     // Set non-blocking mode
//     int flags = fcntl(sock, F_GETFL, 0);
//     if (flags < 0) {
//         std::cerr << "Error getting socket flags: " << strerror(errno) << std::endl;
//         return false;
//     }
	
//     if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
//         std::cerr << "Error setting non-blocking mode: " << strerror(errno) << std::endl;
//         return false;
//     }
	
//     return true;
// }

// void Server::run() {
//     _running = true;
	
//     // Set up initial poll structure with listen sockets
//     updatePollFds();
	
//     std::cout << "Server started successfully" << std::endl;
	
//     while (_running) {
//         // Wait for events with poll
//         int numEvents = poll(_pollFds.data(), _pollFds.size(), -1);
		
//         if (numEvents < 0) {
//             if (errno == EINTR) {
//                 // Interrupted by signal, just continue
//                 continue;
//             }
			
//             std::cerr << "Error in poll(): " << strerror(errno) << std::endl;
//             break;
//         }
		
//         // Process events
//         for (size_t i = 0; i < _pollFds.size() && numEvents > 0; i++) {
//             if (_pollFds[i].revents == 0) {
//                 continue;
//             }
			
//             int sock = _pollFds[i].fd;
//             bool isListenSocket = std::find(_listenSockets.begin(), _listenSockets.end(), sock) != _listenSockets.end();
			
//             if (isListenSocket && (_pollFds[i].revents & POLLIN)) {
//                 // Accept new connection
//                 handleNewConnection(sock);
//             } else if (_pollFds[i].revents & POLLIN) {
//                 // Read from client
//                 handleClientRead(sock);
//             } else if (_pollFds[i].revents & POLLOUT) {
//                 // Write to client
//                 handleClientWrite(sock);
//             } else if (_pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
//                 // Error or hang up
//                 if (!isListenSocket) {
//                     removeClient(sock);
//                 } else {
//                     std::cerr << "Error on listen socket, stopping server" << std::endl;
//                     _running = false;
//                     break;
//                 }
//             }
			
//             numEvents--;
//         }
		
//         // Update poll fds after events have been processed
//         updatePollFds();
//     }
	
//     std::cout << "Server shutting down" << std::endl;
// }

// void Server::handleNewConnection(int listenSocket) {
//     struct sockaddr_in clientAddr;
//     socklen_t clientAddrLen = sizeof(clientAddr);
	
//     int clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
	
//     if (clientSocket < 0) {
//         if (errno != EAGAIN && errno != EWOULDBLOCK) {
//             std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
//         }
//         return;
//     }
	
//     // Set client socket to non-blocking
//     int flags = fcntl(clientSocket, F_GETFL, 0);
//     if (flags < 0 || fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) < 0) {
//         std::cerr << "Error setting client socket to non-blocking mode" << std::endl;
//         close(clientSocket);
//         return;
//     }
	
//     // Get client IP address
//     char clientIP[INET_ADDRSTRLEN];
//     inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
	
//     std::cout << "New connection from " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
	
//     // Add the client
//     ServerConfig* serverConfig = _socketToServer[listenSocket];
//     addClient(clientSocket, serverConfig);
// }

// void Server::handleClientRead(int clientSocket) {
//     auto it = _clients.find(clientSocket);
//     if (it == _clients.end()) {
//         return;
//     }
	
//     Client* client = it->second;
	
//     // Read from socket into client's buffer
//     bool keepAlive = client->readFromSocket();
	
//     if (!keepAlive) {
//         removeClient(clientSocket);
//         return;
//     }
	
//     // Process the client if data is available
//     processClient(client);
// }

// void Server::handleClientWrite(int clientSocket) {
//     auto it = _clients.find(clientSocket);
//     if (it == _clients.end()) {
//         return;
//     }
	
//     Client* client = it->second;
	
//     // Write from client's buffer to socket
//     bool keepAlive = client->writeToSocket();
	
//     if (!keepAlive) {
//         removeClient(clientSocket);
//     }
// }

// void Server::addClient(int clientSocket, ServerConfig* serverConfig) {
//     Client* client = new Client(clientSocket, serverConfig, &_config);
//     _clients[clientSocket] = client;
// }

// void Server::removeClient(int clientSocket) {
//     auto it = _clients.find(clientSocket);
//     if (it != _clients.end()) {
//         delete it->second;
//         _clients.erase(it);
//     }
	
//     close(clientSocket);
// }

// void Server::updatePollFds() {
//     _pollFds.clear();
	
//     // Add listen sockets to poll
//     for (int sock : _listenSockets) {
//         struct pollfd pfd;
//         pfd.fd = sock;
//         pfd.events = POLLIN;
//         pfd.revents = 0;
//         _pollFds.push_back(pfd);
//     }
	
//     // Add client sockets to poll
//     for (const auto& pair : _clients)
//      {
//         int sock = pair.first;
//         Client* client = pair.second;
		
//         struct pollfd pfd;
//         pfd.fd = sock;
//         pfd.events = 0;
//         pfd.revents = 0;
		
//         // Set events based on client state
//         if (client->wantsToRead()) {
//             pfd.events |= POLLIN;
//         }
		
//         if (client->wantsToWrite()) {
//             pfd.events |= POLLOUT;
//         }
		
//         _pollFds.push_back(pfd);
//     }
// }

// void Server::processClient(Client* client) {
//     // Process client's current state
//     client->process();
// }

// void Server::stop() {
//     _running = false;
// }

// //========================================================


#include "Server.hpp"
#include "Client.hpp"
#include <cstring>
#include <errno.h>

Server::Server(const ServerConfig& config) : _config(config), serverSocket(-1) {
}

Server::~Server() {
	// Close server socket if open
	if (serverSocket >= 0) {
		close(this->serverSocket);
	}
	
	// Close all client connections
	for (size_t i = 0; i < _fds.size(); i++) {
		if (_fds[i].fd >= 0 && _fds[i].fd != this->serverSocket) {
			close(_fds[i].fd);
		}
	}
}

bool Server::setNonBlocking(int sock) {
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Failed to get socket flags" << std::endl;
		return false;
	}
	
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "Failed to set socket to non-blocking mode" << std::endl;
		return false;
	}
	
	return true;
}

bool Server::createServer() {
	struct sockaddr_in server_addr;
	
	// Create socket
	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverSocket == -1) {
		std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
		return false;
	}
	
	// Allow socket reuse
	int opt = 1;
	if (setsockopt(this->serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		std::cerr << "Failed to set socket options: " << strerror(errno) << std::endl;
		close(this->serverSocket);
		return false;
	}
	
	// Set non-blocking mode
	if (!setNonBlocking(this->serverSocket)) {
		close(this->serverSocket);
		return false;
	}
	
	// Setup server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	// server_addr.sin_addr.s_addr = inet_addr(_config.getHost().c_str());
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(_config.getPort());
	
	// Bind socket
	if (bind(this->serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
		close(this->serverSocket);
		return false;
	}
	
	// Start listening
	if (listen(this->serverSocket, 10) < 0) {
		std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
		close(this->serverSocket);
		return false;
	}
	
	// Add server socket to polling array
	pollfd serverPollFd;
	serverPollFd.fd = this->serverSocket;
	serverPollFd.events = POLLIN;
	_fds.push_back(serverPollFd);
	
	return true;
}

void	Server::handleNewConnection() {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	
	int clientSocket = accept(this->serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (clientSocket < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
		}
		return;
	}
	
	// Set client socket to non-blocking
	if (!setNonBlocking(clientSocket)) {
		close(clientSocket);
		return;
	}
	
	// Add to poll array
	pollfd clientPollFd;
	clientPollFd.fd = clientSocket;
	clientPollFd.events = POLLIN;
	_fds.push_back(clientPollFd);
	
	// Create new client object
	_clients[clientSocket] = Client(clientSocket, clientAddr, _config);
	
	// std::cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) 
	std::cout << "New client connected: " << clientAddr.sin_addr.s_addr 
			  << ":" << ntohs(clientAddr.sin_port) << std::endl;
}

void Server::handleClientData(int clientFd) {
	if (_clients.find(clientFd) == _clients.end()) {
		std::cerr << "Error: Client not found in map" << std::endl;
		removeClient(clientFd);
		return;
	}
	
	Client& client = _clients[clientFd];
	
	// Process client request
	// if (!client.readRequest()) {
	// 	std::cout << "Client disconnected during read" << std::endl;
	// 	removeClient(clientFd);
	// 	return;
	// }
	
	// // Check if request is complete
	// if (client.isRequestComplete()) {
	// 	// Parse and process the request
	// 	if (client.parseRequest()) {
	// 		// Generate response
	// 		client.generateResponse();
			
	// 		// Update the interested events to include POLLOUT for writing response
	// 		for (size_t i = 0; i < _fds.size(); i++) {
	// 			if (_fds[i].fd == clientFd) {
	// 				_fds[i].events |= POLLOUT;
	// 				break;
	// 			}
	// 		}
	// 	} else {
	// 		// Invalid request, send error response
	// 		client.sendErrorResponse(400, "Bad Request");
			
	// 		// Update for writing
	// 		for (size_t i = 0; i < _fds.size(); i++) {
	// 			if (_fds[i].fd == clientFd) {
	// 				_fds[i].events |= POLLOUT;
	// 				break;
	// 			}
	// 		}
	// 	}
	// }
	char buffer[210];
	ssize_t bytes_read = recv(client._socket, buffer, 210, 0); // Read 49 bytes at a time
	if (bytes_read < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			return; // No data available, wait for next poll()
		}
		std::cerr << "Recv failed: " << strerror(errno) << std::endl;
		close(client._socket);
		_fds.erase(_fds.begin() + clientFd);
		return;
	}
	
	if (bytes_read == 0) 
	{
		//std::cout << "Client disconnected, closing connection." << std::endl;
		close(client._socket);
		_fds.erase(_fds.begin() + clientFd);
		return;
	}

	std::string data(buffer, bytes_read);
	client._requestBuffer += data; // Append new data to client's buffer

	// if (client._requestBuffer.size() > MAX_REQUEST_SIZE) {
	// 	std::cerr << "Request too large!" << std::endl;
	// 	sendErrorResponse(413, "Request Entity Too Large");
	// 	return false;
	// }
	if (!client.isRequestComplete())
	{
		if (!client.parse_Header_Request(client._requestBuffer))
			client.sendErrorResponse(client.getStatusCode());
	}
	else
	{
		if (client._request.getMethod() == "POST")
		{
			if (!client.parseBody())
			{
				client.sendErrorResponse(client.getStatusCode());
			}
		}
		else
			client.generateResponse_GET_DELETE();
				
		// Update the interested events to include POLLOUT for writing response
		for (size_t i = 0; i < _fds.size(); i++) {
			if (_fds[i].fd == clientFd) {
				_fds[i].events |= POLLOUT;
				break;
			}
		}
	}

}

void Server::removeClient(int clientFd) {
	// Remove from clients map
	_clients.erase(clientFd);
	
	// Remove from polling array
	for (size_t i = 0; i < _fds.size(); i++) {
		if (_fds[i].fd == clientFd) {
			close(clientFd);
			_fds.erase(_fds.begin() + i);
			break;
		}
	}
}

void Server::processEvents() {
	// Poll for events with a short timeout
	int pollResult = poll(_fds.data(), _fds.size(), 100);
	
	if (pollResult < 0) {
		if (errno == EINTR) {
			// Interrupted by signal, just continue
			return;
		}
		std::cerr << "Poll failed: " << strerror(errno) << std::endl;
		return;
	}
	
	if (pollResult == 0) {
		// Timeout, no events
		return;
	}
	
	// Check each file descriptor for events
	for (size_t i = 0; i < _fds.size(); i++) {
		if (_fds[i].revents == 0) {
			continue;
		}
		
		// Handle server socket - new connection
		if (_fds[i].fd == this->serverSocket && (_fds[i].revents & POLLIN)) {
			handleNewConnection();
			continue;
		}
		
		// Handle client data - reading request
		if (_fds[i].revents & POLLIN) {
			handleClientData(_fds[i].fd);
		}
		
		// Handle client data - writing response
		if (_fds[i].revents & POLLOUT) {
			int clientFd = _fds[i].fd;
			
			if (_clients.find(clientFd) != _clients.end()) {
				Client& client = _clients[clientFd];
				
				// Send response
				if (!client.sendResponse()) {
					// Failed to send or completed sending
					if (client.isDoneWithResponse()) {
						// If keep-alive is not set, close the connection
						if (!client.keepAlive()) {
							removeClient(clientFd);
						} else {
							// Reset client for next request
							client.reset();
							// Update to only listen for reads again
							_fds[i].events = POLLIN;
						}
					}
				}
			}
		}
		
		// Handle errors or disconnection
		if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
			std::cout << "Client disconnected due to error or hangup" << std::endl;
			removeClient(_fds[i].fd);
		}
	}
}