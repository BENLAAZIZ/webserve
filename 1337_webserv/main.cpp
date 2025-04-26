

#include "include/web.h"

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
		std::cout.setf(std::ios::unitbuf); // Disables buffering completely

	
		ConfigFile configFile;

		configFile.parser.process_config(ac, av);

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
		configFile.run();

	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}