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

        // Setters

        // Getters
        int getStatus() const;
        // const std::string& getHttpVersion() const;
        // const std::map<std::string, std::string>& getHeaders() const;
        // const std::string& getBody() const;

        // Methods
        void reset();
        // bool sendResponse(int client_fd);
        // int handleGetRequest();
        void handleGetRequest(int *flag);

        // void genetate_error_response(int statusCode,  int client_fd);
        // std::string	get_error_missage(int errorCode) const;

        // std::string	get_code_error_path(int errorCode) const;

        bool keepAlive() const;

        std::string get_MimeType (const std::string& path);

};

#endif // RESPONSE_HPP