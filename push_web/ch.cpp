	// in Server.cpp

    int Server::sendResponse(int client_fd, Client &client) {
    
    else if (client._request.getMethod() == "DELETE")
	{
		if (handleDeleteResponse(client) == 1)
		{
			client._keepAlive = client._response._keepAlive;
			client._response.generate_error_response(client._request.getStatusCode(), client_fd, serv_hldr);
			return 1;
		}
		return (handleResponse(client_fd, client));
	}


    }


