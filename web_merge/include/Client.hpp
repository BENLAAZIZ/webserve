
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "web.h"

class Client {
	public:
		// Response _response;
		Request _request;
		int _clientFd;
		std::string _requestBuffer;
		std::string _responseBuffer;
		bool request_Header_Complete;
		bool _responseSent;
		bool _keepAlive;
		bool _header_falg;
		bool _isopen;
		std::ifstream file;
		std::ofstream filetest;
		int fd;
		// ofset for file
		size_t _fileOffset;

	bool _isChunkedFile;
	std::ifstream _fileToSend;
	size_t _fileBytesRemaining;
	size_t _chunkSize;
		
		// Helper methods for handling different HTTP methods
		int handleGetRequest();
		// void handlePostRequest();
		void handleDeleteRequest();
		// Helper for determining content type
		std::string get_MimeType (const std::string& path);
		// set client_fd
		void setClientFd(int client_fd);
		// get client_fd
		int getClientFd() const;
	public:
		Client();
		// Client(int socket, struct sockaddr_in address);
		~Client();
		Client(const Client& other);
		Client& operator=(const Client& other);
		// Check if the request is complete
		bool is_Header_Complete();
		// Parse HTTP request
		bool parse_Header_Request(std::string& line_buf);
		// Generate HTTP response
		int generateResponse_GET_DELETE();
		// Send HTTP response
		bool sendResponse(int client_fd);
		// Send error response
		void genetate_error_response(int statusCode, int client_fd);
		// Check if response is completely sent
		bool isDoneWithResponse() const;
		// Check if connection should be kept alive
		bool keepAlive() const;
		// Reset client for new request
		void reset();
		// end_of_headers
		void end_of_headers(std::string& line, int *flag);
		bool generate_header_map(std::string& line);

		std::string	get_code_error_path(int errorCode) const;
};
#endif // CLIENT_HPP