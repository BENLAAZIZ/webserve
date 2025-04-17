
#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "web.h"


class Response {
    public:

        std::ifstream	file;
		std::ofstream	filetest;
		std::string		_responseBuffer;
		bool			_responseSent;
		bool			_keepAlive;
		bool			_header_falg;
		bool			_isopen;
		int				_clientFd;
		size_t			_fileOffset;
        int				fd;
        std::string		fullPath;
        int flag_p;
        int is_file;
        int is_dir;
        int bytes_sent;
        off_t			fileSize;

        // const size_t CHUNK_SIZE = 1024;
        size_t CHUNK_SIZE;


    public:
        Response();
        ~Response();
        Response(const Response& other);
		Response& operator=(const Response& other);

        // Methods

    
        bool        keepAlive() const;
        void        handleGetResponse(int *flag, Request &request, int flag_delete);
        void        reset();
        std::string	get_status_missage(int errorCode) const;
        std::string	get_code_error_path(int errorCode) const;
        std::string get_MimeType (const std::string& path);
        void        send_header_response(size_t CHUNK_SIZE, std::string path, Request &request, int flag_autoindex);
        int         send_file_response(char *buffer, int bytes_read);
        int         open_file(int *flag, std::string fullPath, int *code);
        //=============

        void type_of_path(std::string& path);// mo2a
        void send_header_response_autoIndex(std::string path, Request &request);

        bool send_Error_Response(int client_fd);
        void there_is_error_file(std::string fullPath, int statusCode);
        void generate_default_error_response(int statusCode);
        void generate_error_response(int statusCode,  int client_fd, Server_holder& serv_hldr);

        //  bool handleDeleteResponse(Client &client, Server_holder &serv_hldr);

};

#endif // RESPONSE_HPP