
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
	client.setClientFd(client_fd);
	if (!client.is_Header_Complete())
	{
		if (client.read_data())
			return -1;
		if (!client.is_Header_Complete())
		{
			if (client.parse_Header_Request() == false)
			{
				std::cerr << "Error parsing request =====" << std::endl;	
				return 2;
			}

		}
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
	client.setClientFd(client_fd);
	client._response._clientFd = client_fd;
	// client._response._request = client._request;
	int flag;
	if (client._request.getMethod() == "POST")
	{
		std::cout << "POST request received" << std::endl;
	}
	else if (client._request.getMethod() == "GET" && client._request.getStatusCode() < 400)
	{
		// std::cout << "GET response received" << std::endl;
		client._response.handleGetResponse(&flag, client._request);
	 	if (client._request.getStatusCode() >= 400)
		{
			client._keepAlive = client._response._keepAlive;
			client._response.generate_error_response(client._request.getStatusCode(), client_fd);
			return 1;
		}
		if (flag == 1)
			return 1;
		if (flag == 0)
			return 0;
		if (flag == 2)
		{
			std::cout << "File sent successfully flag = 2" << std::endl;
			client._response._responseBuffer.clear();
			client._response._header_falg = false;
			client._response._fileOffset = 0;
			client._response.file.close();
			client._response._isopen = false;
			// client._response.reset();
			client._keepAlive = client._response._keepAlive;
			return 1;
		}
	}
	else if (client._request.getMethod() == "DELETE" && client._request.getStatusCode() < 400)
	{
		// client._request.endOfRequest = false;
	}
	if (client._request.getStatusCode() >= 400)
	{
		client._keepAlive = client._response._keepAlive;
		client._response.generate_error_response(client._request.getStatusCode(), client_fd);
		return 1;
	}
	return 0;
}

void Server::setConfig(ConfigFile *config)
{
	_config = config;
}