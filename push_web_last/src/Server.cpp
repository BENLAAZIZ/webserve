
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

// Server::Server(int port, Server_holder& serv_hldr) : serv_hldr(serv_hldr), port(port) {
// 		const char* hostStr = serv_hldr.host == "localhost" ? NULL : serv_hldr.host.c_str();

// 		struct addrinfo hints, *res;
// 		memset(&hints, 0, sizeof hints);
// 		hints.ai_family = AF_INET;
// 		hints.ai_socktype = SOCK_STREAM;
// 		hints.ai_flags = AI_PASSIVE;

// 		std::stringstream ss;
// 		ss << port;
// 		std::string portStr = ss.str();

// 		int status = getaddrinfo(hostStr, portStr.c_str(), &hints, &res);
// 		if (status != 0)
// 		throw std::runtime_error("getaddrinfo error");

// 		server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
// 		if (server_fd < 0)
// 		throw std::runtime_error("Socket creation failed");

// 		int yes = 1;
// 		setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

// 		if (bind(server_fd, res->ai_addr, res->ai_addrlen) < 0)
// 		throw std::runtime_error("Bind failed");

// 	if (listen(server_fd, MAX_CLIENTS) < 0) {
// 		close(server_fd);
// 		throw std::runtime_error("Listen failed");
// 	}
// 		freeaddrinfo(res);
// 	setNonBlocking(server_fd);
// 	// freeaddrinfo(res); // Done with the addrinfo
// 	std::cout << "Server fd: " << server_fd << " listening on port " << port << std::endl;
// }

Server::Server(int port, Server_holder& serv_hldr) : serv_hldr(serv_hldr), port(port) {
	struct addrinfo hints, *res;
	int status;

	std::cout << "after host: " << serv_hldr.host << std::endl;

	const char* hostStr = (serv_hldr.host == "localhost") ? "127.0.0.1" : serv_hldr.host.c_str();

	// Prepare hints
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // Fill in IP for us if NULL

	// Convert port to string
	std::ostringstream portStream;
	portStream << port;
	std::string portStr = portStream.str();

	// Get address info
	std::cout << "host: " << hostStr << std::endl;
	status = getaddrinfo(hostStr, portStr.c_str(), &hints, &res);
	if (status != 0) {
		throw std::runtime_error(std::string("getaddrinfo error: ") + gai_strerror(status));
	}

	// Create socket
	server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (server_fd < 0) {
		freeaddrinfo(res);
		throw std::runtime_error("Socket creation failed");
	}

	// Reuse address
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		close(server_fd);
		freeaddrinfo(res);
		throw std::runtime_error("setsockopt failed");
	}

	// Bind
	if (bind(server_fd, res->ai_addr, res->ai_addrlen) < 0) {
		close(server_fd);
		freeaddrinfo(res);
		throw std::runtime_error("Bind failed");
	}


	// Listen
	if (listen(server_fd, MAX_CLIENTS) < 0) {
		close(server_fd);
		throw std::runtime_error("Listen failed");
	}
	freeaddrinfo(res); // Done with the addrinfo

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
			client.parse_Header_Request();
		if (client.is_Header_Complete() && client._request.getMethod() != "POST") {
			client._request.endOfRequest = true;
			return 2;
		}
		else if (client.is_Header_Complete() && client._request.getMethod() == "POST")
		{
			std::cout << "POST request received" << std::endl;
			client.handlePostRequest();
			if (client._request.endOfRequest)
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

	int flag = -1;
	// find location 
	// serv_hldr.locations
	// resolve_request_path(client);


	if (client.resolve_request_path(serv_hldr) >= 400 || client._request.getStatusCode() >= 400)
	{
		std::cout << "---- Error: " << client._request.getStatusCode() << std::endl;
		client._response.generate_error_response(client._request.getStatusCode(), client_fd);
		return 1;
	}
	std::string filePath = client._request.getpath();
	std::cout << ",,,,,,,, file exist: " << filePath << std::endl;

	// pause();
	
	if (client._request.getMethod() == "POST")
	{
		std::cout << "POST request received" << std::endl;
		client.handlePostResponse();
	}
	else if (client._request.getMethod() == "GET")
	{
		std::cout << "GET response received" << std::endl;
		client._response.handleGetResponse(&flag, client._request);
		if (flag == 1)
		{
			std::cout << "+++++++ File sent successfully flag = 1" << std::endl;
			client._response.generate_error_response(client._request.getStatusCode(), client_fd);
			return 1;
		}
		if (flag == 0)
		{
			// std::cout << "File sent not yet successfully flag = 0" << std::endl;
			return 0;
		}
		if (flag == 2)
		{
			std::cout << "File sent successfully flag = 2" << std::endl;
			client._response._responseBuffer.clear();
			client._response._header_falg = false;
			client._response._fileOffset = 0;
			client._response.file.close();
			client._response._isopen = false;
			client._response.reset();
			client._keepAlive = client._response._keepAlive;
			return 1;
		}
	}
	else if (client._request.getMethod() == "DELETE")
	{
		// client._request.endOfRequest = false;
	}
	// if (client._request.getStatusCode() >= 400)
	// {
	// 	client._keepAlive = client._response._keepAlive;
	// 	client._response.generate_error_response(client._request.getStatusCode(), client_fd);
	// 	return 1;
	// }
	return 0;
}

void Server::setConfig(ConfigFile *config)
{
	_config = config;
}


