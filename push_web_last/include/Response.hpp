#ifndef RESPONSE_HPP
#define RESPONSE_HPP


#include "web.h"


class Response {
    public:
        // bool _header_falg;
        // std::string _responseBuffer;
        // bool _responseSent;
        // bool _keepAlive;

        std::ifstream	file;
		std::ofstream	filetest;
		// std::string		_requestBuffer;
		std::string		_responseBuffer;
		Request			_request;
		// Responce		_responce;
		// bool			request_Header_Complete;
		bool			_responseSent;
		bool			_keepAlive;
		bool			_header_falg;
		bool			_isopen;
		int				_clientFd;
		size_t			_fileOffset;


    public:
        Response();
        ~Response();
        Response(const Response& other);
		Response& operator=(const Response& other);


        // Getters
        int getStatus() const;

        // Methods

        bool        is_CGI();
        bool        sendResponse(int client_fd);
        bool        keepAlive() const;
        void        handleGetResponse(int *flag);
        void        reset();
        void        generate_error_response(int statusCode,  int client_fd);
        std::string	get_error_missage(int errorCode) const;
        std::string	get_code_error_path(int errorCode) const;
        std::string get_MimeType (const std::string& path);
        // void        send_header_response(size_t CHUNK_SIZE);
        void        send_header_response(size_t CHUNK_SIZE, std::string path);
        int        send_file_response(char *buffer, int bytes_read);
        int         open_file(int *flag, std::string fullPath);


};

#endif // RESPONSE_HPP