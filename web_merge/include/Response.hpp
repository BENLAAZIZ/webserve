#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <vector>
#include <fstream>

enum ResponseStatus {
    OK_200 = 200,
    CREATED_201 = 201,
    NO_CONTENT_204 = 204,
    MOVED_PERMANENTLY_301 = 301,
    FOUND_302 = 302,
    BAD_REQUEST_400 = 400,
    UNAUTHORIZED_401 = 401,
    FORBIDDEN_403 = 403,
    NOT_FOUND_404 = 404,
    METHOD_NOT_ALLOWED_405 = 405,
    REQUEST_TIMEOUT_408 = 408,
    LENGTH_REQUIRED_411 = 411,
    PAYLOAD_TOO_LARGE_413 = 413,
    URI_TOO_LONG_414 = 414,
    UNSUPPORTED_MEDIA_TYPE_415 = 415,
    INTERNAL_SERVER_ERROR_500 = 500,
    NOT_IMPLEMENTED_501 = 501,
    BAD_GATEWAY_502 = 502,
    SERVICE_UNAVAILABLE_503 = 503,
    GATEWAY_TIMEOUT_504 = 504,
    HTTP_VERSION_NOT_SUPPORTED_505 = 505
};

enum ResponseState {
    RESPONSE_BUILDING,
    RESPONSE_READY,
    RESPONSE_SENDING,
    RESPONSE_COMPLETE,
    RESPONSE_ERROR
};

class Response {
private:
    // Response status
    ResponseStatus _status;
    std::string _statusMessage;
    
    // Response components
    std::string _httpVersion;
    std::map<std::string, std::string> _headers;
    std::string _body;
    
    // For file serving
    std::ifstream _fileStream;
    std::string _filePath;
    bool _isServingFile;
    size_t _fileSize;
    size_t _bytesSent;
    
    // State tracking
    ResponseState _state;
    std::string _buffer;
    size_t _bufferPosition;
    
    // Helper methods
    std::string getStatusMessage(ResponseStatus status);
    std::string getMimeType(const std::string& path);
    void buildHeaders();
    
public:
    Response();
    ~Response();
    
    // Response building
    void setStatus(ResponseStatus status);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
    bool setFileToServe(const std::string& path);
    
    // Response generation
    void buildResponse();
    
    // Sending helpers
    bool hasMoreData() const;
    const char* getNextDataChunk(size_t& length);
    void advancePosition(size_t sent);
    
    // State management
    ResponseState getState() const;
    void setState(ResponseState state);
    void reset();
    
    // Standard error responses
    void generateErrorPage(ResponseStatus status);
};

#endif // RESPONSE_HPP