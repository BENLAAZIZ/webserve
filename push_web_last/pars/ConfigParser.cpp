#include "../include/web.h"

void	ConfigParser::remove_comments()
{
	std::string	result = config_content.substr(0, config_content.find("#", 0));
	// size_t	start = 0;
	size_t	pos = 0;
	size_t	pos1 = config_content.find("\n", pos);

	while (pos < config_content.length())
	{
		if (pos1 == std::string::npos)
		{
			result += config_content.substr(pos1, pos - pos1);
			break;
		}
		pos = config_content.find("#", pos1);
		result += config_content.substr(pos1, pos - pos1);
		pos1 = config_content.find("\n", pos);
	}
	config_content = result;
}

void	ConfigParser::handel_multiple_derictive_in_one_line()
{
	size_t	pos = 0;
	size_t	start = 0;
	size_t	pos1 = 0;
	std::string	reslt;
	while (pos != std::string::npos)
	{
		pos = config_content.find(";", start);
		pos1 = config_content.find_last_not_of(" \t\n", pos - 1);
		reslt += config_content.substr(start, pos1 - start + 1);
		if(pos == std::string::npos)
			break;
		reslt += " ;\n";
		start = pos + 1;
	}
	config_content = reslt;
}

void	ConfigParser::remove_newlines()
{
	for (size_t i = 0; i < config_content.length(); i++)
	{
		if (config_content[i] == '\n')
			config_content[i] = ' ';
	}
}

void	ConfigParser::puting_acolads_in_line()
{
	size_t i = 0;
	std::string reslt;
	while (config_content[i])
	{
		if (config_content[i] == '{')
			reslt += "\n{\n";
		else if (config_content[i] == '}')
			reslt += "\n}\n";
		else
			reslt += config_content[i];
		i++;
	}
	config_content = reslt;
}

void	ConfigParser::checke_syntax(std::vector<std::string>& line)
{
	int	open_acollad = 0;
	int	close_acollad = 0;
	for (size_t i = 0; i < line.size(); i++)
	{
		if (line[i] == "{")
			open_acollad++;
		if (line[i] == "}")
			close_acollad++;
		if(line[i] == "server" && i + 1 < line.size() &&  line[i + 1] != "{")
			throw std::runtime_error("server must flewed by {");
	}
	if (open_acollad < close_acollad)
		throw std::runtime_error("missing {");
	else if (open_acollad > close_acollad)
		throw std::runtime_error("missing }");
}

void	ConfigParser::get_servers()
{
	std::string reslt;
	lines = split(config_content,'\n');
	checke_syntax(lines);
	std::vector<std::string> group;

	group.push_back(lines[0]);
	for(size_t i = 1; i < lines.size(); i++)
	{
		if (i == lines.size() - 1)
		{
			group.push_back(lines[i]);
			serv_blocs.push_back(group);
			break;
		}
		if(lines[i + 1] != "server")
			group.push_back(lines[i]);
		else {
			group.push_back(lines[i]);
			serv_blocs.push_back(group);
			group.clear();
		}
	}
}

void	ConfigParser::format_config_content(int ac, char **av)
{
	if (ac != 2)
		throw std::runtime_error("ERROR : executeble <filename>");
	std::ifstream file(av[1]);
	if (!file)
		throw std::runtime_error("ERROR : opening file");
	std::ostringstream ss;
	ss << file.rdbuf();
	config_content = ss.str();
	remove_comments();
	remove_newlines();
	handel_multiple_derictive_in_one_line();
	puting_acolads_in_line();
	get_servers();
}

void	ConfigParser::cheke_format_server_block(std::vector<std::string>& serv)
{
	// size_t	size = serv.size();

	if (serv[0] != "server")
		std::cerr << "Error" << serv[0] << " outsidd server block" << std::endl, exit(1);
	if (serv[1]!= "{" )
		std::cerr << "Error :server must followed by { " << std::endl, exit(1);
	if (serv[2] == "}" && serv.size() != 3)
		std::cerr << "Error :syntax erorr near } " << std::endl, exit(1);
	if (serv[serv.size() - 1] != "}")
		std::cerr << "Error" << serv[serv.size() - 1] << " outsidd server block" << std::endl, exit(1);
	int holder = 1;
	if (serv[2] == "{")
		holder++;
	if (serv[2] == "}")
		holder--;
	size_t i;
	for ( i = 3; i < serv.size(); i++)
	{
		if (serv[i] == "{")
			holder++;
		if (serv[i] == "}")
			holder--;
		if (holder == 0)
			break;
	}
	if (i != serv.size() - 1) 
	{	
		std::cerr << "Error :syntax erorr near } " << std::endl, exit(1);
	}
	for (size_t i = 2; i < serv.size(); i++)
	{
		if (serv[i] == "{")
		{
			std::vector<std::string>  tokens = split(serv[i - 1], ' ');
			if (tokens[0] != "server" && tokens[0] != "location")
				throw::std::runtime_error("{} not in wright scope");
		}
	}
}

void	ConfigParser::pars_config_centent()
{
	for (size_t i = 0; i < serv_blocs.size(); i++)
	{
		Server_holder server;
		cheke_format_server_block(serv_blocs[i]);
		pars_server(serv_blocs[i], server);
		servers.push_back(server);
	}
}

void	ConfigParser::prints_serves_configuration()
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		std::cout << "there is "<< servers.size() << " server in the config file!!!!!"<< std::endl;
		Server_holder& server = servers[i];
		std::cout << "Server " << i + 1 << " Configuration:\n";
		std::vector<int> port = server.port;
		if (!port.empty())
		{
			std::cout << "Port: " ;
			for (size_t l = 0; l < port.size(); l++)
				std::cout << port[l] << "	";
			std::cout << std::endl;
		}
		if (!server.server_name.empty())
			std::cout << "Server name: " << server.server_name << "\n";
		if (!server.host.empty())
			std::cout << "Host: " << server.host << "\n";
		std::cout << "Client Max Body Size: " << server.client_max_body_size << " bytes\n";
		std::map<int, std::string>	error_page = server.error_pages;
		if (!error_page.empty())
		{
			std::cout << "Error pages: \n";
			for (std::map<int, std::string>::iterator it = error_page.begin(); it != error_page.end(); ++it )
				std::cout <<"      "<< it->first << ":" << it->second << std::endl; 
		}
		std::vector<Location> locations = server.locations;
		for (size_t j = 0; j < locations.size(); j++)
		{
			std::cout << "Total Servers: " << servers.size() << std::endl;
			const Location& loc = locations[j];
			std::cout << "\n  Location ";
			if (!loc.path.empty())
				std::cout << loc.path << std::endl;
			if (!loc.root.empty())
				std::cout << "    Root: " << loc.root << std::endl;
			if (!loc.index.empty())
			{
				std::cout << "    Index: ";
				for (size_t l = 0; l < loc.index.size(); l++)
					std::cout << loc.index[l] << " ";
				std::cout << std::endl;
			}
			std::cout << "    Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
			std::cout << "    Client Max Body Size: " << loc.client_max_body_size << std::endl;
			if (loc.redirect_code)
				std::cout << "    redirect code: " << loc.redirect_code << std::endl;
			if (!loc.redirect_url.empty())
				std::cout << "    redirect url: " << loc.redirect_url << std::endl;
			if (!loc.allowed_methods.empty())
			{
				std::cout << "    allowed methods: " ;
				for (size_t l = 0; l < loc.allowed_methods.size(); l++)
					std::cout << loc.allowed_methods[l] << " ";
				std::cout << std::endl;
			}
			if (!loc.cgi_extension.empty())
			{
				std::cout << "    cgi extention: " ;
				for (size_t l = 0; l < loc.cgi_extension.size(); l++)
					std::cout << loc.cgi_extension[l] << " ";
				std::cout << std::endl;
			}
		}
		std::cout << "================" << std::endl; 
	}
}


void	ConfigParser::process_config(int ac, char **av)
{
	format_config_content(ac, av);
	// for (size_t i = 0; i < config.serv_blocs.size(); i++)
	// {
	// 	for (int j = 0; j < config.serv_blocs[i].size(); j++)
	// 	{
	// 		std::cout << config.serv_blocs[i][j] << std::endl;
	// 	}
	// }
	pars_config_centent();
	// config.prints_serves_configuration();
}