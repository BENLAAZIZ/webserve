/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:16:57 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/20 22:47:00 by hben-laz         ###   ########.fr       */
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


// void Server::handleClientData(std::size_t index) {
	
// 	char buffer[BUFFER_SIZE];
// 	int client_fd = pollfds[index].fd;
	
// 	std::size_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
// 	if (bytes_read <= 0) {
// 		if (bytes_read == 0) {
// 			std::cout << "client disconnected fd: " << client_fd << std::endl;
// 		} else {
// 			std::cerr << "Receive error: " << strerror(errno) << std::endl;
// 		}
// 		close(client_fd);
// 		pollfds.erase(pollfds.begin() + index);
// 		clientBuffers.erase(client_fd);  // Clean up client buffer
// 		return;
// 	}

// 	buffer[bytes_read] = '\0';
// 	clientBuffers[client_fd] += buffer; // Append new data to client's buffer
// 	// Process data line by line

// 	// std::size_t lineEnd;
// 	while (1) 
// 	{
// 		// Find next line ending
// 		size_t lineEnd = clientBuffers[client_fd].find("\r\n");
// 		if (lineEnd == std::string::npos) {
// 			// std::cout << "No complete line found, wait for more data" << std::endl;
// 			break; // No complete line found, wait for more data
// 		}
// 		// std::cout << "Complete line found, process it" << std::endl;
// 		// std::cout << "Line: " << clientBuffers[client_fd].substr(0, lineEnd) << std::endl;

// 		std::string line = clientBuffers[client_fd].substr(0, lineEnd);
// 		clientBuffers[client_fd].erase(0, lineEnd + 2); // Remove processed line

// 		// 1 Parse request line (first line)
// 		if (requests[client_fd].getMethod().empty()) 
// 		{
// 			std::istringstream iss(line);
// 			std::string method, path, version;
// 			if (!(iss >> method >> path >> version)) {
// 				requests[client_fd].sendErrorResponse(400); // Bad Request
// 				return;
// 			}
// 			if (method != "GET" && method != "POST" && method != "DELETE") {
// 				requests[client_fd].sendErrorResponse(405); // Method Not Allowed
// 				return;
// 			}
// 			if (path.empty() || path[0] != '/' || version != "HTTP/1.1") {
// 				requests[client_fd].sendErrorResponse(400); // Bad Request
// 				return;
// 			}

// 			// Store parsed values
// 			requests[client_fd].setMethod(method);
// 			requests[client_fd].setPath(path);
// 			requests[client_fd].setVersion(version);

// 			std::cout << "Method: " << method << "\nPath: " << path << "\nVersion: " << version << std::endl;
// 		}

// 		// 2️ Parse headers (starting from second line)
// 		else 
// 		{

// 			if (line.empty()) 
// 			{
// 				// End of headers reached, validate Host header
// 				if (requests[client_fd].getHeaders().find("Host") == requests[client_fd].getHeaders().end()) {
// 					requests[client_fd].sendErrorResponse(400); // Bad Request: Missing Host Header
// 					std::cout << "-- Host header missing 400 --" << std::endl;
// 					return;
// 				}

// 				// Handle POST request body
// 				if (requests[client_fd].getMethod() == "POST") {
// 					std::string contentLength = requests[client_fd].getHeader("Content-Length");
// 					if (contentLength.empty()) {
// 						requests[client_fd].sendErrorResponse(411); // Length Required
// 						std::cout << "-- Content-Length header missing 411 --" << std::endl;
// 						return;
// 					}
// 					requests[client_fd].setContentLength(std::atoi(contentLength.c_str()));
// 				} else {
// 					// Process GET or DELETE request immediately
// 					// handleRequest(client_fd, requests[client_fd]);
// 					requests[client_fd] = HTTPRequest(); // Reset for next request
// 				}
// 				break;
// 			}
// 			// Validate Host header (must be second line)
// 			std::string hostHeader;
// 			if (requests[client_fd].getHeaders().empty() && line.substr(0, 5) != "Host:")
// 			{
// 				hostHeader = line.substr(0, 5);
// 				std::cout << "---> Host header: " << hostHeader << std::endl;
// 				requests[client_fd].sendErrorResponse(400); // Bad Request: Invalid second line
// 				std::cout << "-- Invalid second line 400 --" << std::endl;
// 				return;
// 			}
// 			// 3️ Store header in map
// 			size_t colonPos = line.find(":");
// 			if (colonPos == std::string::npos || colonPos == 0 || line[colonPos - 1] == ' ') {
// 				requests[client_fd].sendErrorResponse(400); // Bad Request: Malformed header
// 				std::cout << "-- Malformed header 400 --" << std::endl;
// 				return;
// 			}

// 			std::string key = line.substr(0, colonPos);
// 			std::string value = line.substr(colonPos + 1);
// 			value.erase(0, value.find_first_not_of(" ")); // Trim leading spaces

// 			requests[client_fd].setHeader(key, value);
// 			std::cout << "Header: " << key << " = " << value << std::endl;
// 		}
// 	// aficher all request client
// 	std::cout << "================================================" << std::endl;
// 	std::cout << "Request: " << requests[client_fd].getMethod() << " " << requests[client_fd].getpath() << std::endl;
// 	}
// }
//=============================================================================================
//=============================================================================================
//=============================================================================================
//=============================================================================================
//=============================================================================================
//=============================================================================================
// void Server::handleClientData(std::size_t index) {
// 	char buffer[BUFFER_SIZE];
// 	int client_fd = pollfds[index].fd;

// 	std::size_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
// 	if (bytes_read <= 0) {
// 		if (bytes_read == 0) {
// 			std::cout << "Client disconnected fd: " << client_fd << std::endl;
// 		} else {
// 			std::cerr << "Receive error: " << strerror(errno) << std::endl;
// 		}
// 		close(client_fd);
// 		pollfds.erase(pollfds.begin() + index);
// 		clientBuffers.erase(client_fd);  // Clean up client buffer
// 		return;
// 	}

// 	buffer[bytes_read] = '\0';
// 	clientBuffers[client_fd] += buffer; // Append new data to client's buffer
	
// 	// Process data line by line
// 	while (1) {
// 		size_t lineEnd = clientBuffers[client_fd].find("\r\n");
// 		if (lineEnd == std::string::npos) {
// 			std::cout << "No complete line found, wait for more data" << std::endl;
// 			// pause();
// 			break; // Wait for more data
// 		}
// 		else
// 			std::cout << "Complete line found, process it" << std::endl;
// 		std::string line = clientBuffers[client_fd].substr(0, lineEnd);
// 		clientBuffers[client_fd].erase(0, lineEnd + 2); // Remove processed line


// 		if (requests[client_fd].getMethod().empty() || requests[client_fd].getpath().empty() || requests[client_fd].getVersion().empty()) {
// 			if (!requests[client_fd].parseFirstLine(line)) {
// 				// requests[client_fd].sendErrorResponse(400);
// 				std::cout << "400 Bad Request " << std::endl;
// 				break;
// 			}
// 			std::cout << "----------------->First Line method: |" << requests[client_fd].getMethod() << "|" << std::endl;
// 		}
		

// 		// 2. Parse Headers (Following Lines)
// 		else {
// 			// End of headers, check for Host header
// 			std::string hostHeader;
// 			if (line.empty()) // End of headers
// 			{
// 				flag_end_of_headers = true;
// 				std::cout << "End of headers" << std::endl;
// 				if (requests[client_fd].getHeaders().find("host") == requests[client_fd].getHeaders().end()) {
// 					requests[client_fd].sendErrorResponse(400); // Bad Request: Missing Host Header
// 					std::cout << "-- Host header missing 400 --" << std::endl;
// 					return;
// 				}
 
// 				// Handle POST request body
// 				if (requests[client_fd].getMethod() == "POST") {
// 					std::string contentLength = requests[client_fd].getHeader("Content-Length");
// 					if (contentLength.empty()) {
// 						requests[client_fd].sendErrorResponse(411); // Length Required
// 						std::cout << "-- Content-Length header missing 411 --" << std::endl;
// 						return;
// 					}
// 					requests[client_fd].setContentLength(std::atoi(contentLength.c_str()));
// 					std::cout << "**Content-Length:** " << requests[client_fd].getContentLength() << std::endl;
// 				} 
// 				// else {
// 				// 	// Process GET or DELETE request immediately
// 				// 	// handleRequest(client_fd, requests[client_fd]);
// 				// 	requests[client_fd] = HTTPRequest(); // Reset for next request
// 				// }
// 				break;
// 			}
// 			// 3. Store Header in Map
// 			size_t colonPos = line.find(":");
// 			if (colonPos == std::string::npos || colonPos == 0 || line[colonPos - 1] == ' ') {
// 				requests[client_fd].sendErrorResponse(400); // Malformed header
// 				std::cout << "-- Ma lformed header 400 --" << std::endl;
// 				return;
// 			}
// 			hostHeader = line.substr(0, colonPos);
// 			std::string key;
// 			std::transform(hostHeader.begin(), hostHeader.end(), hostHeader.begin(), ::tolower);
// 			if (hostHeader == "host")
// 			{
// 				key = hostHeader.substr(0, colonPos);
// 				std::cout << "Host header: " << key << std::endl;
// 			}
// 			else
// 				key = line.substr(0, colonPos);
// 			std::string value = line.substr(colonPos + 1);
// 			value.erase(0, value.find_first_not_of(" ")); // Trim leading spaces

// 			requests[client_fd].setHeader(key, value);
// 			std::cout << "Header: ||" << key << "|| = ||" << value << "||" << std::endl;
// 		}
// 	}
// 	// Process GET or DELETE or POST request immediately
// 	if (flag_end_of_headers) {
// 		// handleRequest(client_fd, requests[client_fd]);
// 		requests[client_fd] = HTTPRequest(); // Reset for next request
// 		flag_end_of_headers = false;
// 	}
// }


// ----------------------------------------------

void Server::handleClientData(std::size_t index) {
	char buffer[BUFFER_SIZE];
	int client_fd = pollfds[index].fd;

	std::size_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	if (bytes_read <= 0) {
		if (bytes_read == 0) {
			std::cout << "Client disconnected fd: " << client_fd << std::endl;
		} else {
			std::cerr << "Receive error: " << strerror(errno) << std::endl;
		}
		close(client_fd);
		pollfds.erase(pollfds.begin() + index);
		clientBuffers.erase(client_fd);  // Clean up client buffer
		requests.erase(client_fd);
		return;
	}

	buffer[bytes_read] = '\0';
	clientBuffers[client_fd] += buffer; // Append new data to client's buffer

	// Check for end of headers ("\r\n\r\n")
	if(!requests[client_fd].getFlagEndOfHeaders())
	{
		size_t headerEndPos = clientBuffers[client_fd].find("\r\n\r\n");
		if (headerEndPos != std::string::npos) {
			requests[client_fd].setFlagEndOfHeaders(true);
			std::cout << "End of headers" << std::endl;
		}

		// Process data line by line
		while (1) {
			size_t lineEnd = clientBuffers[client_fd].find("\r\n");
			if (lineEnd == std::string::npos) {
				break; // Wait for more data
			}
			std::string line = clientBuffers[client_fd].substr(0, lineEnd);
			clientBuffers[client_fd].erase(0, lineEnd + 2); // Remove processed line

			if (requests[client_fd].getMethod().empty() || requests[client_fd].getpath().empty() || requests[client_fd].getVersion().empty()) {
				if (!requests[client_fd].parseFirstLine(line)) {
					std::cout << "400 Bad Request" << std::endl;
					return;
				}
				std::cout << "----------------->First Line method: |" << requests[client_fd].getMethod() << "|" << std::endl;
			}
			else {
				if (line.empty()) {
					std::cout << "End of headers" << std::endl;
					if (requests[client_fd].getHeaders().find("host") == requests[client_fd].getHeaders().end()) {
						requests[client_fd].sendErrorResponse(400);
						std::cout << "-- Host header missing 400 --" << std::endl;
						return;
					}


					break;
				}

				size_t colonPos = line.find(":");
				if (colonPos == std::string::npos || colonPos == 0 || line[colonPos - 1] == ' ') {
					requests[client_fd].sendErrorResponse(400);
					std::cout << "-- Malformed header 400 --" << std::endl;
					return;
				}
				std::string key = line.substr(0, colonPos);
				std::transform(key.begin(), key.end(), key.begin(), ::tolower);
				std::string value = line.substr(colonPos + 1);
				value.erase(0, value.find_first_not_of(" "));

				requests[client_fd].setHeader(key, value);
				std::cout << "Header: ||" << key << "|| = ||" << value << "||" << std::endl;
			}
		}

		// Process GET, DELETE, or complete POST request
		// if (requests[client_fd].getFlagEndOfHeaders()) {

		// 	// handleRequest(client_fd, requests[client_fd]);
		// 	requests[client_fd] = HTTPRequest(); // Reset for next request
		// 	requests[client_fd].setFlagEndOfHeaders(false);
		// }
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
	// if (requests[client_fd].getFlagEndOfHeaders() && requests[client_fd].getBodyFlag()) {
	if (requests[client_fd].getFlagEndOfHeaders()) {

			// handleRequest(client_fd, requests[client_fd]);
			std::cout << "==================================================" << std::endl;
			std::cout << "Body: " << requests[client_fd].getBody() << std::endl;
			requests[client_fd].setFlagEndOfHeaders(false);
			requests[client_fd] = HTTPRequest(); // Reset for next request
		}
		
}