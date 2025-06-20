
#include "../include/web.h"

void parseContentDisposition(const std::string& header, FormData& formData) { 
    std::size_t name_pos = header.find("name=\"");
    if (name_pos != std::string::npos) {
        name_pos += 6;
        std::size_t name_end = header.find("\"", name_pos);
        if (name_end != std::string::npos) {
            formData.name = header.substr(name_pos, name_end - name_pos);
        }
    }
            
    std::size_t filename_pos = header.find("filename=\"");
    if (filename_pos != std::string::npos) {
        filename_pos += 10;
        std::size_t filename_end = header.find("\"", filename_pos);
        if (filename_end != std::string::npos) {
            formData.filename = header.substr(filename_pos, filename_end - filename_pos);
        }
    }
        
    size_t content_type_pos = header.find("Content-Type:");
    if (content_type_pos != std::string::npos) {
        formData.contentType = header.substr(content_type_pos + 14);
    }
}

void getHeaderBody(Request& request) {
	std::size_t headersEndPos = request.buffer.find("\r\n\r\n", request.formData.pos);
	if (headersEndPos == std::string::npos)
		return;

	std::size_t start = request.boundary.length() + request.formData.pos + 2;
	request.formData.header = request.buffer.substr(start, headersEndPos - start);
	parseContentDisposition(request.formData.header, request.formData);

	// std::string filename = "/Users/aben-cha/goinfre/uploads/" + request.formData.filename;
    std::string filename = request.my_root + "/docs/upload/" + request.formData.filename;

	request.formData.fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (request.formData.fd == -1) {
        request.formData.flag = 0;
        return;
    }
	
	request.buffer.erase(0, headersEndPos + 4);
	request.formData.flag = 1;
}

void writeDataIfNoBoundary(Request& request) {
	std::size_t safeWriteLen = request.buffer.size() - request.boundary.size();
    write(request.formData.fd, request.buffer.c_str(), safeWriteLen);
    request.buffer.erase(0, safeWriteLen);
}

void eraseBoundary(Request& request, size_t nextBoundaryPos) {
    std::size_t fileDataEnd = (nextBoundaryPos >= 2) ? nextBoundaryPos - 2 : nextBoundaryPos;
    write(request.formData.fd, request.buffer.c_str(), fileDataEnd);
    request.buffer.erase(0, nextBoundaryPos);
}

int writeDataBoundaryFound(Request& request, std::size_t pos) {
    std::size_t fileDataEnd = (pos >= 2) ? pos - 2 : pos;
    request.formData.data = request.buffer.substr(0, fileDataEnd);

    write(request.formData.fd, request.formData.data.c_str(), request.formData.data.size());

    request.buffer.erase(0, fileDataEnd);
    if (request.buffer.find(request.boundary_end) != std::string::npos) {
        request.endOfRequest = true;

    	
		request.buffer.erase(0, request.boundary_end.size());
    	request.formData.flag = 0;
    	close(request.formData.fd);
        
		return 1;
    }
    request.formData.flag = 0;
    close(request.formData.fd);
    return 0;
}

void checkIfBoundaryEndReached(Request& request) {
	if (request.buffer.find(request.boundary_end) != std::string::npos && 
		request.buffer.substr(0, request.boundary_end.size()) == request.boundary_end) {
        request.endOfRequest = true;

		close(request.formData.fd);
		request.formData.fd = -1;
		request.formData.flag = 0;
		request.buffer.erase(0, request.boundary_end.size());
		return ;
	} else {
		close(request.formData.fd);
		request.formData.fd = -1; 
		request.formData.flag = 0;
	}
}
