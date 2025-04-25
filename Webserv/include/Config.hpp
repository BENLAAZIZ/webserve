/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aben-cha <aben-cha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 23:45:31 by aben-cha          #+#    #+#             */
/*   Updated: 2025/04/14 22:47:24 by aben-cha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "web.h"


class ConfigParser;
class Server;
class Client;

class ConfigFile {
	public:
		ConfigFile();
		~ConfigFile();
	
		void addServer(int port, Server_holder& serv_hldr);
		void run();

	public:
		std::vector<struct pollfd> poll_fds;
		std::vector<Server*> servers;
		std::map<int, int> client_server_map;  // Maps client fd -> server index to track which server a client is connected to
		std::map<int, Client> _clients;

		ConfigParser parser;

		void handleEvents();
		void cleanupDisconnectedClient(int client_fd);
		void set_client_map(int client_fd, Client &client);
};

#endif