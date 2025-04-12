
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "web.h"

class Client {
	public:
		std::ifstream	file;
		std::ofstream	filetest;
		Request			_request;
		Response		_response;
		bool			request_Header_Complete;
		bool			_responseSent;
		bool			_keepAlive;
		bool			_header_falg;
		bool			_isopen;
		int				_clientFd;
		size_t			_fileOffset;
		
		void handlePostRequest();
		void setClientFd(int client_fd);
		int getClientFd() const;
	public:
		Client();
		~Client();
		Client(const Client& other);
		Client& operator=(const Client& other);
		// Check if the request is complete
		bool is_Header_Complete();
		// Parse HTTP request
		bool parse_Header_Request();
		bool keepAlive() const;
		// Reset client for new request
		void reset();
		// end_of_headers
		void end_of_headers(std::string& line, int *flag);
		bool generate_header_map(std::string& line);
		int read_data();
};
#endif // CLIENT_HPP