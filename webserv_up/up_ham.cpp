int Server::handleClientData(int client_fd, Client &client) {
	client.setClientFd(client_fd);
	if (!client.is_Header_Complete())
	{
		if (client.read_data())
			return -1;
		if (!client.is_Header_Complete())
		{
			// client.parse_Header_Request();
			client.parse_Header_Request(this->serv_hldr);
		}


===================================================

bool Client::parse_Header_Request(Server_holder& serv_hldr) 
{
    .
    .
    .
    .
    .
    .
    ...
    .
    .
    .
    	if (_request.getContentLength() > serv_hldr.client_max_body_size)
	{
		_request.set_status_code(413);
		this->request_Header_Complete = true;
		return false;
	}
	return true;
=================================================




Location* find_location(const std::string& path, const std::vector<Location>& locations) {
    Location* bestMatch = NULL;
    size_t bestLen = 0;

    for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const std::string& locPath = it->path;

        // Skip if location path is longer than request path
        if (path.length() < locPath.length())
            continue;

        // Check if path starts with location path
        if (path.compare(0, locPath.length(), locPath) == 0) {
            // OPTIONAL: If exact match, OR next char is '/', consider it a valid segment
            if (path.length() > locPath.length() && path[locPath.length()] != '/')
                continue;

            // Prefer the longest matching prefix
            if (locPath.length() > bestLen) {
                bestLen = locPath.length();
                bestMatch = const_cast<Location*>(&(*it));
            }
        }
    }
    return bestMatch;
}

int check_location(Server_holder& serv_hldr, Client& client) {
	std::cout << "================================================ check_location" << std::endl;
	std::string path = "/upload";
	Location* loc = find_location(path, serv_hldr.locations);
	// pause();
	  if (loc == NULL) {
        std::cerr << "Location not found" << std::endl;
        client._request.set_status_code(404);
		client._request.endOfRequest = true;
		return 1;
    }
	if (loc->root.empty() && serv_hldr.root.empty()) {
		std::cerr << "Root not found" << std::endl;
		client._request.set_status_code(500);
		client._request.endOfRequest = true;
		return 1;
	}
    // check if method is allowed exactly POST
    if (loc->allowed_methods.size() > 0) {
        std::string method = client._request.getMethod();
        if (std::find(loc->allowed_methods.begin(), loc->allowed_methods.end(), method) == loc->allowed_methods.end()) {
            std::cerr << "Method not allowed" << std::endl;
            client._request.set_status_code(405);
             client._request.endOfRequest = true;
			 return 1;
        }
    }
    else
    {
        std::cerr << "Method not allowed" << std::endl;
        client._request.set_status_code(405);
        client._request.endOfRequest = true;
		return 1;
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
			client.parse_Header_Request();
		if (client.is_Header_Complete() && client._request.getMethod() != "POST") {
			// std::cout << "================= header request ==================" << std::endl;
			client._request.endOfRequest = true;
			return 2;
		}
		else if (client.is_Header_Complete() && client._request.getMethod() == "POST")
		{
			//=================================
			if (client._request.getStatusCode() >= 400)
                return 2;
			if (check_location(serv_hldr, client) == 1)
				return 2;
			//================================
			std::cout << "-------- POST request received----------" << std::endl;
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
		// //=================================
		// if (client._request.getStatusCode() >= 400)
        //         return 2;
		// if (check_location(serv_hldr, client) == 1)
		// 	return 2;
		// //================================
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