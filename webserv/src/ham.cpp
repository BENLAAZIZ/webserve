
// ====== location ======

int  Client::resolve_request_path(Server_holder & serv_hldr) {
	Location* loc = find_matching_location(_request, serv_hldr.locations);
	std::string root = loc ? loc->root : serv_hldr.root;
	std::string location_path = loc ? loc->path : "/";
	std::string getpath = _request.getpath();
	_request.set_fake_path(getpath);

	if (loc && loc->root.empty() && serv_hldr.root.empty()) 
		return (_request.set_status_code(500), 500);
	// if not found location
	if (loc == NULL) {
		full_path = join_paths(_request.my_root, join_paths(serv_hldr.root, getpath));
		if (is_directory(full_path))
			full_path = join_paths(full_path, "index.html");
		std::cout << "Location not found - full_path = " << full_path << std::endl;
		if (file_exists(full_path)) 
			return (_request.setPath(full_path), full_path.clear(), _request.set_status_code(200), 200);
		else
			return (_request.set_status_code(404), 404);
	}
	// redirect path
	if (loc && loc->redirect_code != 0) 
	{
		if (loc->redirect_url.empty()) 
			return (_request.set_status_code(400), 400);
		return (_request.setPath(loc->redirect_url), _request.set_status_code(loc->redirect_code), loc->redirect_code);
	}
	// check allowed methods
	if (loc && loc->allowed_methods.size() > 0)
	{
		std::string method = _request.getMethod();
		if (std::find(loc->allowed_methods.begin(), loc->allowed_methods.end(), method) == loc->allowed_methods.end())
			return (_request.set_status_code(405), 405);
	}
	std::string relative_path = getpath;
	if (getpath.find(location_path) == 0)
		relative_path = getpath.substr(location_path.length()); // strip location path
	// Remove leading '/' to avoid double slashes when joining
	if (!relative_path.empty() && relative_path[0] == '/')
		relative_path = relative_path.substr(1);
	// Compose full_path
	full_path = join_paths(_request.my_root, join_paths(root, relative_path));
	if (_request.isCGI)
		return (_request.setPath(full_path), 200);
	// Check if full_path is a directory or a file
	if (is_directory(full_path)) 
	{
		if (_request.getMethod() == "DELETE")
			return (_request.set_status_code(403), 403);
		if (loc && !loc->index.empty()) {
			std::string index_path = join_paths(full_path, loc->index[0]);
			if (file_exists(index_path))
				return (_request.setPath(index_path), full_path.clear(), _request.set_status_code(200), 200);
			else
				return (_request.set_status_code(404), 404);
		}
		else if (loc && loc->autoindex)
			return (_request.set_status_code(200), _request.setPath(full_path), full_path.clear(), 200);
		return (_request.set_status_code(403), 403);
	}
	else if (file_exists(full_path))
		return (_request.setPath(full_path), full_path.clear(), _request.set_status_code(200), 200);
	return (_request.set_status_code(404), 404);
}

/////////////////////////////////
 std::string timeoutResp = oss.str();
        // send(fd, timeoutResp.c_str(), timeoutResp.size(), 0);
        _clients[fd]._request.set_status_code(408);
        std::cout << "timeoutResp: " << timeoutResp << std::endl;
        std::cout << "status code : " << _clients[fd]._request.getStatusCode() << std::endl;
        int owner_server = client_server_map[fd];
        servers[owner_server]->sendResponse(fd, _clients[fd]);
        cleanupDisconnectedClient(fd);