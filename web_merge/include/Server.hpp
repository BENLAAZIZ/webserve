/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:05:47 by aben-cha          #+#    #+#             */
/*   Updated: 2025/03/14 04:14:57 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include "web.h"

class Client;
class ConfigFile;

#define BUFFER_SIZE 10
#define MAX_CLIENTS 1000
#define MAX_REQUEST_SIZE 1000000

class Server {
    public:
        ConfigFile *_config; 
        Server(int port);
        ~Server();
        int getServerFd() const { return server_fd; }
        int getPort() const { return port; }
        int acceptNewConnection();
        int handleClientData(int client_fd, Client &client);
        void setNonBlocking(int fd);
        void setConfig(ConfigFile* config); // Add setter method

    private:
        int server_fd;
        int port;
};

#endif