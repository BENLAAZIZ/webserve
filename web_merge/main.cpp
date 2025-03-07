
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
	// std::vector<int> fds;
	std::vector<pollfd> _fds;
	std::map<int, Client> _clients;
	std::vector<int> created;
	std::vector<ServerConfig> Sconfigs;

	// if (argc > 2) {
	// 	printUsage(argv[0]);
	// 	return EXIT_FAILURE;
	// } else if (argc == 2) {
	// 	configPath = argv[1];
	// 	std::cout << "Using configuration file: " << configPath << std::endl;
	// }

	try {
		// Parse configuration
		// Config config;
		// if (!config.load(configPath)) {
		// 	std::cerr << "Failed to load configuration from " << configPath << std::endl;
		// 	return EXIT_FAILURE;
		// }

		// std::cout << "Configuration loaded successfully from " << configPath << std::endl;

		// Initialize servers based on configuration
		// const std::vector<ServerConfig>& serverConfigs = config.getServers();
		ServerConfig serverc1;
		ServerConfig serverc2;
		ServerConfig serverc3;
		// serverConfig.host = "localhost";
		// serverConfig.port = 8080;
		serverc1.host = "localhost";
		serverc1.port = 8080;
		Sconfigs.push_back(serverc1);

		serverc2.host = "localhost";
		serverc2.port = 7070;
		Sconfigs.push_back(serverc2);

		serverc3.host = "localhost";
		serverc3.port = 5050;
		Sconfigs.push_back(serverc3);

		// for (size_t i = 0; i < 3; ++i) {
		for (size_t i = 0; i < Sconfigs.size(); ++i) {
			try {
				// Server server(serverConfigs[i]);
				Server server(Sconfigs[i]);
				if (server.createServer()) {
					servers.push_back(server);
					_fds.insert(_fds.end(), servers[i]._fds.begin(), servers[i]._fds.end());
					std::cout << "listen to http://" << Sconfigs[i].host //serverConfigs[i].host
							  << ":" << Sconfigs[i].port << std::endl;
				} else {
					std::cerr << "Failed to initialize server on " << Sconfigs[i].host //serverConfigs[i].host
							  << ":" << Sconfigs[i].port << std::endl;//serverConfigs[i].port
				}
			} catch (const std::exception& e) {
				std::cerr << "Error initializing server: " << e.what() << std::endl;
			}
		}
		std::cout << "\n ------- Severs initialize successfully --------\n" << std::endl;
		// pause();
		if (servers.empty()) {
			std::cerr << "No servers could be initialized. Exiting." << std::endl;
			return EXIT_FAILURE;
		}

		std::cout << "Webserver started successfully. Press Ctrl+C to stop." << std::endl;

		// Main server loop
		// while (g_running){
		int pollResult;
		while (true) {
			// std::cout << "Processing events..." << std::endl;
			pollResult = poll(_fds.data(), _fds.size(), 100);
			//Poll for events with a short timeout
			// int pollResult = poll(_fds.data(), _fds.size(), 100);
			// std::cout << "pollResult: " << pollResult << std::endl;
			if (pollResult < 0) {
				if (errno == EINTR) {
					// Interrupted by signal, just continue
					break;
				}
				std::cerr << "Poll failed: " << strerror(errno) << std::endl;
				break;
			}

			if (pollResult == 0) {
				// Timeout, no events
				break;
			}
			size_t i;
			for (i = 0; i < _fds.size(); ++i) {
			// for (size_t i = 0; i < servers.size(); ++i) {
				processEvents(i, _fds, servers, _clients);
				// std::cout << "i = " << i << std::endl;
				// std::cout << "server[i]._config.port: " << servers[i]._config.port << std::endl;
			}
		}

		std::cout << "Shutting down servers..." << std::endl;
		// Cleanup will happen automatically as servers go out of scope

	} catch (const std::exception& e) {
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Webserver shutdown complete." << std::endl;
	return 0;
}




// ========================================================

