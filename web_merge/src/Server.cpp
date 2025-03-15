
#include "../include/web.h"

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

int Server::handleClientData(int client_fd, Client &client) {
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (bytes_read <= 0) {
		if (bytes_read < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
			return -1; // No data available yet
		}
		std::cout << "Client disconnected from server on port " << port << ": FD " << client_fd << std::endl;
		// Don't close the fd here - we'll do it in the main loop
		client._request.set_status_code(413);
		return -1;
	}

	std::string data(buffer, bytes_read);

	client._requestBuffer += data; // Append new data to client's buffer
	if (client._requestBuffer.size() > MAX_REQUEST_SIZE) {
		client._request.set_status_code(413);
		return -1;
	}
	if (!client.is_Header_Complete())
	{
		if (!client.parse_Header_Request(client._requestBuffer))
		{
			std::cerr << "Error parsing request =====" << std::endl;
			// client.genetate_error_response(413, "Request Entity Too Large", client_fd);
		}
			// client.genetate_error_response(client.getStatusCode());
	}
	if (client.is_Header_Complete())
	{
		
		std::cout << "Request header complete" << std::endl;
		if (client._request.getMethod() == "POST")
		{
			// if (!client.parseBody())
			// {
			
			// 	client.genetate_error_response(client.getStatusCode());
			// }
			std::cout << "POST request received" << std::endl;
		}
		else if (client._request.getMethod() == "GET" || client._request.getMethod() == "DELETE")
		{
			std::cout << "GET request received" << std::endl;
			
			client.generateResponse_GET_DELETE();
		}
		if (client._request.getStatusCode() != 200)
		{
			client.genetate_error_response(client._request.getStatusCode(), client_fd);
		}
		// else
			std::cout << "Response generated" << std::endl;

					ssize_t bytesSent = send(client_fd, client._responseBuffer.c_str(), client._responseBuffer.size(), 0);
					
					if (bytesSent < 0) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							// Would block, try again later
							return true;
						}
						
						std::cerr << "Error sending response: " << strerror(errno) << std::endl;
						return false;
					}
					
					if (bytesSent > 0) {
						// Remove sent data from buffer
						client._responseBuffer.erase(0, bytesSent);
					}
		
		return 1;
	}
	
	return 0;
}

void Server::setConfig(ConfigFile *config)
{
	_config = config;
}