/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:16:57 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/13 22:01:59 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::~Server() {
    for (std::size_t i = 0; i < pollfds.size(); i++) {
        close(pollfds[i].fd);
    }
}

void Server::setNonBlocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}
        
Server::Server(int port) {
    struct sockaddr_in server_addr;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw std::runtime_error("Socket creation failed");
    
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        throw std::runtime_error("Setsockopt failed");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("Bind failed");

    if (listen(server_fd, MAX_CLIENTS) < 0)
        throw std::runtime_error("Listen Failed");
    
    setNonBlocking(server_fd);

    struct pollfd pfd;
    pfd.fd = server_fd;
    pfd.events = POLLIN;
    pollfds.push_back(pfd);
    
    std::cout << "Server listening on port " << port << std::endl;
}
        
void Server::handleNewConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return ;
    }

    setNonBlocking(client_fd);

    struct pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    pollfds.push_back(pfd);

    std::cout << "New connection fd: " << client_fd <<  " from " 
              << inet_ntoa(client_addr.sin_addr)
              << ":" << ntohs(client_addr.sin_port) << std::endl;
}

void Server::handleClientData(std::size_t index) {
    char buffer[BUFFER_SIZE];
    int client_fd = pollfds[index].fd;
    
    std::size_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            std::cout << "client disconnected fd: " << client_fd << std::endl;
            if (requests.size() > 0)// add this condition to avoid segfault
                requests.erase(requests.begin() + index - 1);
            std::cout << "nbr of clients after: " << requests.size()<<  std::endl;
        }
        else if (bytes_read < 0)
            std::cerr << "Receive a message from a socket failed: " << strerror(errno) << std::endl;
        close(client_fd);
        pollfds.erase(pollfds.begin() + index);
        return ;
    }
    buffer[bytes_read] = '\0';
  
    // std::cout << "nbr of clients: " << requests.size()  + 1<<  std::endl;

    //parse request
    HTTPRequest request;
    // request.parseRequest(buffer);
	// std::cout << "---------extension-----------: "  << std::endl;
    //client disconnected fd:

    if (!request.parseRequest(buffer)) {
        request.sendErrorResponse(request.getStatusCode());

        std::cout << "code: " << request.getStatusCode() << std::endl;
        std::cout << "message: " << request.getStatusCodeMessage() << std::endl;
        return;
    }
    std::cout << buffer<<  std::endl;

    if (request.getMethod() == "POST") {
        // std::find("\r\n\r\n", buffer);
    }

//     else 
//         requests.push_back(request);
//   std::cout << "----------------------------------------\n"; 
//     for (std::size_t i = 0; i < requests.size(); i++) {

//         std::cout << "request nbr : " << i << std::endl;
//         std::cout << "Method: " << requests[i].getMethod() << std::endl;
// 		std::cout << "Path: " << requests[i].getpath() << std::endl;
// 		std::cout << "Version: " << requests[i].getVersion() << std::endl;
// 		std::cout << "extension: " << requests[i].getExtension() << std::endl << "headers: ";
//         for (std::map<std::string, std::string>::const_iterator it = requests[i].getHeaders().begin(); it != requests[i].getHeaders().end(); ++it)
//             std::cout << it->first << ": "<< it->second << std::endl;
//         std::cout << "==================================================\n";    
//     }
}

void Server::run() {
    while (1) {
        int poll_count = poll(&pollfds[0], pollfds.size(), -1);
        if (poll_count < 0) {
            std::cerr << "Poll failed: " << strerror(errno) << std::endl;
            continue ;
        }
        for (std::size_t i = 0; i < pollfds.size(); ++i) {
            if (!(pollfds[i].revents & POLLIN))
                continue ;
                    
            if (pollfds[i].fd == server_fd)
                handleNewConnection();
            else
                handleClientData(i);
        }
    }
}


