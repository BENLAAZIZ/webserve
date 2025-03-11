
// #ifndef SERVER_HPP
// #define SERVER_HPP


// #include <vector>
// #include <string>
// #include <map>
// #include <poll.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <iostream>
// #include "Config.hpp"
// #include "Client.hpp"
// #define MAX_CLIENTS 10

// class Server {
// 	public:
// 		ServerConfig _config;
// 		int serverSocket;
// 		std::map<int, Client> _clients;
// 		std::vector<pollfd> _fds;
// 		std::vector<int> fds;


// 		int server_fd;
// 		bool throw_error;

		
// 		// Makes a socket non-blocking
// 		bool setNonBlocking(int sock);
		
// 		// Accepts new client connections
// 		void handleNewConnection();
		
// 		// Handles client data
// 		void handleClientData(int clientFd);
		
// 		// Removes disconnected client
// 		void removeClient(int clientFd);

// 	public:
// 		Server(const ServerConfig& config);
// 		Server(int port);
// 		~Server();
		
// 		bool createServer();
// 		void processEvents();
// 		// bool handleClientResponse(int clientFd);
// 		// bool clientReadyToSend(int clientFd);
// 		// int acceptNewConnection();
// 		// bool handleClientData(int clientFd);


// };

// #endif // SERVER_HPP

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aben-cha <aben-cha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:05:47 by aben-cha          #+#    #+#             */
/*   Updated: 2025/03/10 23:57:40 by aben-cha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include "../include/Request.hpp"


#define BUFFER_SIZE 1000
#define MAX_CLIENTS 1000

class Server {
    public:
        Server(int port);
        ~Server();
        int getServerFd() const { return server_fd; }
        int getPort() const { return port; }
        int acceptNewConnection();
        int handleClientData(int client_fd);
        void setNonBlocking(int fd);
        
    private:
        int server_fd;
        int port;
};

#endif