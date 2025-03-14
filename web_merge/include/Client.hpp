/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 14:44:11 by hben-laz          #+#    #+#             */
/*   Updated: 2025/03/13 03:30:12 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <map>
#include <vector>
#include <netinet/in.h>
// #include "ServerConfig.hpp"
#include "Config.hpp"
#include "Request.hpp"
// #include "Response.hpp"

class Client {
	public:
		// Response _response;
		Request _request;
		int _socket;
		struct sockaddr_in _address;
		int _clientFd;
		std::string _requestBuffer;
		std::string _responseBuffer;
		bool request_Header_Complete;
		bool _responseSent;
		bool _keepAlive;
		
		// Helper methods for handling different HTTP methods
		void handleGetRequest();
		// void handlePostRequest();
		void handleDeleteRequest();
		
		// Helper for determining content type
		std::string getExtension(const std::string& path);

		// set client_fd
		void setClientFd(int client_fd);

		// get client_fd
		int getClientFd() const;

	public:
		Client();
		Client(int socket, struct sockaddr_in address);
		~Client();
		Client(const Client& other);
		Client& operator=(const Client& other);

		// Check if the request is complete
		bool is_Header_Complete();
		
		// Parse HTTP request
		bool parse_Header_Request(std::string& line_buf);
		
		// Generate HTTP response
		void generateResponse_GET_DELETE();
		
		// Send HTTP response
		bool sendResponse(int client_fd);
		
		// Send error response
		void sendErrorResponse(int statusCode, const std::string& statusMessage);
		
		// Check if response is completely sent
		bool isDoneWithResponse() const;
		
		// Check if connection should be kept alive
		bool keepAlive() const;
		
		// Reset client for new request
		void reset();
};
#endif // CLIENT_HPP