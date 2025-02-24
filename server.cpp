/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:16:57 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/24 21:39:10 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <fstream>

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

	// //std::cout << "New connection fd: " << client_fd << " from "
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

	// //std::cout << "Server listening on port " << port << std::endl;
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

void Server::handleClientData(size_t index) 
{
	int client_fd = poll_fds[index].fd;
	char buffer[BUFFER_SIZE];
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
		//std::cout << "Client disconnected, closing connection." << std::endl;
		close(client_fd);
		poll_fds.erase(poll_fds.begin() + index);
		return;
	}
	std::string data(buffer, bytes_read);

	requests[client_fd].body += data; // Append new data to client's buffer
	
	if(!requests[client_fd].getFlagEndOfHeaders())
	{
		if (!requests[client_fd].parseHeader(requests[client_fd].body))
			requests[client_fd].sendErrorResponse(requests[client_fd].getStatusCode());
	}

		
	// Process GET, DELETE, or complete POST request
	if (requests[client_fd].getFlagEndOfHeaders()) {
		if (requests[client_fd].getMethod() == "GET") {
			std::cout << "GET: "  << requests[client_fd].getMethod() << std::endl;
			// handleGET(client_fd, path);
			std::string method = requests[client_fd].getMethod();
			std::string path = requests[client_fd].getpath();
			std::cout << "Method: " << method << std::endl;
			std::cout << "Path: " << path << std::endl;
			std::string file_path = "/Users/hben-laz/Desktop/webserve/docs/html" + path;

			std::ifstream file(file_path.c_str(), std::ios::in | std::ios::binary);
			if (!file) {
				requests[client_fd].sendErrorResponse(404);
				std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
				send(client_fd, response.c_str(), response.length(), 0);
				return;
			}
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string file_content = buffer.str();
			std::string response = "HTTP/1.1 200 OK\r\n";
			response += "Content-Length: " + std::to_string(file_content.size()) + "\r\n";
			response += "Content-Type: text/html\r\n\r\n";
			response += file_content;
			send(client_fd, response.c_str(), response.length(), 0);
			std::cout << "Response: " << response << std::endl;
			
			return ;
		} 
		else if (requests[client_fd].getMethod() == "DELETE") {
			std::cout << "DELETE: "  << requests[client_fd].getMethod() << std::endl;
			// handleDELETE(client_fd, path);
			return ;
		} 
		else {
			std::cout << "POST : "  << requests[client_fd].getMethod() << std::endl;
			std::cout << "bb : "  << requests[client_fd].getBoundary() << std::endl;
			// requests[client_fd] = HTTPRequest(); // Reset for next request
			// Method not implemented (POST will be handled later)
		}
    }	
	// requests[client_fd] = request;
	
}