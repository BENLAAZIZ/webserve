#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "web.h"

#include <cstdio>
#include <sstream>
#include <unistd.h> 


class Cgi 
{
	public:
		Request	req;
		std::string	path;
		std::string	path_info;
		std::string	query;
		std::string	ext;
		std::string myroot;
		std::string	script_name;
		std::string	method;
		std::string	centent_type;
		std::string	centent_length;
		std::string	http_coockie;
		std::string http_user_agent;
		std::string	bin_path;

		Cgi(Request & req);
		char**	init_env();
		char**	set_args_cgi();
		void	free_2d(char **str);
		void	set_server_name_port(std::map<std::string, std::string>  &  env_var);
		int	execute_cgi(std::string & result);
};

#endif