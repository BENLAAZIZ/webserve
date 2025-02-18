/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:05:47 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/18 18:00:26 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "HTTPRequest.hpp"


#define PORT 8080
#define BUFFER_SIZE 10
#define MAX_CLIENTS 1000

class Server {
    private:
        int server_fd;
        std::vector<struct pollfd> pollfds;
        void setNonBlocking(int fd);
        // std::vector<HTTPRequest> requests;
        std::map<int, HTTPRequest> requests;
        std::map<int, std::string> clientBuffers; // Store accumulated request data for each client
        bool flag_end_of_headers;
    public:
        // Server(const Server& copy);
        // Server& operator=(const Server& copy);
        ~Server();
        Server(int port);
        void handleNewConnection();
        void handleClientData(std::size_t index);
        void run();
        void handleRequest(int client_fd, HTTPRequest &request);

        // std::string toUpperCase(const std::string& str);
};

#endif