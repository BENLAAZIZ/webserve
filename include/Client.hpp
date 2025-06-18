
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "web.h"

class Client {
	public:
		time_t _lastActive;
		void updateActivityTime();
		time_t getLastActivityTime() const;
		bool endOfResponse;

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
		

		void handlePostRequest();
		void handlePostResponse();
		void setClientFd(int client_fd);
		int getClientFd() const;
	public:
		Client();
		~Client();
		Client(const Client& other);
		Client& operator=(const Client& other);
		bool is_Header_Complete();
		bool parse_Header_Request(Server_holder & serv_hldr);

		bool keepAlive() const;
		void reset();
		void end_of_headers(std::string& line, int *flag);
		bool generate_header_map(std::string& line);
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