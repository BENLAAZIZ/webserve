#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "web.h"

// enum RequestParseState {
//     PARSE_START_LINE,
//     PARSE_HEADERS,
//     PARSE_BODY,
//     PARSE_CHUNKED_BODY,
//     PARSE_COMPLETE,
//     PARSE_ERROR
// };

// enum RequestMethod {
//     GET,
//     POST,
//     DELETE,
//     UNKNOWN
// };


struct FormData{
    std::string name;
    std::string filename;
    std::string contentType;
    std::string data;
	int fd;
	int flagData;
	size_t pos_new_line;
	std::string dataCrlf;
	int flag;
	std::string header;
	std::size_t pos;
	std::size_t totalBytesWritten;
	std::size_t totalBytesRead;
	std::size_t totalBytes;
};

struct ChunkedData {
    ssize_t chunkSize;
    std::size_t bytesRead;
    std::size_t bytesRemaining;
    std::string data;
    std::string strHex;
    int fd;
    std::string end; // 0\r\n\r\n
    int flag;
	bool isFinished;
};

class Request 
{
	private:
		std::string	method;
		std::string	path;
		std::string	version;
		std::string	extension;
		std::string query;
		std::map<std::string, std::string> headers;
		
		bool bodyFlag;
		bool flag_end_of_headers;
		bool headersParsed;
	public:
		int code;
		bool 			endOfRequest;
		bool 			isCGI;
		std::map<std::string, std::string> encode;
		// StatusCode statusCode;
		std::string content_type;
		std::string boundary;
		std::string boundary_end;
		size_t content_length;
		bool transferEncodingExist;
		//---- for post
		std::string	body; 
		std::string	_requestBuffer; 
		std::string buffer;
        FormData formData;
		ChunkedData chunked;
		//--------------------------------
		Request();
		~Request();
		Request(const Request& other);
		Request& operator=(const Request& other);
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
		void 		set_status_code(int code);
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
		// bool		parseHeader(std::string& line);
		void 		reset();
		// bool		checkPath();
		// void		initializeEncode();
		std::string urlDecode(const std::string& str);
		bool checkPath(std::string& path);


		//-----POst
		void handleChunkedData(Request& request);
		void handleBoundary(Request& request);
		void handleChunkedBoundary(Request& request);
};

//chunked
int hexToDecimal(const std::string& str);
void openFile(const std::string& contentType, ChunkedData& chunked);
int  getChunkedSize(std::string& body, ChunkedData& chunked, std::size_t pos);
// void writeData(std::string& body, ChunkedData& chunked, size_t chunkDataSize, int flag);
void writeData(std::string& body, ChunkedData& chunked, size_t chunkDataSize);
void endChunkedData(std::string& body, ChunkedData& chunked);


//boundary
void parseContentDisposition(const std::string& header, FormData& formData);
void getHeaderBody(Request& request);
void writeDataIfNoBoundary(Request& request);
void eraseBoundary(Request& request, size_t nextBoundaryPos);
void checkIfBoundaryEndReached(Request& request);

#endif // REQUEST_HPP