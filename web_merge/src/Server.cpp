/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:16:57 by aben-cha          #+#    #+#             */
/*   Updated: 2025/03/12 03:23:51 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <algorithm>



#include "../include/Server.hpp"

Server::Server(int port) : port(port) {
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

	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw std::runtime_error("Bind failed");

	if (listen(server_fd, MAX_CLIENTS) < 0)
		throw std::runtime_error("Listen Failed");

	setNonBlocking(server_fd);
	std::cout << "Server fd: " << server_fd << " listening on port " << port << std::endl;
}

Server::~Server() {
	close(server_fd);
}

void Server::setNonBlocking(int fd) {
	fcntl(fd, F_SETFL, O_NONBLOCK);
}

int Server::acceptNewConnection() {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		std::cerr << "Accept failed: " << strerror(errno) << std::endl;
		return -1;
	}

	setNonBlocking(client_fd);
	std::cout << "Server on port " << port << " accepted new connection: FD " << client_fd 
			  << " from " << inet_ntoa(client_addr.sin_addr) 
			  << ":" << ntohs(client_addr.sin_port) << std::endl;
	
	return client_fd;
}

// int Server::handleClientData(int client_fd) {
int Server::handleClientData(int client_fd, Client &client) {
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
// std::cout << "** client_fd: " << client_fd << std::endl;
	if (bytes_read <= 0) {
		if (bytes_read < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
			return -1; // No data available yet
		}
		std::cout << "Client disconnected from server on port " << port << ": FD " << client_fd << std::endl;
		// Don't close the fd here - we'll do it in the main loop
		return -1;
	}

	// std::string data(buffer, bytes_read);
	std::string data(buffer, bytes_read);

	client._requestBuffer += data; // Append new data to client's buffer
	// std::cout << "Server on port " << port << " received data from FD " << client_fd << ": " << client._requestBuffer << std::endl;
	if (client._requestBuffer.size() > MAX_REQUEST_SIZE) {
		std::cerr << "Request too large!" << std::endl;
		client.sendErrorResponse(413, "Request Entity Too Large");
		return -1;
	}
	if (!client.is_Header_Complete())
	{
		if (!client.parse_Header_Request(client._requestBuffer))
			std::cerr << "Error parsing request =====" << std::endl;
			// client.sendErrorResponse(client.getStatusCode());
	}
	else
	{
		if (client._request.getMethod() == "POST")
		{
			// if (!client.parseBody())
			// {
			
			// 	client.sendErrorResponse(client.getStatusCode());
			// }
			std::cout << "POST request received" << std::endl;
		}
		else
			std::cout << "GET request received" << std::endl;
		// else
			client.generateResponse_GET_DELETE();
			std::cout << "Response generated" << std::endl;
			// std::cout << "Response: " << client._responseBuffer << std::endl;
		// Update the interested events to include POLLOUT for writing response
		// for (size_t i = 0; i < _config->poll_fds.size(); i++) {
		//     // std::cout << "====== Client fd ====== " << client._clientFd << std::endl;
		//     std::cout << "====== Client fd ====== " << _config->poll_fds[i].fd << std::endl;
		// 	if (_config->poll_fds[i].fd == client_fd) {
		//         std::cout << "______ Client_fd ________ " << client_fd << std::endl;
		// 		_config->poll_fds[i].events = POLLOUT;
		// 		break;
		// 	}
		// }
					// ssize_t bytesSent = send(client_fd, client._responseBuffer.c_str(), client._responseBuffer.size(), 0);
					
					// if (bytesSent < 0) {
					// 	if (errno == EAGAIN || errno == EWOULDBLOCK) {
					// 		// Would block, try again later
					// 		return true;
					// 	}
						
					// 	std::cerr << "Error sending response: " << strerror(errno) << std::endl;
					// 	return false;
					// }
					
					// if (bytesSent > 0) {
					// 	// Remove sent data from buffer
					// 	client._responseBuffer.erase(0, bytesSent);
					// }
		
		return 1;
	}
	
	// Here you could add code to send a response if needed
	return 0;
}

void Server::setConfig(ConfigFile *config)
{
	_config = config;
}