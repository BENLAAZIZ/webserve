/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 23:45:15 by aben-cha          #+#    #+#             */
/*   Updated: 2025/03/12 03:53:27 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "../include/Config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <poll.h>
#include <unistd.h>

ConfigFile::ConfigFile() {
}

ConfigFile::~ConfigFile() {
	// Clean up all servers
	for (size_t i = 0; i < servers.size(); ++i) {
		delete servers[i];
	}
	
	// Close any remaining client connections
	for (std::map<int, int>::iterator it = client_server_map.begin(); it != client_server_map.end(); ++it) {
		close(it->first);
	}
}

void ConfigFile::addServer(int port) {
	Server* server = new Server(port);
	// ========================
	server->setConfig(this);

	// ========================
	servers.push_back(server);
	
	struct pollfd pfd;
	pfd.fd = server->getServerFd();
	pfd.events = POLLIN;
	poll_fds.push_back(pfd);
}

void ConfigFile::cleanupDisconnectedClient(int client_fd) {
	close(client_fd);
	client_server_map.erase(client_fd);
	
	// Remove from poll_fds
	for (size_t j = 0; j < poll_fds.size(); ++j) {
		if (poll_fds[j].fd == client_fd) {
			poll_fds.erase(poll_fds.begin() + j);
			break;
		}
	}
}

void ConfigFile::handleEvents() {
	int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
	if (poll_count < 0) {
		std::cerr << "Poll failed: " << strerror(errno) << std::endl;
		return;
	}
	
	std::vector<struct pollfd> current_fds = poll_fds;
	
	for (size_t i = 0; i < current_fds.size(); ++i) {
		if (!(current_fds[i].revents & POLLIN)) {
			continue;
		}
		
		int current_fd = current_fds[i].fd;
		bool is_server = false;
		int server_index = -1;
		
		// Check if this is a server socket
		for (size_t j = 0; j < servers.size(); ++j) {
			if (current_fd == servers[j]->getServerFd()) {
				is_server = true;
				server_index = j;
				break;
			}
		}

		if (is_server) { // Handle new connection on this server
			int client_fd = servers[server_index]->acceptNewConnection();
			if (client_fd >= 0) {
				struct pollfd pfd;
				pfd.fd = client_fd;
				pfd.events = POLLIN;
				poll_fds.push_back(pfd);
				
				client_server_map[client_fd] = server_index; // Map client fd to server index
				// ========================
				Client client;
				client.setClientFd(client_fd);
				set_client_map(client_fd, client);

				
				// servers[server_index]->_config.poll_fds = poll_fds;

				// ========================
			}
		} else { // This is a client, handle the data
			if (client_server_map.find(current_fd) != client_server_map.end()) {
				int owner_server = client_server_map[current_fd];
				// std::cout << "== owner_server = " << owner_server << std::endl;
				if (current_fds[i].revents & POLLIN) {// test
					int res = servers[owner_server]->handleClientData(current_fd, _clients[current_fd]);
					
					bool client_disconnected = false;
					
					if (res < 0)
						client_disconnected = true;
					else if (res == 1) {
					// Response is ready to be sent, change to POLLOUT
						// ========================

							for (size_t j = 0; j < poll_fds.size(); j++) {
								if (poll_fds[j].fd == current_fd) {
									poll_fds[j].events = POLLOUT;
									break;
							}
							current_fds = poll_fds;
							std::cout << "====== fd ====== " << poll_fds[j].fd << std::endl;
							std::cout << "======poll_fds[owner_server] ====== " << poll_fds[owner_server].fd << std::endl;
							std::cout << " j = " << j << std::endl;
							std::cout << " i = " << i << std::endl;
							std::cout << " owner_server = " << owner_server << std::endl;

}
						// ========================
					}
					
					if (client_disconnected)
					{
						std::cerr << "Client disconnected" << std::endl;
						cleanupDisconnectedClient(current_fd);
					}
				}// fin test
				// Handle client data - writing response
				if (current_fds[i].revents & POLLOUT) {
					std::cout << "POLLOUT" << std::endl;
					if (client_server_map.find(current_fd) != client_server_map.end()) {
						Client& client = _clients[current_fd];
						
						// Send response
						if (!client.sendResponse()) {
							// Failed to send or completed sending
							if (client.isDoneWithResponse()) {
								// If keep-alive is not set, close the connection
								if (!client.keepAlive()) {
									// servers[owner_server]->removeClient(current_fd);
								} else {
									// Reset client for next request
									client.reset();
									// Update to only listen for reads again
									poll_fds[i].events = POLLIN;
								}
							}
						}
					}
				}
			} else {
				std::cerr << "Error: Client FD " << current_fd << " not associated with any server" << std::endl;
				cleanupDisconnectedClient(current_fd);
			}
		}
	}
}

void ConfigFile::run() {
	while (true) {
		handleEvents();
	}
}



void ConfigFile::set_client_map(int client_fd, Client &client)
{
	_clients[client_fd] = client;
}