
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

// int Server::handleClientData(int client_fd, Client &client) {
// 	char buffer[BUFFER_SIZE];
// 	ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
// 	if (bytes_read <= 0) {
// 		if (bytes_read < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
// 			return -1; // No data available yet
// 		}
// 		std::cout << "Client disconnected from server on port " << port << ": FD " << client_fd << std::endl;
// 		// Don't close the fd here - we'll do it in the main loop
// 		client.sendErrorResponse(413, "Request Entity Too Large");
// 		return -1;
// 	}

// 	std::string data(buffer, bytes_read);

// 	client._request._requestBuffer += data; // Append new data to client's buffer
// 	if (client._request._requestBuffer.size() > MAX_REQUEST_SIZE) {
// 		client.sendErrorResponse(413, "Request Entity Too Large");
// 		return -1;
// 	}
// 	if (!client.is_Header_Complete())
// 	{
// 		if (!client.parse_Header_Request(client._request._requestBuffer))
// 			std::cerr << "Error parsing request =====" << std::endl;
// 			// client.sendErrorResponse(client.getStatusCode());
// 	}
// 	else
// 	{
// 		// std::cout << "Request header complete" << std::endl;
// 		if (client._request.getMethod() == "POST")
// 		{
// 			// std::cout << "POST request received" << std::endl;
// 			client.handlePostRequest();
// 			// std::cout << client._request._requestBuffer << std::endl;
// 			return 0;
// 		}
// 		// else
// 		// {
// 		// 	std::cout << "GET request received" << std::endl;
// 		// 	client.generateResponse_GET_DELETE();
// 		// }
// 		// else
// 		// 	std::cout << "Response generated" << std::endl;
// 		// return 1;
// 	}
	
// 	return 0;
// }

int Server::handleClientData(int client_fd, Client &client) {
	client.setClientFd(client_fd);
	if (!client.is_Header_Complete())
	{
		if (client.read_data())
			return -1;
		if (!client.is_Header_Complete())
			client.parse_Header_Request();
		if (client.is_Header_Complete() && client._request.getMethod() != "POST") {
			client._request.endOfRequest = true;
			return 2;
		}
		return 0;
	}
	if (client.is_Header_Complete() && client._request.getMethod() == "POST")
	{
		if (!client._request.endOfRequest) {
			if (client.read_data())
				return -1;
		}
			client.handlePostRequest();
	}
	if (client._request.endOfRequest) {
		std::cout << "End of request" << std::endl;
		return 2;
	}
	return 0;
}

int Server::sendResponse(int client_fd, Client &client) {
	// std::cout << "Sending response to client FD " << client_fd << std::endl;
	client.setClientFd(client_fd);
	client._response._clientFd = client_fd;
	client._response._request = client._request;
	int flag;
	if (client._request.getMethod() == "POST")
	{
		std::cout << "POST request received" << std::endl;
	}
	else if (client._request.getMethod() == "GET")
	{
		std::cout << "GET response received" << std::endl;
		client._response.handleGetRequest(&flag);
		if (flag == 1)
			return 1;
		if (flag == 0)
			return 0;
		if (flag == 2)
		{
			client._response._responseBuffer.clear();
			client._response._header_falg = false;
			client._response._fileOffset = 0;
			client._response.file.close();
			client._response._isopen = false;
			std::cout << "File transfer complete" << std::endl;
			return 1;
		}
	}
	else if (client._request.getMethod() == "DELETE")
	{
		// client._request.endOfRequest = false;
	}
	return 0;
}

void Server::setConfig(ConfigFile *config)
{
	_config = config;
}