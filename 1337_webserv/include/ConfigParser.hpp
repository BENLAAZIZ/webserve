#ifndef	ConfigParser_HPP
#define ConfigParser_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stack>
#include <cctype>

typedef  struct  Location {
	bool auto_index_set;
	std::string path;
	std::string root;
	std::vector<std::string> index;
	std::string upload_store;
	std::vector<std::string> allowed_methods;
	std::vector<std::string> cgi_extension;
	std::string cgi_handler;
	bool autoindex;
	int redirect_code;
	std::string redirect_url;
} Location;

typedef struct {
	bool	is_default;
	std::vector<int> port;
	std::string host;
	std::string root;
	std::string server_name;
	std::size_t client_max_body_size;
	std::map<int, std::vector<std::string> > error_pages;
	std::vector<Location> locations;
} Server_holder;



class ConfigParser {
	public:
		std::vector<Server_holder> servers;
		
		std::string	config_content;
		std::vector<std::string> lines;
		std::vector<std::vector<std::string> >	serv_blocs;

		void	remove_comments();
		void	remove_newlines();
		void	handel_multiple_derictive_in_one_line();
		void	puting_acolads_in_line();
		void	format_config_content(int ac, char **av);
		void	cheke_format_server_block(std::vector<std::string>& serv);
		void	checke_syntax(std::vector<std::string>& line);
		void	get_servers();

		//getter
		std::vector<Server_holder>& getServers();

		void	prints_serves_configuration();
		void	pars_config_centent();
		void    chek_duplicated_server();
		void	process_config(int ac, char **av);
};


void	pars_server(std::vector<std::string>& serv, Server_holder & server);
std::vector<std::string> split(const std::string& str, char delimiter);

#endif