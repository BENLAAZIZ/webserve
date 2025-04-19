#include "../include/web.h"

Cgi::Cgi(Request & _req):req(_req)
{
	ext = req.getExtension();
	path_info = _req.path_info;
	query = _req.query;
	path = (_req.getpath()).substr(0, _req.getpath().length() - path_info.length());
	myroot = _req.my_root;
	script_name = (_req.get_fake_path()).substr(0, _req.get_fake_path().length() - path_info.length());
	method = _req.getMethod();
	centent_length = _req.getContentLength();
	centent_type = _req.getContent_type();
	http_coockie = _req.getHeader("Cookie");
	http_user_agent = _req.getHeader("User-Agent");
	if(ext == ".py")
		bin_path = myroot + "/docs/cgi_bin/bin_py";
	else
		bin_path = myroot + "/docs/cgi_bin/bin_php";	
	// std::cout << "path = " << path << std::endl;
	// std::cout << "path_info = " << path_info << std::endl;
	// std::cout << "Query_string= " << query << std::endl;
	// std::cout << "root = " << myroot << std::endl;
	// std::cout << "extention = " << ext << std::endl;
	// std::cout << "script_name = " << script_name << std::endl;
	// std::cout << "centent_length = " << centent_length << std::endl;
	// std::cout << "centent_type = " << centent_type << std::endl;
	// std::cout << "coockie = " << http_coockie << std::endl;
	// std::cout << "user_agent = " << http_user_agent << std::endl;
}

char **set_envarement(const std::map<std::string, std::string> &env_var) {
    char **env = new char *[env_var.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::const_iterator it = env_var.begin(); it != env_var.end(); ++it)
	{
        std::string env_str = it->first + "=" + it->second;
		env[i] = strdup(env_str.c_str());
        i++;
    }
    env[i] = NULL;
    return env;
}

void    print_env_var(std::map<std::string, std::string> & env_var)
{
    std::cout << "-----------env--------------\n";
    for(std::map<std::string, std::string>::iterator it = env_var.begin(); it != env_var.end(); ++it)
    {
        std::cout << it->first << " ===> " << it->second << std::endl;
    }
    std::cout << "--------------------\n";
}

void	Cgi::set_server_name_port(std::map<std::string, std::string>  &  env_var)
{
	std::string	host = req.getHeader("host");
	size_t	index = host.find(":");
	if (index == std::string::npos)
	{
		env_var["SERVER_NAME"] = host;
		env_var["SERVER_PORT"] = "80";
	}
	else
	{
		env_var["SERVER_NAME"] = host.substr(0, index);
		env_var["SERVER_PORT"] = host.substr(index + 1);
	}
}

char**	Cgi::init_env()
{
	std::map<std::string, std::string> env_var;

    env_var["AUTH_TYPE"] = "Basic";
	env_var["REQUEST_METHOD"] = method;
	env_var["PATH_INFO"] = path_info;
	env_var["CONTENT_TYPE"] = centent_type;
	env_var["CONTENT_LENGTH"] = centent_length;
	env_var["QUERY_STRING"] = query;
	set_server_name_port(env_var);
	env_var["GETWAY_INTERFACE"] = "CGI/1.1";
	env_var["SERVER_PROTOCOL"] = "HTTP/1.1";
	env_var["REDIRECT_STATUS"] = "200";
	env_var["SCRIPT_NAME"] = script_name;
	env_var["HTTP_COOKIE"] = http_coockie;
	env_var["HTTP_USER_AGENT"] = http_user_agent;
	env_var["SCRIPT_FILENAME"] = path;
	if (env_var["PATH_INFO"].length())
		env_var["PATH_TRANSLATED"] = myroot + env_var["PATH_INFO"]; 

    // print_env_var(env_var);
	char **env  = set_envarement(env_var);
	return (env);
}

char**	Cgi::set_args_cgi()
{
	char **arg_cgi = new char*[3];

	arg_cgi[0] = strdup(bin_path.c_str());
	arg_cgi[1] = strdup(path.c_str());
	arg_cgi[2] = NULL;

	return (arg_cgi);
}

void	Cgi::free_2d(char **str)
{
	int i = 0;
	while (str[i])
	{
		delete[] str[i];
		i++;
	}
	delete[] str;
}

int	Cgi::execute_cgi(std::string & result)
{
	int pipe_fd[2];
	int std_in = dup(STDIN_FILENO);
	int std_out = dup(STDOUT_FILENO);

	char **env = init_env();
	char **arg_cgi = set_args_cgi();
	result = "";
	if (pipe(pipe_fd) < 0)
	{
	    close(pipe_fd[0]);
        close(pipe_fd[1]);
        close(std_in);
        close(std_out);
		free_2d(env);
		free_2d(arg_cgi);
		std::cerr << "PIPE FAILED" << std::endl;
		return (500);
	}
	pid_t	pid;
	if ((pid = fork()) == -1)
	{
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        close(std_in);
        close(std_out);
		free_2d(env);
		free_2d(arg_cgi);
		std::cerr << "fork failed" << std::endl;
		return (500);
	}
	if (pid == 0)
	{
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO );
        close(pipe_fd[1]);
		execve(arg_cgi[0], arg_cgi, env);
		std::cerr << "execve failed" << std::endl;
        dup2(STDOUT_FILENO, std_out);
        close(std_in);
        close(std_out);
		free_2d(env);
		free_2d(arg_cgi);
		exit(127);
	}
	else 
	{
		close(pipe_fd[1]);
        int status;
        waitpid(pid, & status, 0);
        if (!WEXITSTATUS(status))
        {
            char	buffer[1000];
            size_t	beytes_read;
            while ((beytes_read = read(pipe_fd[0], buffer, sizeof(buffer))))
            {
                if (beytes_read > 0)
                {
                    result. append(buffer, beytes_read);
                }
                else
                    break;
            }
        }
        close(pipe_fd[0]);
	}
	dup2(std_in, STDIN_FILENO);
	dup2(std_out, STDOUT_FILENO);
	close(std_in);
	close(std_out);
	free_2d(env);
	free_2d(arg_cgi);
	return (0);
}
