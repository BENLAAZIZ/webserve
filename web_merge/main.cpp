
#include "include/web.h"
// #include "include/Server.hpp"
// #include "include/Config.hpp"
#include <signal.h>

// ========================================================


// void processEvents(, ) 
void processEvents(size_t i, std::vector<pollfd>& _fds, std::vector<Server>& servers, std::map<int, Client>& _clients)
{
	// Poll for events with a short timeout
	// int pollResult = poll(_fds.data(), _fds.size(), -1);
	// // std::cout << "pollResult: " << pollResult << std::endl;
	// if (pollResult < 0) {
	// 	if (errno == EINTR) {
	// 		// Interrupted by signal, just continue
	// 		return;
	// 	}
	// 	std::cerr << "Poll failed: " << strerror(errno) << std::endl;
	// 	return;
	// }
	
	// if (pollResult == 0) {
	// 	// Timeout, no events
	// 	return;
	// }
	
	// Check each file descriptor for events
	// for (size_t i = 0; i < _fds.size(); i++) 
	// {
		if (_fds[i].revents == 0) {
			// continue;
			return;
		}
		// std::cout << " _fds[i].fd: " << _fds[i].fd << std::endl;
		
		// Handle server socket - new connection
		if (_fds[i].fd == servers[i].serverSocket && (_fds[i].revents & POLLIN)) {
			servers[i].handleNewConnection();
			// continue;
			return;
		}
		
		// Handle client data - reading request
		if (_fds[i].revents & POLLIN) {
			std::cout << " == handleClientData == " << std::endl;
			servers[i].handleClientData(_fds[i].fd);
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
							servers[i].removeClient(clientFd);
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
		if ((!_fds[i].revents) & (POLLERR | POLLHUP | POLLNVAL)) {
			// std::cout << " == Here == " << std::endl;
			// std::cout << "_fds[i].revents: " << _fds[i].revents << std::endl;
			// std::cout << "Client disconnected due to error or hangup" << std::endl;
			servers[i].removeClient(_fds[i].fd);
		}
	// }
}
// ========================================================


// ========================================================

// volatile sig_atomic_t g_running = 1;

// // Signal handler for graceful termination
// void signalHandler(int signum) {
//     std::cout << "\nReceived signal " << signum << ". Shutting down..." << std::endl;
//     g_running = 0;
// }

// void setupSignalHandlers() {
//     struct sigaction sa;
//     sa.sa_handler = signalHandler;
//     sigemptyset(&sa.sa_mask);
//     sa.sa_flags = 0;
	
//     if (sigaction(SIGINT, &sa, NULL) == -1) {
//         perror("Failed to set SIGINT handler");
//         exit(EXIT_FAILURE);
//     }
	
//     if (sigaction(SIGTERM, &sa, NULL) == -1) {
//         perror("Failed to set SIGTERM handler");
//         exit(EXIT_FAILURE);
//     }
// }

void printUsage(const std::string& programName) {
	std::cerr << "Usage: " << programName << " [config_file]" << std::endl;
}

int main() {
    std::string configPath = "default.conf";
    std::vector<Server> servers;
    std::vector<pollfd> allFds;
    std::map<int, Client> _clients;
    
    try {
        // Parse configuration
		// Config config;
		// if (!config.load(configPath)) {
		// 	std::cerr << "Failed to load configuration from " << configPath << std::endl;
		// 	return EXIT_FAILURE;
		// }

		// std::cout << "Configuration loaded successfully from " << configPath << std::endl;
        // Create server configurations
        ServerConfig serverc1;
        ServerConfig serverc2;
        ServerConfig serverc3;
        
        serverc1.host = "localhost";
        serverc1.port = 8080;
        
        serverc2.host = "localhost";
        serverc2.port = 7070;
        
        serverc3.host = "localhost";
        serverc3.port = 5050;
        
        std::vector<ServerConfig> Sconfigs;
        Sconfigs.push_back(serverc1);
        Sconfigs.push_back(serverc2);
        Sconfigs.push_back(serverc3);
        
        // Initialize servers
        for (size_t i = 0; i < Sconfigs.size(); ++i) {
            try {
                Server server(Sconfigs[i]);
                if (server.createServer()) {
                    servers.push_back(server);
                    // Add server socket to global polling array
                    pollfd serverPollFd;
                    serverPollFd.fd = servers.back().serverSocket;
                    serverPollFd.events = POLLIN;
                    allFds.push_back(serverPollFd);
                    
                    std::cout << "Server created successfully on " << Sconfigs[i].host
                              << ":" << Sconfigs[i].port << std::endl;
                    std::cout << "listen to http://" << Sconfigs[i].host
                              << ":" << Sconfigs[i].port << std::endl;
                } else {
                    std::cerr << "Failed to initialize server on " << Sconfigs[i].host
                              << ":" << Sconfigs[i].port << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error initializing server: " << e.what() << std::endl;
            }
        }
        
        std::cout << "\n ------- Servers initialized successfully --------\n" << std::endl;
        
        if (servers.empty()) {
            std::cerr << "No servers could be initialized. Exiting." << std::endl;
            return EXIT_FAILURE;
        }
        
        std::cout << "Webserver started successfully. Press Ctrl+C to stop." << std::endl;
        
        // Create a map to track which server owns each file descriptor
        std::map<int, Server*> fdToServer;
        for (size_t i = 0; i < servers.size(); ++i) {
            fdToServer[servers[i].serverSocket] = &servers[i];
        }
        
        // Main server loop
        while (true) {
            // Poll for events
            int pollResult = poll(allFds.data(), allFds.size(), 100);
            
            if (pollResult < 0) {
                if (errno == EINTR) {
                    // Interrupted by signal, just continue
                    continue;
                }
                std::cerr << "Poll failed: " << strerror(errno) << std::endl;
                break;
            }
            
            if (pollResult == 0) {
                // Timeout, no events
                continue;
            }
            
            // Process events for each file descriptor
            for (size_t i = 0; i < allFds.size(); ++i) {
                if (allFds[i].revents == 0) {
                    continue;
                }
                
                int currentFd = allFds[i].fd;
                
                // Find which server this FD belongs to
                Server* currentServer = nullptr;
                for (size_t j = 0; j < servers.size(); ++j) {
                    if (currentFd == servers[j].serverSocket) {
                        currentServer = &servers[j];
                        break;
                    }
                }
                
                // If it's not a server socket, check the client-to-server map
                if (!currentServer && fdToServer.find(currentFd) != fdToServer.end()) {
                    currentServer = fdToServer[currentFd];
                }
                
                if (!currentServer) {
                    std::cerr << "Error: Can't find server for fd " << currentFd << std::endl;
                    // Remove this fd from polling
                    close(currentFd);
                    allFds.erase(allFds.begin() + i);
                    i--; // Adjust index
                    continue;
                }
                
                // Handle new connections on server sockets
                if (currentFd == currentServer->serverSocket && (allFds[i].revents & POLLIN)) {
                    std::cout << " == handleNewConnection == " << std::endl;
                    int newClientFd = currentServer->acceptNewConnection();
                    if (newClientFd > 0) {
                        // Add new client fd to global polling array
                        pollfd clientPollFd;
                        clientPollFd.fd = newClientFd;
                        clientPollFd.events = POLLIN;
                        allFds.push_back(clientPollFd);
                        
                        // Add to server mapping
                        fdToServer[newClientFd] = currentServer;
                    }
                    continue;
                }
                
                // Handle client data - reading request
                if (allFds[i].revents & POLLIN) {
                    std::cout << " == handleClientData == " << std::endl;
                    bool keepConnection = currentServer->handleClientData(currentFd);
                    if (!keepConnection) {
                        // Remove from poll array
                        fdToServer.erase(currentFd);
                        allFds.erase(allFds.begin() + i);
                        i--; // Adjust index
                        continue;
                    }
                    // If we need to write back, update events
                    if (currentServer->clientReadyToSend(currentFd)) {
                        allFds[i].events |= POLLOUT;
                    }
                }
                
                // Handle client data - writing response
                if (allFds[i].revents & POLLOUT) {
                    bool keepConnection = currentServer->handleClientResponse(currentFd);
                    if (!keepConnection) {
                        // Remove from poll array
                        fdToServer.erase(currentFd);
                        allFds.erase(allFds.begin() + i);
                        i--; // Adjust index
                    } else if (!currentServer->clientReadyToSend(currentFd)) {
                        // Reset to only listen for reads
                        allFds[i].events = POLLIN;
                    }
                }
                
                // Handle errors or disconnection
                if ((allFds[i].revents) & (POLLERR | POLLHUP | POLLNVAL)) {
                    std::cout << "Client/Server error on fd " << currentFd << std::endl;
                    currentServer->removeClient(currentFd);
                    fdToServer.erase(currentFd);
                    allFds.erase(allFds.begin() + i);
                    i--; // Adjust index
                }
            }
        }
        
        std::cout << "Shutting down servers..." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cout << "Webserver shutdown complete." << std::endl;
    return 0;
}


// ========================================================

