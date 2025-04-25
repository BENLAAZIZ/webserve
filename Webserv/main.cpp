

#include "include/web.h"

// int main() {
// 	try {
// 		// Instead of calling flush() every time, you can disable buffering globally :
// 		std::cout.setf(std::ios::unitbuf); // Disables buffering completely
// 		// Use flush(); when redirecting output to files to prevent missing data! 🚀
// 		// //std::cout.flush();  // ✅ Ensures all data is written immediately

// 		std::vector<int> ports;
// 		ports.push_back(8080);
// 		ports.push_back(5050);
// 		ports.push_back(8082);

// 		ConfigFile configFile;

// 		for (size_t i = 0; i < ports.size(); ++i) {
// 			configFile.addServer(ports[i]);
// 		}
// 		configFile.run();
		
// 	} catch (const std::exception& e) {
// 		std::cerr << "Error: " << e.what() << std::endl;
// 	}
// 	return 0;
// }

// int main(int ac, char **av) {
// 	try {
// 		// Instead of calling flush() every time, you can disable buffering globally :
// 		std::cout.setf(std::ios::unitbuf); // Disables buffering completely
// 		// Use flush(); when redirecting output to files to prevent missing data! 🚀
// 		// //std::cout.flush();  // ✅ Ensures all data is written immediately

	
// 		ConfigFile configFile;

// 		configFile.parser.process_config(ac, av);
// 		// configFile.parser.prints_serves_configuration();
// 		// configFile.parser.get_servers();

// 		for (size_t i = 0; i < configFile.parser.servers.size(); ++i) {
// 			for (size_t j = 0; j < configFile.parser.servers[i].port.size(); ++j) {
// 				// std::cout << "Server " << i << " Port: " << configFile.parser.servers[i].port[j] << std::endl;
// 				configFile.addServer(configFile.parser.servers[i].port[j], configFile.parser.servers[i]);
// 			}
// 		}
// 		configFile.run();

// 	} catch (const std::exception& e) {
// 		std::cerr << "Error: " << e.what() << std::endl;
// 	}
// 	return 0;
// }

bool	is_server(ConfigFile & config, int port, std::string host, int &index_server)
{
	for (size_t i = 0; i < config.servers.size(); i++)
	{
		if (host == config.servers[i]->serv_hldr.host && (port == config.servers[i]->getPort()))
			return index_server = i ,true;
	}
	return false;
}

int main(int ac, char **av) {
	try {
		// Instead of calling flush() every time, you can disable buffering globally :
		std::cout.setf(std::ios::unitbuf); // Disables buffering completely
		// Use flush(); when redirecting output to files to prevent missing data! 🚀
		// //std::cout.flush();  // ✅ Ensures all data is written immediately

	
		ConfigFile configFile;

		configFile.parser.process_config(ac, av);
		// configFile.parser.prints_serves_configuration();
		// configFile.parser.get_servers();

		int nbr_of_server = -1;
		for (size_t i = 0; i < configFile.parser.servers.size(); ++i) {
			for (size_t j = 0; j < configFile.parser.servers[i].port.size(); ++j) {
				int index_server = -1;
				if (!is_server(configFile, configFile.parser.servers[i].port[j], configFile.parser.servers[i].host, index_server))
				{
					configFile.addServer(configFile.parser.servers[i].port[j], configFile.parser.servers[i]);
					nbr_of_server++;
					configFile.servers[nbr_of_server]->servers_holder.push_back(configFile.parser.servers[i]);;
					configFile.servers[nbr_of_server]->default_serv_hldr = configFile.parser.servers[i];
				}
 				if (index_server != -1)
				{
					if (configFile.parser.servers[i].is_default == true)
					{
						configFile.servers[index_server]->default_serv_hldr = configFile.parser.servers[i];
						configFile.servers[index_server]->serv_hldr = configFile.parser.servers[i];
					}
					configFile.servers[index_server]->servers_holder.push_back(configFile.parser.servers[i]);
				}
			}
		}
		// std::cout << "++++++++++++++++++++++++++++++++++" << std::endl;
		// for (size_t i = 0; i < configFile.servers.size(); i++){
		// 	std::cout << "server_port : " << configFile.servers[i]->getServerFd() << std::endl;
		// 	std::cout << "default_server : " << configFile.servers[i]->default_serv_hldr.server_name << std::endl << std::endl;

		// 	for (size_t j = 0; j < configFile.servers[i]->servers_holder.size(); j++)
		// 	{
		// 		std::cout << "server_name : " << configFile.servers[i]->servers_holder[j].server_name << std::endl;
		// 	}
		// 	std::cout << "++++++++++++++++++++++++++++++++++" << std::endl;
		// } 
		
		configFile.run();

	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}