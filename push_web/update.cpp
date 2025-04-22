void Response::generate_default_error_response(int statusCode) {
	std::ostringstream body;
	std::string statusMessage = get_status_missage(statusCode);
	// Basic HTML content
	// body << "<!DOCTYPE html>\n"
	// 	 << "<html lang=\"en\">\n"
	// 	 << "<head><meta charset=\"UTF-8\"><title>Error "
	// 	 << statusCode << "</title></head>\n"
	// 	 << "<body style=\"font-family:sans-serif;text-align:center;margin-top:50px;\">\n"
	// 	 << "<h1 style=\"color:red\">" << statusCode << " - " << statusMessage << "</h1>\n"
	// 	 << "<p>Sorry, something went wrong.</p>\n"
	// 	 << "<hr><p>WebServ</p>\n"
	// 	 << "</body></html>";
		body << "<!DOCTYPE html>\n"
	     << "<html lang=\"en\">\n"
	     << "<head>\n"
	     << "  <meta charset=\"UTF-8\">\n"
	     << "  <title>Error " << statusCode << "</title>\n"
	     << "  <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n"
	     << "</head>\n"
	     << "<body class=\"bg-dark\">\n"
	     << "  <div class=\"container text-center text-light py-5\">\n"
	     << "    <div class=\"row justify-content-center\">\n"
	     << "      <div class=\"col-lg-6\">\n"
	     << "        <h1 class=\"display-1 fw-bold text-danger\">" << statusCode << "</h1>\n"
	     << "        <h2 class=\"mb-3 fw-bold\">" << statusCode << " - " << statusMessage << "</h2>\n"
	     << "        <p class=\"lead\">Sorry, something went wrong with your request. The server couldn't process it or the resource may not exist.</p>\n"
	     << "        <hr class=\"my-4 border-light\">\n"
	     << "        <p class=\"text-secondary\">WebServ</p>\n"
	     << "      </div>\n"
	     << "    </div>\n"
	     << "  </div>\n"
	     << "</body>\n"
	     << "</html>";

	std::string responseBody = body.str();
	// Construct full HTTP response headers
	std::ostringstream response;
	response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << responseBody.size() << "\r\n";
	if (_keepAlive) {
		response << "Connection: keep-alive\r\n";
	} else {
		response << "Connection: close\r\n";
	}
	response << "\r\n";
	// Append body
	response << responseBody;
	// Store full response
	_responseBuffer = response.str();
}



==========================================

void Response::generate_error_response(int statusCode,  int client_fd, Server_holder& serv_hldr, const std::string& my_root) {
	std::map<int, std::string>::iterator it = serv_hldr.error_pages.find(statusCode);
	if (it != serv_hldr.error_pages.end()) {
		std::string error_page_path = it->second;
		std::cout << "Error page path: " << error_page_path << std::endl;
		// fullPath = "/Users/hben-laz/Desktop/push_web/docs" + error_page_path;
		fullPath = my_root + "/docs" + error_page_path;
		there_is_error_file(fullPath, statusCode);
	} else {
		std::cout << "No custom error page found for status code: " << statusCode << std::endl;
		generate_default_error_response(statusCode);
	}
	_keepAlive = false;  // Don't keep alive on errors
	send_Error_Response(client_fd);
	_responseBuffer.clear();
}




=============================================

int check_white_space(const std::string& str, int *white_space)
{
	int space = 0;
	for (size_t i = 0; str[i]; ++i)
	{
		if ((str[i] >= 9 && str[i] <= 13) || str[i] == 32) 
		{
			if (str[i] == 32)
				space++;
			else
				(*white_space)++;
		}
	}
	return space;
}

bool Request::parseFirstLine(const std::string& line)
{
	int white_space = 0;
	if (check_white_space(line, &white_space) == 2)
	{
		if (white_space != 0)
			return (set_status_code(400), false);
	}
	else
		return (set_status_code(400), false);
	std::istringstream iss(line);
	std::string method, path, version;
	if (!(iss >> method >> path >> version))
		return (set_status_code(400), false);

	if (method != "GET" && method != "POST" && method != "DELETE")
		return (set_status_code(405), false);
	if (version != "HTTP/1.1")
		return (set_status_code(505), false);
	if (checkPath(path))
	{
		setMethod(method);
		setPath(path);
		setVersion(version);
	}
	else
		return false;
	return true;
}





=============================================================
bool Client::handleDeleteResponse(Server_holder &serv_hldr)
{
	std::string targetPath = _request.getpath();

	std::cout << "targetPath: " << targetPath << std::endl;
	// pause();
	// STEP 1: Check if it's a directory
	// struct stat st;
	// if (stat(targetPath.c_str(), &st) != 0) {
	// 	_request.set_status_code(404);
	// 	return true;
	// }
	// if (S_ISDIR(st.st_mode)) {
	// 	_request.set_status_code(403);  // Forbidden: trying to delete a directory
	// 	return true;
	// }

	// add   
	// Check is access to file is allowed
	if (access(targetPath.c_str(), W_OK) != 0) {
		std::cout << "-------- 4  0 3 Forbidden: --------" << std::endl;
		_request.set_status_code(403);  // Forbidden: no write access
		return true;
	}

	// STEP 2: Attempt to delete file
	if (remove(targetPath.c_str()) != 0) {
		_request.set_status_code(500);  // Internal Server Error
		return true;
	}

	// STEP 3: Set redirection to success page
	_request.setPath("/delete/suc.html");

	// Validate that the success page exists
	if (resolve_request_path(serv_hldr) >= 400 || _request.getStatusCode() >= 400) {
		_request.set_status_code(500);
		return true;
	}

	// STEP 4: Success
	_request.set_status_code(204);  // No Content (for DELETE)
	return false; // everything is okay
}


===================================================================================


void Response::handleGetResponse(int *flag, Request &request, int flag_delete) {

	ssize_t bytesSent;
    *flag = 0;

    std::string path = request.getpath();

    std::cout << "==   path  = : " << path << std::endl;
	if (flag_p == 0)
	{
		type_of_path(path);
		flag_p = 1;
	}
    // Check if file exists
	if (request.isCGI && flag_delete == 0)
	{
		std::cout << "CGI" << std::endl;
		Cgi cgi_script(request);
		std::string	output;
		cgi_script.execute_cgi(output);

		request.setContentLength(output.length());
		request.setContent_type("text/html");
		if (send_header_response(CHUNK_SIZE, path, request, 1) == 1)
		{
			std::cout << "Error sending header response" << std::endl;
			*flag = 1;
			return ;
		}
		// check return of send
		bytesSent = send(_clientFd, output.c_str(), output.length(), 0);
		if (bytesSent < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				std::cout << "Socket not ready yet, try again later." << std::endl;
				*flag = 0;  // retry later
				return;
			}
			std::cerr << "Error sending response: " << strerror(errno) << std::endl;
			*flag = 1;  // fatal error
			return;
		}
	}
    if (this->is_file) 
	{
        if (_header_falg == false) {
			if (open_file(flag, path, &request.code) == 1)
			{
				std::cout << "Error opening file ----------------------: " << path << std::endl;
				std::cout << "flag = " << *flag << std::endl;
			    return ;
			}
			if (send_header_response(CHUNK_SIZE, path, request, 0) == 1)
			{
				std::cout << "Error sending header response" << std::endl;
				*flag = 1;
				return ;
			}
        }
        if (_isopen) {
            file.seekg(_fileOffset, std::ios::beg);
            char buffer[CHUNK_SIZE];
            file.read(buffer, CHUNK_SIZE);
            int bytes_read = file.gcount();
            if (bytes_read > 0)
				*flag = send_file_response(buffer, bytes_read);
			else 
			{
				std::cerr << "Error reading file: " << strerror(errno) << std::endl;
				reset();
				*flag = 1;
				return ;
            }
        }
    } 
	else if (this->is_dir) 
	{
		std::cout << "-------- autoindex: --------" << std::endl;
		std::string html = generateAutoIndex(request.getpath(), request.get_fake_path()); // real path, URI path
		request.setContentLength(html.length());
		request.setContent_type("text/html");
        if (send_header_response(CHUNK_SIZE, path, request, 1) == 1)
		{
			std::cout << "Error sending header response" << std::endl;
			*flag = 1;
			return ;
		}
		bytesSent = send(_clientFd, html.c_str(), html.length(), 0);
		if (bytesSent < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				std::cout << "Socket not ready yet, try again later" << std::endl;
				*flag = 0;
				return; // Socket not ready yet
			}
			std::cerr << "Error sending response: " << strerror(errno) << std::endl;
			*flag = 1;
			return;
		}
		reset();
        request.set_status_code(200);
        *flag = 2;
        return ;
    } 
	else 
	{
		std::cout << "== File not found -->   fullPath : " << path << std::endl;
        request.set_status_code(404);
        *flag = 1;
        return ;
    }
    return ;
}


=============================================================================	



ssize_t Response::send_header_response(size_t CHUNK_SIZE, std::string path, Request &request, int flag_autoindex) 
{
	std::string content_type;
	_isopen = true;
	_fileOffset = 0; 
	bool partial = false;
	size_t start = 0;
	size_t end = 0;
	size_t content_length;
	size_t file_size = 0;
    
	if (flag_autoindex  == 0)
	{
		content_type = get_MimeType(path);
		file.seekg(0, std::ios::end);
		file_size = file.tellg();
		file.seekg(0, std::ios::beg);  // Reset to beginning
		end = file_size - 1;
		std::cout << "conte : " << request.getContentLength() << std::endl;
		// Parse Range Header (example: Range: bytes=500-1000)
		if (request.hasHeader("Range")) {
			std::string range = request.getHeader("Range"); // ex: "bytes=500-1000"
			size_t pos = range.find("bytes=");
			if (pos != std::string::npos) {
				range = range.substr(pos + 6); // skip "bytes="
				size_t dash = range.find("-");
				if (dash != std::string::npos) {
					std::string startStr = range.substr(0, dash);
					std::string endStr = range.substr(dash + 1);
					start = std::stoul(startStr);
					if (!endStr.empty())
						end = std::stoul(endStr);
					if (end >= file_size)
						end = file_size - 1;
					if (start < file_size && start <= end)
						partial = true;
				}
			}
		}
		content_length = end - start + 1;
		_fileOffset = start;
		file.seekg(_fileOffset, std::ios::beg);
	}
	else
	{
		content_length = request.getContentLength();
		content_type = request.getContent_type();
	}
	if (content_length > CHUNK_SIZE)
		_keepAlive = true;
	std::cout << "CHUNK_SIZE : " << CHUNK_SIZE << std::endl;
	// Generate headers
	std::ostringstream headers;
	if (partial)
		headers << "HTTP/1.1 206 Partial Content\r\n";
	else
	{
		std::cout << "request.getStatusCode() : " << request.getStatusCode() << std::endl;
		headers << "HTTP/1.1 "  << get_status_missage(request.getStatusCode()) << "\r\n";

	}
	// else
	// 	headers << "HTTP/1.1 200 OK\r\n";

	headers << "Content-Type: " << content_type << "\r\n";
	headers << "Content-Length: " << content_length << "\r\n";
	headers << "Accept-Ranges: bytes\r\n";
	if (partial)
		headers << "Content-Range: bytes " << start << "-" << end << "/" << file_size << "\r\n";
	if (_keepAlive)
		headers << "Connection: keep-alive\r\n";
	else
		headers << "Connection: close\r\n";
	headers << "\r\n";

	_header_falg = true;
	_responseBuffer = headers.str();
	// check return of send
	ssize_t bytesSent = send(_clientFd, _responseBuffer.c_str(), _responseBuffer.size(), 0);
	if (bytesSent < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return false; // Socket not ready yet
		std::cerr << "Error sending response: " << strerror(errno) << std::endl;
		return true;
	}
	if (bytesSent == 0)
	{
		std::cerr << "No data sent" << std::endl;
		return true;
	}

	std::cout << "\n*********************** Headers response *********************" <<  std::endl;
	std::cout  << _responseBuffer ;
	std::cout << "*********************** end of header response *********************\n" <<  std::endl;
	_responseBuffer.clear();
	return false;
}