

#include "../include/web.h"

ConfigFile::ConfigFile() {
}

ConfigFile::~ConfigFile() {
	// Clean up all servers
	for (size_t i = 0; i < servers.size(); ++i) {
		delete servers[i];
	}
	
	// Close any remaining client connections
	for (std::map<int, int>::iterator it = client_server_map.begin(); it != client_server_map.end(); ++it) {
		close(it->first);
	}
}

void ConfigFile::addServer(int port) {
	Server* server = new Server(port);
	// ========================
	server->setConfig(this);
	// ========================
	servers.push_back(server);
	
	struct pollfd pfd;
	pfd.fd = server->getServerFd();
	pfd.events = POLLIN;
	poll_fds.push_back(pfd);
}

void ConfigFile::cleanupDisconnectedClient(int client_fd) {
	close(client_fd);
	client_server_map.erase(client_fd);
	
	// Remove from poll_fds
	for (size_t j = 0; j < poll_fds.size(); ++j) {
		if (poll_fds[j].fd == client_fd) {
			poll_fds.erase(poll_fds.begin() + j);
			break;
		}
	}
}

// void ConfigFile::handleEvents() {
// 	int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
// 	if (poll_count < 0) {
// 		std::cerr << "Poll failed: " << strerror(errno) << std::endl;
// 		return;
// 	}
	
// 	std::vector<struct pollfd> current_fds = poll_fds;
	
// 	for (size_t i = 0; i < current_fds.size(); ++i) {
// 		if (!(current_fds[i].revents & POLLIN)) {
// 			continue;
// 		}
		
// 		int current_fd = current_fds[i].fd;
// 		bool is_server = false;
// 		int server_index = -1;
		
// 		// Check if this is a server socket
// 		for (size_t j = 0; j < servers.size(); ++j) {
// 			if (current_fd == servers[j]->getServerFd()) {
// 				is_server = true;
// 				server_index = j;
// 				break;
// 			}
// 		}

// 		if (is_server) { // Handle new connection on this server
// 			int client_fd = servers[server_index]->acceptNewConnection();
// 			if (client_fd >= 0) {
// 				struct pollfd pfd;
// 				pfd.fd = client_fd;
// 				pfd.events = POLLIN;
// 				poll_fds.push_back(pfd);
// 				client_server_map[client_fd] = server_index; // Map client fd to server index
// 				Client client;
// 				client.setClientFd(client_fd);
// 				set_client_map(client_fd, client);
// 			}
// 		} else { // This is a client, handle the data
// 			if (client_server_map.find(current_fd) != client_server_map.end()) {
// 				int res = 0;
// 				int owner_server = client_server_map[current_fd];

// 				res = servers[owner_server]->handleClientData(current_fd, _clients[current_fd]);
// 				// std::cout << "res: " << res << std::endl;
// 				bool client_disconnected = false;
					
// 				if (res < 0)
// 					client_disconnected = true;
// 				else if (res == 1)
// 					client_disconnected = true;
		
// 				if (client_disconnected)
// 				{
// 					std::cerr << "Client disconnected" << std::endl;
// 					cleanupDisconnectedClient(current_fd);
// 				}
// 				// Handle client data - writing response
			
// 			} else {
// 				std::cerr << "Error: Client FD " << current_fd << " not associated with any server" << std::endl;
// 				cleanupDisconnectedClient(current_fd);
// 			}
// 		}
// 	}
// }

//----- start added -----//

void ConfigFile::handleEvents() {
    int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
    if (poll_count < 0) {
        std::cerr << "Poll failed: " << strerror(errno) << std::endl;
        return;
    }
    
    std::vector<struct pollfd> current_fds = poll_fds;
    
    for (size_t i = 0; i < current_fds.size(); ++i) {
        int current_fd = current_fds[i].fd;
        
        // Handle incoming data (POLLIN events)
        if (current_fds[i].revents & POLLIN) {
            bool is_server = false;
            int server_index = -1;
            
            // Check if this is a server socket
            for (size_t j = 0; j < servers.size(); ++j) {
                if (current_fd == servers[j]->getServerFd()) {
                    is_server = true;
                    server_index = j;
                    break;
                }
            }

            if (is_server) { // Handle new connection on this server
                int client_fd = servers[server_index]->acceptNewConnection();
                if (client_fd >= 0) {
                    struct pollfd pfd;
                    pfd.fd = client_fd;
                    pfd.events = POLLIN; // Initially set to POLLIN to read the request
                    poll_fds.push_back(pfd);
                    client_server_map[client_fd] = server_index; // Map client fd to server index
                    Client client;
                    client.setClientFd(client_fd);
                    set_client_map(client_fd, client);
                }
            } else { // This is a client, handle the request data
                if (client_server_map.find(current_fd) != client_server_map.end()) {
                    int res = 0;
                    int owner_server = client_server_map[current_fd];

                    res = servers[owner_server]->handleClientData(current_fd, _clients[current_fd]);
                    
                    // if (res < 0 || res == 1) {
                    if (res < 0) {
                        // Client disconnected or error
                        std::cerr << "(in pollin) -> Client disconnected from client : | " << current_fd << " | " << std::endl;
                        cleanupDisconnectedClient(current_fd);
                    }
                    else if (res == 0) {
                        // Request still being processed, continue to read
                        continue;
                    }
                    else if (res == 2) {
                        // Request fully received, switch to POLLOUT to send response
                        std::cout << "-----------  res : "  << res << " request complite from client : | " << current_fd << " | --> pass to response -------------" << std::endl;
                        for (size_t j = 0; j < poll_fds.size(); ++j) {
                            if (poll_fds[j].fd == current_fd) {
                                poll_fds[j].events = POLLOUT;
                                break;
                            }
                        }
                        // Prepare the response in the client object
                        // _clients[current_fd].prepareResponse();
                    }
                } else {
                    std::cerr << "Error: Client FD " << current_fd << " not associated with any server" << std::endl;
                    cleanupDisconnectedClient(current_fd);
                }
            }
        }
        // Handle outgoing data (POLLOUT events)
        else if (current_fds[i].revents & POLLOUT) {
            if (client_server_map.find(current_fd) != client_server_map.end()) {
                int owner_server = client_server_map[current_fd];
                // std::cout << "response: " << std::endl;
                // sendSuccessResponse(current_fd);
                int res = servers[owner_server]->sendResponse(current_fd, _clients[current_fd]);

                // std::cout << "--------------------------------------res: " << res << std::endl;
               if (res < 0) {
                    // Error sending response
                    std::cerr << "Error sending response to client" << std::endl;
                    cleanupDisconnectedClient(current_fd);
                }
                else if (res == 1) {
                    // Response fully sent, reset to POLLIN for next request or close connection
                    std::cout << "============  res : "  << res << " response complite from client : | " << current_fd  << " | ===============" <<  std::endl;
                    if (_clients[current_fd].keepAlive()) {
                        std::cout << "Keep-alive: waiting for next request" << std::endl;
                        // If keep-alive is set, switch back to POLLIN for next request
                        for (size_t j = 0; j < poll_fds.size(); ++j) {
                            if (poll_fds[j].fd == current_fd) {
                                poll_fds[j].events = POLLIN;
                                // Reset client request state
                                _clients[current_fd].reset();
                                break;
                            }
                        }
                    } else {
                        // If not keep-alive, close the connection
                        std::cout << "(in pollout) -> Closing connection for client : | " << current_fd << " | " << std::endl;
                        _clients[current_fd].reset();
                        cleanupDisconnectedClient(current_fd);
                    }
                }

                // If res == 0, response is still being sent, continue with POLLOUT
            }
        }
        
        // Handle errors
        if (current_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            if (current_fd != servers[0]->getServerFd()) { // Don't close the server socket
                cleanupDisconnectedClient(current_fd);
            } else {
                std::cerr << "Error on server socket!" << std::endl;
            }
        }
    }
}

//---------------end addeed------------------//
void ConfigFile::run() {
	while (true) {
		handleEvents();
	}
}



void ConfigFile::set_client_map(int client_fd, Client &client)
{
	_clients[client_fd] = client;
}


