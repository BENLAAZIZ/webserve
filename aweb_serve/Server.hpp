/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:05:47 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/25 17:51:58 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include "HTTPRequest.hpp"


#define PORT 8080
#define BUFFER_SIZE 1000
#define MAX_CLIENTS 1000

	class Server {
	private:
		int server_fd;
		std::vector<struct pollfd> poll_fds;
		void setNonBlocking(int fd);
		HTTPRequest request;
		std::string buffer_data;
		std::map<int, HTTPRequest> requests;

		
	public:
		// Server(const Server& copy);
		// Server& operator=(const Server& copy);
		~Server();
		Server(int port);
		void handleNewConnection();
		void handleClientData(std::size_t index);
		void run();
		// void handleRequest(int client_fd, HTTPRequest &request);
		void handleGET(int client_fd, std::string path);
};

#endif
