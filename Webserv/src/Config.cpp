

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

	servers.push_back(server);
	
	struct pollfd pfd;
	pfd.fd = server->getServerFd();
	pfd.events = POLLIN;
	poll_fds.push_back(pfd);
}

void ConfigFile::cleanupDisconnectedClient(int client_fd) {
	close(client_fd);
	client_server_map.erase(client_fd);
	_clients.erase(client_fd);
	for (size_t j = 0; j < poll_fds.size(); ++j) {
		if (poll_fds[j].fd == client_fd) {
			poll_fds.erase(poll_fds.begin() + j);
			break;
		}
	}
}

//----- start added -----//

void ConfigFile::handleEvents() {
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
                        cleanupDisconnectedClient(current_fd);
                        continue;
                    }
                    else if (res == 0) {
                        continue;
                    }
                    else if (res == 2) {
                        for (size_t j = 0; j < poll_fds.size(); ++j) {
                            if (poll_fds[j].fd == current_fd) {
                                poll_fds[j].events = POLLOUT;
                                break;
                            }
                        }
                    }
                } else {
                    cleanupDisconnectedClient(current_fd);
                }
            }
        }
        else if (current_fds[i].revents & POLLOUT) {
            if (client_server_map.find(current_fd) != client_server_map.end()) {
                _clients[current_fd].updateActivityTime();
                int owner_server = client_server_map[current_fd];
                int res = servers[owner_server]->sendResponse(current_fd, _clients[current_fd]);
                if (res < 0) {
                    cleanupDisconnectedClient(current_fd);
                }
                else if (res == 1) {
                    if (_clients[current_fd].keepAlive()) {

                        for (size_t j = 0; j < poll_fds.size(); ++j) {
                            if (poll_fds[j].fd == current_fd) {
                                poll_fds[j].events = POLLIN;
                                _clients[current_fd]._request.endOfRequest = false;
                                _clients[current_fd].endOfResponse = true;
                                _clients[current_fd].reset();
                                _clients[current_fd].updateActivityTime();
                                break;
                            }
                        }
                    } else {
                        _clients[current_fd].reset();
                        cleanupDisconnectedClient(current_fd);
                    }
                }
            }
        }
        
        // Handle errors
        if (current_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            if (current_fd != servers[0]->getServerFd()) { // Don't close the server socket
                cleanupDisconnectedClient(current_fd);
            } 
            // else {
            //     // std::cerr << "Error on server socket!" << std::endl;
            // }
        }
    }

    std::vector<int> clients_to_disconnect;
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        int fd = it->first;
        Client &client = it->second;
        
        if (now - client.getLastActivityTime() > 30) {
            clients_to_disconnect.push_back(fd);
        }
    }
    
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
            _clients[fd].endOfResponse = false;
            cleanupDisconnectedClient(fd);
        }
        else {
            std::string timeoutResp = oss.str();
            _clients[fd]._request.set_status_code(408);
            int owner_server = client_server_map[fd];
            servers[owner_server]->sendResponse(fd, _clients[fd]);
            cleanupDisconnectedClient(fd);
        }
    }
}


void handleStopSignal(int signum) {
    (void)signum; // Ignore the signal
    // std::cout << "SIGTSTP (Ctrl+Z) received. Ignoring suspension." << std::endl;
}

void ConfigFile::run() {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTSTP, handleStopSignal);

    while (true) {
        handleEvents();
    }
}

void ConfigFile::set_client_map(int client_fd, Client &client)
{
	_clients[client_fd] = client;
}