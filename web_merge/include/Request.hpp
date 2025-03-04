#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <vector>

enum RequestParseState {
    PARSE_START_LINE,
    PARSE_HEADERS,
    PARSE_BODY,
    PARSE_CHUNKED_BODY,
    PARSE_COMPLETE,
    PARSE_ERROR
};

enum RequestMethod {
    GET,
    POST,
    DELETE,
    UNKNOWN
};

// class Request {
// private:
//     // Parse state
//     RequestParseState _parseState;
//     size_t _parsePosition;
    
//     // Request components
//     RequestMethod _method;
//     std::string _uri;
//     std::string _httpVersion;
//     std::map<std::string, std::string> _headers;
//     std::string _body;
    
//     // For chunked encoding
//     size_t _chunkSize;
//     bool _lastChunk;
    
//     // Internal buffer
//     std::string _buffer;
    
//     // Parsing helper methods
//     bool parseStartLine();
//     bool parseHeaders();
//     bool parseBody();
//     bool parseChunkedBody();
//     RequestMethod stringToMethod(const std::string& method);
    
// public:
//     Request();
//     ~Request();
    
//     // Buffer management
//     void appendData(const char* data, size_t length);
//     void clear();
    
//     // Incremental parsing
//     bool parse();
    
//     // State getters
//     bool isComplete() const;
//     bool hasError() const;
    
//     // Request info getters
//     RequestMethod getMethod() const;
//     const std::string& getUri() const;
//     const std::string& getHttpVersion() const;
//     const std::map<std::string, std::string>& getHeaders() const;
//     std::string getHeader(const std::string& key) const;
//     const std::string& getBody() const;
    
//     // For debugging
//     void dump() const;
// };



typedef struct StatusCode 
{
	std::size_t	code;
	std::string	message;
} StatusCode;

class Request 
{
	private:
		std::string	method;
		std::string	path;
		std::string	version;
		std::string	extension;
		StatusCode statusCode;
		std::map<std::string, std::string> headers;
		
		bool bodyFlag;
		bool flag_end_of_headers;
		// int statusCode;
		bool headersParsed;
	public:
		std::string	body; 
		std::string content_type;
		std::string boundary;
		size_t content_length;
		bool transferEncodingExist;
		HTTPRequest();
		~HTTPRequest();
		HTTPRequest(const HTTPRequest& other);
		HTTPRequest& operator=(const HTTPRequest& other);
		// Getters
		std::string	getMethod() const;
		std::string	getpath() const;
		std::string	getVersion() const;
		std::string	getExtension() const;
		std::string	getHeader(const std::string& key) const;
		std::string	getStatusCodeMessage() const;
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
		void 		setTransferEncodingExist(bool flag);
		void		setContent_type(const std::string& content_type);
		void		setBoundary(const std::string& boundary);
		// Methods
		void		sendErrorResponse(int errorCode);
		bool		parseFirstLine(const std::string& line);
		bool		parseHeader(std::string& line);
};

#endif // REQUEST_HPP