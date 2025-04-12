
void Response::handleGetResponse(int *flag, Request &request) {

    *flag = 0;

    std::string path = request.getpath();
	// Check for directory traversal attempts
	if (path.find("..") != std::string::npos) {
		std::cerr << "Directory traversal attempt: " << path << std::endl;
		request.set_status_code(403);
        *flag = 1;
		return ;
	}

	
	if (flag_p == 0)
	{
		fullPath = "/Users/hben-laz/Desktop/webserve/web_merge/www";
	      fullPath = fullPath + path;
		  flag_p = 1;
	}
  
  
    if (this->is_file) {
        // File exists, serve it
        const size_t CHUNK_SIZE = 1024; // Increased chunk size for better performance
        if (_header_falg == false) {
			std::cout << "_header_flag: " << _header_falg << std::endl;
			if (open_file(flag, fullPath, &request.code) == 1)
			    return ;
			send_header_response(CHUNK_SIZE, path);
        }
        if (_isopen) {
            file.seekg(_fileOffset, std::ios::beg);
            char buffer[CHUNK_SIZE];
            file.read(buffer, CHUNK_SIZE);
            int bytes_read = file.gcount();
			// write(1, "88888\n", 8);
            if (bytes_read > 0)
			{
				*flag = send_file_response(buffer, bytes_read);
				bytes_sent += bytes_read;
				if (*flag == 2)
				{
					std::cout << "bytes_sent: " << bytes_sent << std::endl;

				}
				
			}
			else 
			{
				std::cerr << "Error reading file: " << strerror(errno) << std::endl;
				reset();
                *flag = 1;
                return ;
            }
        }
    } else if (this->is_dir) {
        // Directory listing (optional, could redirect to index or show listing)
        std::cout << "Directory listing not implemented" << std::endl;
        request.set_status_code(403);
        *flag = 1;
        return ;
    } else {
		std::cout << "==   fullPath: " << fullPath << std::endl;
        // File not found
        request.set_status_code(404);
        *flag = 1;
        return ;
    }
    return ;
}