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