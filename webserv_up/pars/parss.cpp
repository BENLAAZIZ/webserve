#include "../include/web.h"

bool	is_digit(std::string & str)
{
	for(size_t i = 0; i < str.length(); i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

void	validate_host(std::string & str)
{
	size_t i = 0;
	size_t l = 0;
	int	point = 0;
	if (str.length() > 15)
		throw std::runtime_error("invalid host");
	std::string	holder;
	if (str[0] == '.')
		throw std::runtime_error(". at the beging of ip");
	while (str[i]){
		if (isdigit(str[i]))
		{
			l++;
			holder += str[i];
		}
		else if (str[i] == '.')
		{
			point++;
			if (l == 0 || l > 3)
				throw std::runtime_error("invalid host");
			if (std::atoi(holder.c_str()) > 255)
				throw std::runtime_error("nbr > 255 in ip");
			holder = "";
			l = 0;
		}
		else
			throw std::runtime_error("carracter other than piont or digit in ip");
		i++;
	}
	if (point != 3)
		throw std::runtime_error("invalid host");
}

void	check_number_argment(std::vector<std::string> &tokens, size_t nbr)
{
	if (tokens.size() != nbr)
		throw::std::runtime_error("nembre of argment for " + tokens[0] + " directive");
}

void	check_allowed_method(std::vector<std::string> & tokens)
{
	size_t	size = tokens.size();

	for (size_t i = 1; i < size; i++)
	{
		if (tokens[i] != "POST" && tokens[i] != "GET" && tokens[i] != "DELETE")
			throw::std::runtime_error("unkown allowed method" + tokens[i]);
	}

	for (size_t i = 1; i < size - 1 ; i++)
	{
		for (size_t j = i + 1; j < size ; j++)
		{
			if (tokens[i] == tokens[j])
				throw::std::runtime_error("duplicated method");
		}
	}
}

std::string trim(const std::string& str, const std::string& tokens) {
    size_t first = str.find_first_not_of(tokens);
    if (first == std::string::npos) 
        return "";
    size_t last = str.find_last_not_of(tokens);
    return str.substr(first, last - first + 1);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        token = trim(token, " \t");
        if (!token.empty())
            tokens.push_back(token);
    }
    return tokens;
}

std::vector<std::string>	check_semi_colen(std::string& line)
{
	std::vector<std::string>	tokens = split(line, ' ');;
	int	id_last_token = tokens.size() - 1;
	if (tokens.size() == 1 && tokens[0] == ";")
		throw std::runtime_error("Empty directive");
	if ( tokens[0] == "location" || tokens[0] == "server" || tokens[0] == "{" || tokens[0] == "}")
		return tokens;
	if (tokens[id_last_token] != ";")
		std::cerr<< "missing ;" << std::endl, exit(1);
	tokens.pop_back();
	return tokens;
}

void	pars_location(std::vector<std::string>& tokens, Location &loc)
{
	bool	begin_loc = 0;
	if (tokens[0] == "location")
	{
		if (tokens.size() == 1)
			throw::std::runtime_error("path missing aftet the location block");
		if (tokens.size() != 2)
			throw::std::runtime_error("sytaxe error in the location block");
		if (tokens[1][0] != '/')
			throw::std::runtime_error("path must begen whit /"); 
		loc.path = tokens[1];
		begin_loc = 1;
		return ;
	}
	else if (begin_loc && (tokens[0] != "{" && tokens.size()) != 1)
		std::cerr << "Error structer of block location" << std::endl, exit(1);
	else if (tokens[0] == "root")
	{
		check_number_argment(tokens, 2);
		loc.root = tokens[1];
	}
	else if (tokens[0] == "allowed_methods")
	{
		if (tokens.size() < 2 || tokens.size() > 4)
			throw::std::runtime_error("nembre of argement for allowed_method directive");
		check_allowed_method(tokens);
		for (size_t i = 1; i < tokens.size(); i++)
			loc.allowed_methods.push_back(tokens[i]);
	}
	else if (tokens[0] == "autoindex")
	{
		check_number_argment(tokens, 2);
		if (tokens[1] == "on")
			loc.autoindex = 1;
	}
	else if (tokens[0] == "upload_store")
	{
		check_number_argment(tokens, 2);
		loc.upload_store = tokens[1];
	}
	else if (tokens[0] == "client_max_body_size")
	{
		check_number_argment(tokens, 2);
		if (!is_digit(tokens[1]))
			throw::std::runtime_error("non digit caracter in client max body size");
		loc.client_max_body_size = std::atoi(tokens[1].c_str());
	}
	else if (tokens[0] == "redirect" && !loc.redirect_code)
	{
		if (is_digit(tokens[1]))
			loc.redirect_code = std::atoi(tokens[1].c_str());
		else
			throw::std::runtime_error("invalid redirect code " + tokens[1]);
		if (tokens.size() == 3)
		{
			loc.redirect_url = tokens[2];
		}
	}
	else if (tokens[0] == "index")
	{
		if (tokens.size() < 2)
			throw::std::runtime_error("nembre of argment for " + tokens[0] + " directive");
		for (size_t i = 1; i < tokens.size(); i++)
		{
			loc.index.push_back(tokens[i]);
		}
	}
	else if (tokens[0] == "cgi_extention")
	{
		if (tokens.size() < 2)
			throw::std::runtime_error("nembre of argment for " + tokens[0] + " directive");
		for (size_t i = 1; i < tokens.size(); i++)
		{
			if (tokens[i] != ".py" || tokens[i] != ".php")
				throw::std::runtime_error("inavalid extention" + tokens[i]);
			loc.cgi_extension.push_back(tokens[i]);
		}
	}
	else if (tokens[0] == "cgi_handler")
	{
		check_number_argment(tokens, 2);
		loc.cgi_handler = tokens[1];
	}
	begin_loc = 0;
}

void	pars_server(std::vector<std::string>& serv, Server_holder & server)
{
	for (size_t i = 2; i < serv.size() - 1 ; i++)
	{
		// server.root = "/docs";
		std::vector<std::string> tokens = check_semi_colen(serv[i]);
		if (tokens[0] == "listen")
		{
			// check_number_argment(tokens, 2);
			if (tokens.size() < 2)
				throw::std::runtime_error("nembre of argment for " + tokens[0] + " directive");
			for (size_t i = 1; i < tokens.size(); i++)
			{
				if (!is_digit(tokens[i]))
					throw std::runtime_error("non digit caracter in port");
				int	nbr = std::atoi(tokens[i].c_str());
				for (size_t j = 0; j < server.port.size(); j++)
				{	
					if (nbr == server.port[j])
						throw::std::runtime_error("port number id dublicated");
				}
				server.port.push_back(nbr);
			}
		}
    	else if (tokens[0] == "host")
		{
			check_number_argment(tokens, 2); 
			if(tokens[1] != "localhost")
				validate_host(tokens[1]);
			if (tokens[1] == "localhost")
				tokens[1] = "127.0.0.1";
           	server.host = tokens[1];
		}
		else if (tokens[0] == "server_name")
		{
			check_number_argment(tokens, 2);
			server.server_name = tokens[1];
		}
        else if (tokens[0] == "client_max_body_size")
		{
			check_number_argment(tokens, 2);
			if (!is_digit(tokens[1]))
				throw::std::runtime_error("non digit caracter in client max body size");
			server.client_max_body_size = std::atol(tokens[1].c_str());
		}
		else if (tokens[0] == "error_page" )
		{
			check_number_argment(tokens, 3);
			if (!is_digit(tokens[1]))
				throw::std::runtime_error("invalid status code");
			int nbr = std::atoi(tokens[1].c_str());
			if (nbr < 300 || nbr > 599)
				throw::std::runtime_error("value of argement of error page must be betwen 300 or 599");
				server.error_pages[nbr] = tokens[2];
		}
		else if (tokens[0] == "root")
		{
			check_number_argment(tokens, 2);
			server.root = tokens[1];
		}
		else if (tokens[0] == "location")
		{
			Location loc;
			loc.client_max_body_size = 1024 * 1024;
			loc.autoindex = 0;
			loc. redirect_code = 0;
			while (1)
			{
				if (tokens[0] == "}")
					break;
				pars_location(tokens, loc);
				i++;
				tokens = check_semi_colen(serv[i]);
			}
			server.locations.push_back(loc);
		}
		else
			throw::std::runtime_error("unkowng directive" + tokens[0] + "for server block");
	}
}


// fix redirect url