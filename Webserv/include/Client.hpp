
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "web.h"

class Client {
	public:
		// ----- timeout -----//
		// unsigned long _lastActive;
		time_t _lastActive;
		void updateActivityTime();
		time_t getLastActivityTime() const;
		bool endOfResponse;
		// unsigned long getLastActivityTime() const;
		// ----- end timeout -----// 
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

		std::string 	full_path;

		bool			_isResolved;
		
		// Helper methods for handling different HTTP methods
		// void handleGetRequest();
		// void handlePostRequest();
		// void handleDeleteRequest();

		void handlePostRequest();
		//response 
		void handlePostResponse();
		// Helper for determining content type
		// std::string getExtension(const std::string& path);
		// std::string get_MimeType (const std::string& path);
		// set client_fd
		void setClientFd(int client_fd);
		// get client_fd
		int getClientFd() const;
	public:
		Client();
		~Client();
		Client(const Client& other);
		Client& operator=(const Client& other);
		// Check if the request is complete
		bool is_Header_Complete();
		// Parse HTTP request
		bool parse_Header_Request(Server_holder & serv_hldr);
		// Generate HTTP response
		// int generateResponse_GET_DELETE();
		// Send HTTP response
		// bool sendResponse(int client_fd);
		// Send error response
		// void genetate_error_response(int statusCode, int client_fd);
		// Check if connection should be kept alive
		bool keepAlive() const;
		// Reset client for new request
		void reset();
		// end_of_headers
		void end_of_headers(std::string& line, int *flag);
		bool generate_header_map(std::string& line);
		// std::string	get_code_error_path(int errorCode) const;
		// int read_data(int client_fd);
		int read_data();


		void sendSuccessResponse(int clientSocket, const std::string& path);



		int  resolve_request_path(Server_holder & serv_hldr);
		Location* find_matching_location(Request &request, std::vector<Location>& locations);
		bool is_directory(const std::string& path);
		std::string join_paths(const std::string& a, const std::string& b);
		bool has_trailing_slash(const std::string& path);
		bool file_exists(const std::string& path);
		std::string generate_directory_listing(const std::string& path);

		bool is_resolved();
		void set_resolved(bool resolved);

		bool handleDeleteResponse();

};
#endif // CLIENT_HPP