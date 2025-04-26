

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

void ConfigFile::addServer(int port, Server_holder& serv_hldr) {
	Server* server = new Server(port, serv_hldr);
    server->serv_hldr = serv_hldr;
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
	_clients.erase(client_fd); // Remove from _clients map
	// Remove from poll_fds
	for (size_t j = 0; j < poll_fds.size(); ++j) {
		if (poll_fds[j].fd == client_fd) {
			poll_fds.erase(poll_fds.begin() + j);
			break;
		}
	}
}

//----- start added -----//

void ConfigFile::handleEvents() {
    // int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
    int poll_count = poll(poll_fds.data(), poll_fds.size(), 1000); // 
    if (poll_count < 0) {
        std::cerr << "Poll failed: " << strerror(errno) << std::endl;
        exit(1);
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
                    client.updateActivityTime();
                    set_client_map(client_fd, client);
                }
            } else { // This is a client, handle the request data

                if (client_server_map.find(current_fd) != client_server_map.end()) {
                    _clients[current_fd].updateActivityTime();
                    int res = 0;
                    int owner_server = client_server_map[current_fd];

                    res = servers[owner_server]->handleClientData(current_fd, _clients[current_fd]);
                    
                    if (res < 0) {
                        // Client disconnected or error
                        std::cerr << "Client disconnected" << std::endl;
                        cleanupDisconnectedClient(current_fd);
                        continue;
                    }
                    else if (res == 0) {
                        // Request still being processed, continue to read
                        continue;
                    }
                    else if (res == 2) {
                        // Request fully received, switch to POLLOUT to send response
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
                _clients[current_fd].updateActivityTime();
                int owner_server = client_server_map[current_fd];
                // sendSuccessResponse(current_fd);
                // std::cout << "Sending response to client" << std::endl;
                int res = servers[owner_server]->sendResponse(current_fd, _clients[current_fd]);
                // std::cout << "res: " << res << std::endl;
                if (res < 0) {
                    // Error sending response
                    std::cerr << "Error sending response to client" << std::endl;
                    cleanupDisconnectedClient(current_fd);
                }
                else if (res == 1) {
                    // Response fully sent, reset to POLLIN for next request or close connection
                    if (_clients[current_fd].keepAlive()) {
                        // std::cout << "Keep-alive: waiting for next request" << std::endl;
                        // If keep-alive is set, switch back to POLLIN for next request
                        std::cout << "------[Keep-alive]-----" << std::endl;

                        for (size_t j = 0; j < poll_fds.size(); ++j) {
                            if (poll_fds[j].fd == current_fd) {
                                poll_fds[j].events = POLLIN;
                                _clients[current_fd]._request.endOfRequest = false;
                                // Reset client request state
                                _clients[current_fd].endOfResponse = true;
                                _clients[current_fd].reset();
                                _clients[current_fd].updateActivityTime();
                                break;
                            }
                        }
                    } else {
                        std::cout << "------Closing connection-----" << std::endl;
                        _clients[current_fd].reset();
                        // If not keep-alive, close the connection
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

    // --- Timeout check for inactive clients ---

    // Check for client timeouts
    // Use a separate vector to store clients to be disconnected
    std::vector<int> clients_to_disconnect;
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec; // Use seconds consistently
    
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        int fd = it->first;
        Client &client = it->second;
        
        // Check if client has been idle for more than 60 seconds
        // Assuming getLastActivityTime() returns seconds
        if (now - client.getLastActivityTime() > 60) {
            clients_to_disconnect.push_back(fd);
        }
    }
    
    // Disconnect idle clients after iterating through the map
    for (size_t i = 0; i < clients_to_disconnect.size(); ++i) {
        int fd = clients_to_disconnect[i];

        std::string body = "<html><head><title>408 Request Timeout</title></head>"
                "<body><h1>408 Request Timeout</h1><p>Your session timed out due to inactivity.</p></body></html>";

        std::ostringstream oss;
        oss << "HTTP/1.1 408 Request Timeout\r\n";
        oss << "Content-Type: text/html\r\n";
        oss << "Content-Length: " << body.size() << "\r\n";
        oss << "Connection: close\r\n";
        oss << "\r\n";
        oss << body;
        
        if (_clients[fd].endOfResponse) {
            std::cout << "------[Timeout just close the connection ]-----" << std::endl;
            _clients[fd].endOfResponse = false;
            cleanupDisconnectedClient(fd);
        }
        else {
            std::cout << "------[Timeout Sending 408 Request Timeout response ]-----" << std::endl;
            std::string timeoutResp = oss.str();
            // send(fd, timeoutResp.c_str(), timeoutResp.size(), 0);
            _clients[fd]._request.set_status_code(408);
            // std::cout << "timeoutResp: " << timeoutResp << std::endl;
            // std::cout << "status code : " << _clients[fd]._request.getStatusCode() << std::endl;
            int owner_server = client_server_map[fd];
            servers[owner_server]->sendResponse(fd, _clients[fd]);
            cleanupDisconnectedClient(fd);
        }
    }
}

//---------------end addeed------------------//
// void ConfigFile::run() {
//     signal(SIGPIPE, SIG_IGN);
// 	while (true) {
// 		handleEvents();
// 	}
// }


void handleStopSignal(int signum) {
    (void)signum; // Ignore the signal
    std::cout << "SIGTSTP (Ctrl+Z) received. Ignoring suspension." << std::endl;
}

void ConfigFile::run() {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTSTP, handleStopSignal);  // Custom handler

    while (true) {
        handleEvents();
    }
}

void ConfigFile::set_client_map(int client_fd, Client &client)
{
	_clients[client_fd] = client;
}