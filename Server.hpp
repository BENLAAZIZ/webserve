/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aben-cha <aben-cha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:05:47 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/13 12:50:18 by aben-cha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "HTTPRequest.hpp"


#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 1000

class Server {
    private:
        int server_fd;
        std::vector<struct pollfd> pollfds;
        void setNonBlocking(int fd);
        std::vector<HTTPRequest> requests;
    public:
        // Server(const Server& copy);
        // Server& operator=(const Server& copy);
        ~Server();
        Server(int port);
        void handleNewConnection();
        void handleClientData(std::size_t index);
        void run();
};

#endif