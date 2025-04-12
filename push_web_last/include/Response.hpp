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
        // stuct of location 
        // Location        _location;
        off_t			fileSize;


    public:
        Response();
        ~Response();
        Response(const Response& other);
		Response& operator=(const Response& other);


        // Getters
        // int getStatus() const;

        // Methods

        // bool        is_CGI();
        bool        sendResponse(int client_fd);
        bool        keepAlive() const;
        void        handleGetResponse(int *flag, Request &request);
        void        reset();
        void        generate_error_response(int statusCode,  int client_fd);
        std::string	get_error_missage(int errorCode) const;
        std::string	get_code_error_path(int errorCode) const;
        std::string get_MimeType (const std::string& path);
        // void        send_header_response(size_t CHUNK_SIZE);
        void        send_header_response(size_t CHUNK_SIZE, std::string path);
        int        send_file_response(char *buffer, int bytes_read);
        int         open_file(int *flag, std::string fullPath, int *code);
        //=============

        // std::string resolve_request_path(const std::string& uri, std::vector<Location>& locations, const std::string& default_root)
        //Location* find_matching_location(const std::string& uri, std::vector<Location>& locations);
        void resolverequest_path22(std::string& path);

};

#endif // RESPONSE_HPP