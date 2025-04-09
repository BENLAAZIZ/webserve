/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 23:45:31 by hben-laz          #+#    #+#             */
/*   Updated: 2025/04/09 00:10:44 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "web.h"


class Server;
class Client;

class ConfigFile {
	public:
		ConfigFile();
		~ConfigFile();
	
		void addServer(int port);
		void run();

	public:
		std::vector<struct pollfd> poll_fds;
		std::vector<Server*> servers;
		std::map<int, int> client_server_map;  // Maps client fd -> server index to track which server a client is connected to
		std::map<int, Client> _clients;

		void handleEvents();
		void cleanupDisconnectedClient(int client_fd);
		void set_client_map(int client_fd, Client &client);
};

#endif