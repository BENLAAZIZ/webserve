/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 22:11:36 by hben-laz          #+#    #+#             */
/*   Updated: 2025/02/28 22:59:12 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <string>
#include "Config.hpp"
#include "Client.hpp"

class Server {
private:
    // Configuration
    Config config;
    
    // Socket management
    std::vector<int> listenSockets;
    
    // Active clients
    // std::map<int, Client*> clients; // wait
    
    // Poll array
    struct pollfd* pollFds;
    int pollFdsCount;
    int pollFdsCapacity;
    
    // Helper methods
    bool setupListenSockets();
    int createListenSocket(const std::string& host, int port);
    void acceptNewConnection(int serverSocket);
    void handleClientIO(int clientFd, short revents);
    void removePollFd(int fd);
    void addPollFd(int fd, short events);
    void updatePollFd(int fd, short events);
    const ServerConfig* findMatchingServer(int socket, const std::string& host);
    
public:
    Server();
    ~Server();
    
    bool initialize(const std::string& configPath);
    void run();
    void stop();
    
    // Signal handling
    static void handleSignal(int signal);
    static volatile bool running;
};

#endif