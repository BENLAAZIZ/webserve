#include "../include/web.h"

void	ConfigParser::remove_comments()
{
	size_t i = 0; 
	std::string	result;

	while (config_content[i])
	{
		if (config_content[i] == '#')
		{
			while (config_content[i] && config_content[i] != '\n')
				i++;
		}
		if (config_content[i])
		{
			result += config_content[i];
			i++;
		}
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

bool is_directory_func(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return S_ISDIR(info.st_mode);
}


void	ConfigParser::format_config_content(int ac, char **av)
{

	std::string config_file;

    if (ac > 2)
        throw std::runtime_error("Too many arguments. Usage: ./webserv [config_file]");

    if (ac == 1)
        config_file = "docs/config/webserv.conf";
    else
        {config_file = av[1];}
	if (is_directory_func(std::string(config_file.c_str())))
	{

		throw std::runtime_error("ERROR : opening file derictory!!!!");
	}
    std::ifstream file(config_file.c_str());
    if (!file.is_open())
       { throw std::runtime_error("Cannot open config file: " + config_file);}

	std::ostringstream ss;
	ss << file.rdbuf();
	file.close();
	config_content = ss.str();
	remove_comments();
	remove_newlines();
	handel_multiple_derictive_in_one_line();
	puting_acolads_in_line();
	if (!config_content.length())
		{throw std::runtime_error("empty config file");}
	get_servers();
}

void	ConfigParser::cheke_format_server_block(std::vector<std::string>& serv)
{
	// size_t	size = serv.size();

	if (serv[0] != "server")
		throw::std::runtime_error("Error : " + serv[0] + " outsidd server block");
	if (serv[1]!= "{" )
		throw::std::runtime_error("Error :server must followed by { ");
	if (serv[2] == "}" && serv.size() != 3)
		throw::std::runtime_error("Error :syntax erorr near } ");
	if (serv[serv.size() - 1] != "}")
		throw::std::runtime_error("Error " + serv[serv.size() - 1] + " outsidd server block");
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
		throw::std::runtime_error("Error :syntax erorr near } ");
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

void	chek_server_essentails(Server_holder & server)
{
	if (server.host.empty())
		throw::std::runtime_error("invalid server block host missing");
	if (server.port.empty())
		throw::std::runtime_error("invalid server block port missing");
}

void    ConfigParser::chek_duplicated_server()
{
    int    port;
    std::string    host;
    std::string server_name;
    for (size_t i = 0; i < servers.size(); i++)
    {
        host = servers[i].host;
        server_name = servers[i].server_name;
        for (size_t j = 0; j < servers[i].port.size(); j++)
        {
            port = servers[i].port[j];
            for (size_t z = i + 1; z < servers.size(); z++)
            {
                if ( host == servers[z].host && server_name == servers[z].server_name)
                {
                    for (size_t k = 0; k < servers[z].port.size(); k++)
                    {
                        if(port == servers[z].port[k])
                            throw::std::runtime_error("duplicated server!!!");
                    }
                }
            }
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
		// check esentials to be in server
		chek_server_essentails(server);
		servers.push_back(server);
		chek_duplicated_server(); 
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
		if (!server.root.empty())
			std::cout << "Default Root: " << server.root << "\n";
		if (!server.host.empty())
			std::cout << "Host: " << server.host << "\n";
		std::cout << "Client Max Body Size: " << server.client_max_body_size << " bytes\n";
		std::map<int, std::vector<std::string> >    error_page = server.error_pages;
        if (!error_page.empty())
        {
            std::cout << "Error pages: \n";
            for (std::map<int, std::vector<std::string> >::iterator it = error_page.begin(); it != error_page.end(); ++it )
            {
                std::cout <<"      "<< it->first << ":" ;
                for (size_t i = 0; i < it->second.size(); ++i)
                {
                       std::cout << " " << it->second[i];
                }
                std::cout << "\n";
            }
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
			// std::cout << "    Client Max Body Size: " << loc.client_max_body_size << std::endl;
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
	pars_config_centent();
}