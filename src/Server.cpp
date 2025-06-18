
#include "../include/web.h"

Server::Server(int port, Server_holder& serv_hldr) : serv_hldr(serv_hldr), port(port) {
	struct addrinfo hints, *res;
	int status;

	const char* hostStr = (serv_hldr.host == "localhost") ? "127.0.0.1" : serv_hldr.host.c_str();

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // Fill in IP for us if NULL

	std::ostringstream portStream;
	portStream << port;
	std::string portStr = portStream.str();

	status = getaddrinfo(hostStr, portStr.c_str(), &hints, &res);
	if (status != 0) {
		throw std::runtime_error(std::string("getaddrinfo error: ") + gai_strerror(status));
	}

	server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (server_fd < 0) {
		freeaddrinfo(res);
		throw std::runtime_error("Socket creation failed");
	}

	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		close(server_fd);
		freeaddrinfo(res);
		throw std::runtime_error("setsockopt failed");
	}

	if (bind(server_fd, res->ai_addr, res->ai_addrlen) < 0) {
		close(server_fd);
		freeaddrinfo(res);
		throw std::runtime_error("Bind failed");
	}


	if (listen(server_fd, MAX_CLIENTS) < 0) {
		close(server_fd);
		throw std::runtime_error("Listen failed");
	}
	freeaddrinfo(res); // Done with the addrinfo

	setNonBlocking(server_fd);
	std::cout  << "http://" << hostStr << ":" << port << std::endl;


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
		return -1;
	}

	setNonBlocking(client_fd);
	
	return client_fd;
}

Location* find_location(const std::string& path, const std::vector<Location>& locations) {
    Location* bestMatch = NULL;
    size_t bestLen = 0;

    for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const std::string& locPath = it->path;

        if (path.length() < locPath.length())
            continue;

        if (path.compare(0, locPath.length(), locPath) == 0) {
            if (path.length() > locPath.length() && path[locPath.length()] != '/')
                continue;

            if (locPath.length() > bestLen) {
                bestLen = locPath.length();
                bestMatch = const_cast<Location*>(&(*it));
            }
        }
    }
    return bestMatch;
}

int check_location(Server_holder& serv_hldr, Client& client) {
	std::string path = "/upload";
	if (client._request.isCGI)
        return 0;
	if (path != client._request.getpath()) {
		client._request.set_status_code(404);
		client._request.endOfRequest = true;
		return 1;
	}
	Location* loc = find_location(path, serv_hldr.locations);
	
	if (loc != NULL) {

		if (loc->root.empty() && serv_hldr.root.empty()) {
			client._request.set_status_code(500);
			client._request.endOfRequest = true;
			return 1;
		}
		if (loc->allowed_methods.size() > 0) {
			std::string method = client._request.getMethod();
			if (std::find(loc->allowed_methods.begin(), loc->allowed_methods.end(), method) == loc->allowed_methods.end()) {
				client._request.set_status_code(405);
				client._request.endOfRequest = true;
				return 1;
			}
		}
    }

	return 0;
}

int Server::handleClientData(int client_fd, Client &client) {
	client.setClientFd(client_fd);
	if (!client.is_Header_Complete())
	{
		if (client.read_data())
			return -1;
		if (!client.is_Header_Complete())
			client.parse_Header_Request(serv_hldr);
		if (client._request.endOfRequest)
			return 2;
		if (client.is_Header_Complete() && client._request.getMethod() != "POST") {
			client._request.endOfRequest = true;
			return 2;
		}
		else if (client.is_Header_Complete() && client._request.getMethod() == "POST")
		{
			if (client._request.getStatusCode() >= 400)
                return 2;
			if (check_location(serv_hldr, client) == 1)
				return 2;
			if (client._request.getStatusCode() >= 400)
                client._request.endOfRequest = true;
            else
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
		return 2;
	}
	return 0;
}



int Server::handleResponse(int client_fd, Client &client, int flag_delete) {

	int flag = -1;
		client._response.handleGetResponse(&flag, client._request, flag_delete, serv_hldr);
		if (flag == 1)
		{
			client._keepAlive = client._response._keepAlive;
			client._response.generate_error_response(client._request.getStatusCode(), client_fd, serv_hldr, client._request.my_root);
			return 1;
		}
		else if (flag == 0)
			return 0;
		else if (flag == 2)
		{
			client._response._responseBuffer.clear();
			client._response._header_falg = false;
			client._response._fileOffset = 0;
			client._response.file.close();
			client._response._isopen = false;
			client._keepAlive = client._response._keepAlive;
			return 1;
		}
	return -1;
}

int	is_host_ip(std::string & str)
{
	size_t i = 0;
	size_t l = 0;
	int	point = 0;
	if (str.length() > 15)
		return 0;
	std::string	holder;
	if (str[0] == '.')
		throw std::runtime_error(". at the beging of ip");
	while (str[i]){
		if (isdigit(str[i]))
		{
			l++;
			holder += str[i];
		}
		else if (str[i] == '.')
		{
			point++;
			if (l == 0 || l > 3)
				return 0;
			if (std::atoi(holder.c_str()) > 255)
				return 0;
			holder = "";
			l = 0;
		}
		else
			return 0;
		i++;
	}
	if (point != 3)
		return 0;
	return 1;
}

void	match_server_name(Request & req, std::vector<Server_holder> &servers_holder, Server_holder &serv)
{
	std::string	host = req.getHeader("host");
	size_t pos = host.find(":");
	if (pos != std::string::npos)
		host = host.substr(0, pos);
	if (is_host_ip(host))
		return ;
	for (size_t i = 0; i < servers_holder.size(); ++i)
	{
		if (servers_holder[i].server_name == host)
		{
			serv = servers_holder[i];
		}
	}
}

int Server::sendResponse(int client_fd, Client &client) {
	serv_hldr = default_serv_hldr;
	match_server_name(client._request, servers_holder, serv_hldr);
	client.setClientFd(client_fd);
	client._response._clientFd = client_fd;
	if (client._request.getStatusCode() >= 400)
	{
		client._keepAlive = client._response._keepAlive;
		client._response.generate_error_response(client._request.getStatusCode(), client_fd, serv_hldr, client._request.my_root);
		return 1;
	}
	if (!client.is_resolved())
	{
		if (client.resolve_request_path(serv_hldr) >= 400 || client._request.getStatusCode() >= 400)
		{
			client._keepAlive = client._response._keepAlive;
			client._response.generate_error_response(client._request.getStatusCode(), client_fd, serv_hldr, client._request.my_root);
			return 1;
		}
		if (client._request.getStatusCode() >= 300 && client._request.getStatusCode() < 400)
		{
			client._response._keepAlive = false;
			client._response.generate_redirect_response(client._request.getStatusCode(), client._request.getpath());
			return 1;
		}
		client.set_resolved(true);
	}
	std::string fullPath = client._request.getpath();
	if (client._request.getMethod() == "POST")
	{
		if (client._request.isCGI)
		{
			client._request.setPath(fullPath);
			return (handleResponse(client_fd, client, 0));
		}
		std::string fullPath = client._request.my_root + "/docs/upload/success.html";
		client._request.setPath(fullPath);
		return (handleResponse(client_fd, client, 0));
	}
	if (client._request.getMethod() == "GET" && client._request.getStatusCode() < 400)
	{
		return (handleResponse(client_fd, client, 0));
	}
	else if (client._request.getMethod() == "DELETE")
	{
		if (client.handleDeleteResponse() == 1)
		{
			client._keepAlive = client._response._keepAlive;
			client._response.generate_error_response(client._request.getStatusCode(), client_fd, serv_hldr, client._request.my_root);
			return 1;
		}
		return (handleResponse(client_fd, client, 1));
	}
	if (client._request.getStatusCode() >= 400)
	{
		client._keepAlive = client._response._keepAlive;
		client._response.generate_error_response(client._request.getStatusCode(), client_fd, serv_hldr, client._request.my_root);
		return 1;
	}
	return 0;
}

