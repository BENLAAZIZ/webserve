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

class Request {
private:
    // Parse state
    RequestParseState _parseState;
    size_t _parsePosition;
    
    // Request components
    RequestMethod _method;
    std::string _uri;
    std::string _httpVersion;
    std::map<std::string, std::string> _headers;
    std::string _body;
    
    // For chunked encoding
    size_t _chunkSize;
    bool _lastChunk;
    
    // Internal buffer
    std::string _buffer;
    
    // Parsing helper methods
    bool parseStartLine();
    bool parseHeaders();
    bool parseBody();
    bool parseChunkedBody();
    RequestMethod stringToMethod(const std::string& method);
    
public:
    Request();
    ~Request();
    
    // Buffer management
    void appendData(const char* data, size_t length);
    void clear();
    
    // Incremental parsing
    bool parse();
    
    // State getters
    bool isComplete() const;
    bool hasError() const;
    
    // Request info getters
    RequestMethod getMethod() const;
    const std::string& getUri() const;
    const std::string& getHttpVersion() const;
    const std::map<std::string, std::string>& getHeaders() const;
    std::string getHeader(const std::string& key) const;
    const std::string& getBody() const;
    
    // For debugging
    void dump() const;
};

#endif // REQUEST_HPP