/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:16:57 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/16 18:47:37 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::~Server() {
	for (std::size_t i = 0; i < pollfds.size(); i++) {
		close(pollfds[i].fd);
	}
}

void Server::setNonBlocking(int fd) {
	fcntl(fd, F_SETFL, O_NONBLOCK);
}
		
Server::Server(int port) {
	struct sockaddr_in server_addr;
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		throw std::runtime_error("Socket creation failed");
	
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
		throw std::runtime_error("Setsockopt failed");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		throw std::runtime_error("Bind failed");

	if (listen(server_fd, MAX_CLIENTS) < 0)
		throw std::runtime_error("Listen Failed");
	
	setNonBlocking(server_fd);

	struct pollfd pfd;
	pfd.fd = server_fd;
	pfd.events = POLLIN;
	pollfds.push_back(pfd);
	
	std::cout << "Server listening on port " << port << std::endl;
}
		
void Server::handleNewConnection() {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		std::cerr << "Accept failed: " << strerror(errno) << std::endl;
		return ;
	}

	setNonBlocking(client_fd);

	struct pollfd pfd;
	pfd.fd = client_fd;
	pfd.events = POLLIN;
	pollfds.push_back(pfd);

	std::cout << "New connection fd: " << client_fd <<  " from " 
			  << inet_ntoa(client_addr.sin_addr)
			  << ":" << ntohs(client_addr.sin_port) << std::endl;
}


void Server::run() {
	while (1) {
		int poll_count = poll(&pollfds[0], pollfds.size(), -1);
		if (poll_count < 0) {
			std::cerr << "Poll failed: " << strerror(errno) << std::endl;
			continue ;
		}
		for (std::size_t i = 0; i < pollfds.size(); ++i) {
			if (!(pollfds[i].revents & POLLIN))
				continue ;
					
			if (pollfds[i].fd == server_fd)
				handleNewConnection();
			else
				handleClientData(i);
		}
	}
}


void Server::handleClientData(std::size_t index) {
	
	char buffer[BUFFER_SIZE];
	int client_fd = pollfds[index].fd;
	
	std::size_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	if (bytes_read <= 0) {
		if (bytes_read == 0) {
			std::cout << "client disconnected fd: " << client_fd << std::endl;
		} else {
			std::cerr << "Receive error: " << strerror(errno) << std::endl;
		}
		close(client_fd);
		pollfds.erase(pollfds.begin() + index);
		clientBuffers.erase(client_fd);  // Clean up client buffer
		return;
	}

	buffer[bytes_read] = '\0';
	clientBuffers[client_fd] += buffer; // Append new data to client's buffer
	// Process data line by line

	// std::size_t lineEnd;
	while (1)
	{
		 // Find next line ending
		size_t lineEnd = clientBuffers[client_fd].find("\r\n");
		if (lineEnd == std::string::npos) 
		{
			break; // No complete line found, wait for more data
		}
		 // Extract the line
		std::string line = clientBuffers[client_fd].substr(0, lineEnd);
		// Remove processed line from buffer (including \r\n)
		clientBuffers[client_fd].erase(0, lineEnd + 2);

		 // 1️ Parse request line (first line)
		if (requests[client_fd].getMethod().empty())
		{
			std::istringstream iss(line);
			std::string method, path, version;
			if (!(iss >> method >> path >> version)) {
				requests[client_fd].sendErrorResponse(400); // Bad Request
				return;
			}
			if (method != "GET" && method != "POST" && method != "DELETE") {
				requests[client_fd].sendErrorResponse(405); // Method Not Allowed
				return;
			}
			if (path.empty() || path[0] != '/' || version != "HTTP/1.1") {
				requests[client_fd].sendErrorResponse(400); // Bad Request
				return;
			}

			// Store parsed values
			requests[client_fd].setMethod(method);
			requests[client_fd].setPath(path);
			requests[client_fd].setVersion(version);

			std::cout << "Method: " << method << "\nPath: " << path << "\nVersion: " << version << std::endl;
			std::cout << " ------------------- " << std::endl;
		}
		else
		{
		// 2️ Parse headers (starting from second line)
			if (line.empty()) 
			{
				// End of headers reached, validate Host header
				
				if (requests[client_fd].getHeaders().find("Host") == requests[client_fd].getHeaders().end()) 
				{
					requests[client_fd].sendErrorResponse(400); // Bad Request: Missing Host Header
					return;
				}
			}
			// Handle POST request body
			 if (requests[client_fd].getMethod() == "POST")
			{
				std::string contentLength = requests[client_fd].getHeader("Content-Length");
				if (contentLength.empty()) {
					requests[client_fd].sendErrorResponse(411); // Length Required
					return;
				}
				requests[client_fd].setContentLength(std::atoi(contentLength.c_str()));
			}
			else 
			{
				// Process GET or DELETE request immediately
				handleRequest(client_fd, requests[client_fd]);
				requests[client_fd] = HTTPRequest(); // Reset for next request
			}
			break;
		}
		// Validate Host header (must be second line)
		if (requests[client_fd].getHeaders().empty() && line.substr(0, 5) != "Host:") 
		{
			requests[client_fd].sendErrorResponse(400); // Bad Request: Invalid second line
			return;
		}
		// 3 Store header in map







		
	}

}