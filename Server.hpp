/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:05:47 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/21 15:57:36 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "HTTPRequest.hpp"


#define PORT 8080
#define BUFFER_SIZE 30000
#define MAX_CLIENTS 1000

class Server {
    private:
        int server_fd;
        std::vector<struct pollfd> pollfds;
        void setNonBlocking(int fd);
        std::map<int, HTTPRequest> requests;
        std::map<int, std::string> clientBuffers; // Store accumulated request data for each client

        
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


//=============================================================================================
// i am working in websever project at 1337 school . my part now is the handler parse request that coming from client  especially for methods: (GET , POST and DELETE).
// this request line can come of parts According to size of buffer #define BUFFER_SIZE 10
// example request :
// GET /html HTTP/1.1\r\nHost: localhost:8080\r\nTransfer-Encoding: chunked\r\nContent-Length: 39\r\nContent-Type: multipart/form-data;\r\n\r\n
// we know that the header terminate with 2 CRLF "\r\n\r\n"

// for the parsing :

// parse the firs line GET /html HTTP/1.1\r\n intel CRLF "\r\n"