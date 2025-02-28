/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 23:02:28 by hben-laz          #+#    #+#             */
/*   Updated: 2025/02/28 23:02:31 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <cstring>

volatile bool Server::running = true;

Server::Server() : pollFds(NULL), pollFdsCount(0), pollFdsCapacity(0) {}

Server::~Server() {
    stop();
}

bool Server::initialize(const std::string& configPath) {
    // Parse configuration
    if (!config.parse(configPath)) {
        std::cerr << "Failed to parse config file" << std::endl;
        return false;
    }
    
    // Setup listen sockets
    if (!setupListenSockets()) {
        std::cerr << "Failed to setup listen sockets" << std::endl;
        return false;
    }
    
    // Allocate initial poll array
    pollFdsCapacity = listenSockets.size() + 10; // Some extra capacity
    pollFds = new struct pollfd[pollFdsCapacity];
    pollFdsCount = 0;
    
    // Add listen sockets to poll array
    for (size_t i = 0; i < listenSockets.size(); i++) {
        addPollFd(listenSockets[i], POLLIN);
    }
    
    std::cout << "Server initialized with " << listenSockets.size() << " listen sockets" << std::endl;
    return true;
}

bool Server::setupListenSockets() {
    const std::vector<ServerConfig>& servers = config.getServers();
    
    for (size_t i = 0; i < servers.size(); i++) {
        const ServerConfig& server = servers[i];
        
        // Create and add the listen socket
        int socketFd = createListenSocket(server.host, server.port);
        if (socketFd == -1) {
            // Cleanup on error
            for (size_t j = 0; j < listenSockets.size(); j++) {
                close(listenSockets[j]);
            }
            listenSockets.clear();
            return false;
        }
        
        listenSockets.push_back(socketFd);
    }
    
    return !listenSockets.empty();
}

int Server::createListenSocket(const std::string& host, int port) {
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }
    
    // Set socket to non-blocking
    int flags = fcntl(socketFd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Failed to get socket flags: " << strerror(errno) << std::endl;
        close(socketFd);
        return -1;
    }
    
    if (fcntl(socketFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Failed to set socket non-blocking: " << strerror(errno) << std::endl;
        close(socketFd);
        return -1;
    }
    
    // Enable address reuse
    int opt = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
        close(socketFd);
        return -1;
    }
    
    // Bind socket
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (host == "0.0.0.0" || host.empty()) {
        serverAddr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid host address: " << host << std::endl;
            close(socketFd);
            return -1;
        }
    }
    
    if (bind(socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        close(socketFd);
        return -1;
    }
    
    // Listen for connections
    if (listen(socketFd, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
        close(socketFd);
        return -1;
    }
    
    std::cout << "Listening on " << host << ":" << port << std::endl;
    return socketFd;
}

void Server::run() {
    std::cout << "Server running..." << std::endl;
    
    while (running) {
        // Wait for events using poll
        int numEvents = poll(pollFds, pollFdsCount, 1000); // 1 second timeout
        
        if (numEvents == -1) {
            if (errno == EINTR) {
                // Interrupted by a signal, check if we should continue
                continue;
            }
            
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        }
        
        // Process events
        for (int i = 0; i < pollFdsCount && numEvents > 0; i++) {
            if (pollFds[i].revents == 0) {
                continue;
            }
            
            numEvents--; // Processed one event
            
            int fd = pollFds[i].fd;
            short revents = pollFds[i].revents;
            
            // Check if this is a listen socket
            bool isListenSocket = false;
            for (size_t j = 0; j < listenSockets.size(); j++) {
                if (fd == listenSockets[j]) {
                    isListenSocket = true;
                    break;
                }
            }
            
            if (isListenSocket) {
                if (revents & POLLIN) {
                    acceptNewConnection(fd);
                }
            } else {
                // Handle client I/O
                handleClientIO(fd, revents);
            }
        }
        
        // Check for timed out clients
        std::vector<int> timeoutFds;
        for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->second->hasTimedOut()) {
                timeoutFds.push_back(it->first);
            }
        }
        
        // Remove timed out clients
        for (size_t i = 0; i < timeoutFds.size(); i++) {
            int fd = timeoutFds[i];
            std::cout << "Client timed out: " << fd << std::endl;
            
            delete clients[fd];
            clients.erase(fd);
            removePollFd(fd);
        }
    }
}