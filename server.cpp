/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:16:57 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/22 18:23:50 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::~Server() {
    for (size_t i = 0; i < poll_fds.size(); ++i) {
        close(poll_fds[i].fd);
    }
}

void Server::setNonBlocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

void Server::handleNewConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return;
    }

    setNonBlocking(client_fd);

    struct pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    poll_fds.push_back(pfd);

    // std::cout << "New connection fd: " << client_fd << " from "
    //     << inet_ntoa(client_addr.sin_addr)
    //           << ":" << ntohs(client_addr.sin_port) << std::endl;
}

Server::Server(int port)
{
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw std::runtime_error("Socket creation failed");

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        throw std::runtime_error("Setsockopt failed");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("Bind failed");

    if (listen(server_fd, MAX_CLIENTS) < 0)
        throw std::runtime_error("Listen Failed");

    setNonBlocking(server_fd);

    struct pollfd pfd;
    pfd.fd = server_fd;
    pfd.events = POLLIN;
    poll_fds.push_back(pfd);

    // std::cout << "Server listening on port " << port << std::endl;
}

void Server::run() {
    while (true) {
        int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);

        if (poll_count < 0) {
            std::cerr << "Poll failed: " << strerror(errno) << std::endl;
            continue;
        }
        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (!(poll_fds[i].revents & POLLIN)) {
                continue;
            }
            if (poll_fds[i].fd == server_fd) {
                handleNewConnection();
            } else {
                handleClientData(i);
            }
        }
    }
}

// Server::~Server() {
// 	for (std::size_t i = 0; i < poll_fds.size(); i++) {
// 		close(poll_fds[i].fd);
// 	}
// }

// void Server::setNonBlocking(int fd) {
// 	fcntl(fd, F_SETFL, O_NONBLOCK);
// }
		
// Server::Server(int port) {
// 	struct sockaddr_in server_addr;
	
// 	server_fd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (server_fd < 0)
// 		throw std::runtime_error("Socket creation failed");
	
// 	int reuse = 1;
// 	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
// 		throw std::runtime_error("Setsockopt failed");

// 	server_addr.sin_family = AF_INET;
// 	server_addr.sin_port = htons(port);
// 	server_addr.sin_addr.s_addr = INADDR_ANY;

// 	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
// 		throw std::runtime_error("Bind failed");

// 	if (listen(server_fd, MAX_CLIENTS) < 0)
// 		throw std::runtime_error("Listen Failed");
	
// 	setNonBlocking(server_fd);

// 	struct pollfd pfd;
// 	pfd.fd = server_fd;
// 	pfd.events = POLLIN;
// 	poll_fds.push_back(pfd);
	
// 	std::cout << "Server listening on port " << port << std::endl;
// }
		
// void Server::handleNewConnection() {
// 	struct sockaddr_in client_addr;
// 	socklen_t client_len = sizeof(client_addr);
// 	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
// 	if (client_fd < 0) {
// 		std::cerr << "Accept failed: " << strerror(errno) << std::endl;
// 		return ;
// 	}

// 	setNonBlocking(client_fd);

// 	struct pollfd pfd;
// 	pfd.fd = client_fd;
// 	pfd.events = POLLIN;
// 	poll_fds.push_back(pfd);

// 	std::cout << "New connection fd: " << client_fd <<  " from " 
// 			  << inet_ntoa(client_addr.sin_addr)
// 			  << ":" << ntohs(client_addr.sin_port) << std::endl;
// }


// void Server::run() {
// 	while (1) {
// 		int poll_count = poll(&poll_fds[0], poll_fds.size(), -1);
// 		if (poll_count < 0) {
// 			std::cerr << "Poll failed: " << strerror(errno) << std::endl;
// 			continue ;
// 		}
// 		for (std::size_t i = 0; i < poll_fds.size(); ++i) {
// 			if (!(poll_fds[i].revents & POLLIN))
// 				continue ;
					
// 			if (poll_fds[i].fd == server_fd)
// 				handleNewConnection();
// 			else
// 				handleClientData(i);
// 		}
// 	}
// }

// ----------------------------------------------
void Server::handleClientData(size_t index) {
    int client_fd = poll_fds[index].fd;
    char buffer[BUFFER_SIZE];
    HTTPRequest request;
    ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0); // Read 49 bytes at a time

    if (bytes_read < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return; // No data available, wait for next poll()
        }
        std::cerr << "Recv failed: " << strerror(errno) << std::endl;
        close(client_fd);
        poll_fds.erase(poll_fds.begin() + index);
        return;
    }
    
    if (bytes_read == 0) {
        std::cout << "Client disconnected, closing connection." << std::endl;
        close(client_fd);
        poll_fds.erase(poll_fds.begin() + index);
        return;
    }
    std::string data(buffer, bytes_read);
    std::cout << data;
	clientBuffers[client_fd] += data; // Append new data to client's buffer

	if(!requests[client_fd].getFlagEndOfHeaders())
	{
			if (!requests[client_fd].parseHeader(clientBuffers[client_fd])) {
				requests[client_fd].sendErrorResponse(requests[client_fd].getStatusCode());
				std::cout << requests[client_fd].getStatusCodeMessage() << std::endl;
			}
	}
	// body
	// if (requests[client_fd].getFlagEndOfHeaders())
	// {
	// 	if (!requests[client_fd].getBodyFlag())
	// 	{
	// 		// Check for end of body
	// 		size_t bodyEndPos = clientBuffers[client_fd].find("\r\n\r\n");
	// 		if (bodyEndPos != std::string::npos) {
	// 			requests[client_fd].setBodyFlag(true);
	// 			std::cout << "End of bodys" << std::endl;
	// 		}

	// 		while (1) 
	// 		{
	// 			size_t lineEnd = clientBuffers[client_fd].find("\r\n");
	// 			if (lineEnd == std::string::npos) {	
	// 				break; // Wait for more data
	// 			}
	// 			std::string line = clientBuffers[client_fd].substr(0, lineEnd);
	// 			clientBuffers[client_fd].erase(0, lineEnd + 2); // Remove processed line

	// 				if (line.empty()) {
	// 					std::cout << "End of body" << std::endl;
						
	// 					// Handle POST request body
	// 					requests[client_fd].setBody(requests[client_fd].getBody() + line);	
	// 					break;
	// 				}
	// 				requests[client_fd].setBody(requests[client_fd].getBody() + line);
	// 		}
	// 	}
	// }
		
// Process GET, DELETE, or complete POST request
	if (requests[client_fd].getFlagEndOfHeaders()) {

			// handleRequest(client_fd, requests[client_fd]);
			std::cout << "==================================================" << std::endl;
			std::cout << "Body: " << requests[client_fd].getBody() << std::endl;
			std::cout << "==================================================" << std::endl;
			
			requests[client_fd].setFlagEndOfHeaders(false);
			
			requests[client_fd] = HTTPRequest(); // Reset for next request
		}	
}