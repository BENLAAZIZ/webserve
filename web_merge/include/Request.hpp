#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "web.h"


class Request 
{
	private:
		std::string	method;
		std::string	path;
		std::string	version;
		std::string	extension;
		std::map<std::string, std::string> headers;
		
		bool		bodyFlag;
		bool		flag_end_of_headers;
		std::size_t	code;
		// bool headersParsed;
	public:
		std::string	body; 
		std::string	buffer; 
		std::string content_type;
		std::string boundary;
		size_t 		content_length;
		bool 		transferEncodingExist;
		std::string _requestBuffer;

		Request();
		~Request();
		Request(const Request& other);
		Request& operator=(const Request& other);
		// Getters
		std::string	getMethod() const;
		std::string	getpath() const;
		std::string	getVersion() const;
		std::string	get_MimeType () const;
		std::string	getHeader(const std::string& key) const;
		std::string	get_error_missage(int errorCode) const;
		std::string	getBody() const;
		std::string	getContent_type() const;
		std::size_t	getContentLength() const;
		std::string getBoundary() const;
		bool		getFlagEndOfHeaders() const;
		bool		getBodyFlag() const;
		int			getStatusCode() const ;
		bool 		getTransferEncodingExist() const;

		const std::map<std::string, std::string>& getHeaders() const;
		// Setters
		void		setMethod(const std::string& method);
		void		setPath(const std::string& path);
		void		setVersion(const std::string& version);
		void		setHeader(const std::string& key, const std::string& value);
		void		setBody(const std::string& body);
		void		setContentLength(int contentLength);
		void		setFlagEndOfHeaders(bool flag);
		void		setBodyFlag(bool flag);
		// void 		setTransferEncodingExist(bool flag);
		void		setContent_type(const std::string& content_type);
		void		setBoundary(const std::string& boundary);
		void 		set_status_code(int code);
		bool		parseFirstLine(const std::string& line);
		void 		reset();
		bool		checkPath(std::string& path);
		// void		initializeEncode();

		std::string urlDecode(const std::string& str);


};

#endif // REQUEST_HPP