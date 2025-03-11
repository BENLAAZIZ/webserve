/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:05:47 by aben-cha          #+#    #+#             */
/*   Updated: 2025/03/11 03:54:26 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include "Request.hpp"
#include "Client.hpp"

class Client;

#define BUFFER_SIZE 1000
#define MAX_CLIENTS 1000
// MAX_REQUEST_SIZE
#define MAX_REQUEST_SIZE 1000000

class Server {
    public:
        Server(int port);
        ~Server();
        int getServerFd() const { return server_fd; }
        int getPort() const { return port; }
        int acceptNewConnection();
        // int handleClientData(int client_fd);
        int handleClientData(int client_fd, Client &client);
        void setNonBlocking(int fd);
        
    private:
        int server_fd;
        int port;
};

#endif