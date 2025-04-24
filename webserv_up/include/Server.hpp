/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aben-cha <aben-cha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:05:47 by aben-cha          #+#    #+#             */
/*   Updated: 2025/04/17 18:28:33 by aben-cha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include "web.h"

class Client;
class ConfigFile;

#define BUFFER_SIZE 8000
#define MAX_CLIENTS 1000
#define MAX_REQUEST_SIZE 1000000

class Server {
    public:
        ConfigFile *_config; 
        // Server(int port);
        Server(int port, Server_holder& serv_hldr);
        ~Server();
        int getServerFd() const { return server_fd; }
        int getPort() const { return port; }
        int acceptNewConnection();
        int handleClientData(int client_fd, Client &client);
        void setNonBlocking(int fd);
        void setConfig(ConfigFile* config); // Add setter method
        int sendResponse(int client_fd, Client &client);
        void sendSuccessResponse(int clientSocket);

        int handleResponse(int client_fd, Client &client, int flag_delete);
        
        Server_holder serv_hldr;
        
    private:
        int server_fd;
        int port;
};

#endif