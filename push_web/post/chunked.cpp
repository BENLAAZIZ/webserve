/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/14 03:59:43 by hben-laz          #+#    #+#             */
/*   Updated: 2025/04/16 17:47:03 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "../include/Request.hpp"
#include "../include/web.h"

#include <cstdlib> // strtol
#include <ctime>
#include <cstdlib>

int hexToDecimal(const std::string& str) {
    return std::strtol(str.c_str(), NULL, 16);
}

static std::string generateRandomFilename(size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const size_t charsetSize = sizeof(charset) - 1;
    std::string result;

    std::srand(std::time(0));

    for (size_t i = 0; i < length; ++i) {
        result += charset[std::rand() % charsetSize];
    }
    return result;
}


static std::string getExtension(const std::string& contentType) {
    std::map<std::string, std::string> contentTypes;
    contentTypes[".html"] = "text/html";
    contentTypes[".css"] = "text/css";
    contentTypes[".js"] = "application/javascript";
    contentTypes[".png"] = "image/png";
    contentTypes[".jpg"] = "image/jpeg";
    contentTypes[".gif"] = "image/gif";
    contentTypes[".txt"] = "text/plain";
    contentTypes[".mp4"] = "video/mp4";

    for (std::map<std::string, std::string>::iterator it = contentTypes.begin(); it != contentTypes.end(); ++it) {
        if (it->second == contentType) {
            return it->first;
        }
    }
    return "";
}

void openFile(const std::string& contentType, ChunkedData& chunked) {
    // std::string contentType = headers.at("Content-Type");
    std::string filename = "/Users/hben-laz/goinfre/uploads/";
    filename += generateRandomFilename(10) + getExtension(contentType);
    chunked.fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (chunked.fd == -1) {
        std::cerr << "Error opening file: " << std::strerror(errno) << std::endl;
        exit(1);
    }
    std::cout << "filename: " << filename << std::endl;
}

int  getChunkedSize(std::string& body, ChunkedData& chunked, std::size_t pos, bool& endOfRequest) {
    chunked.strHex = body.substr(0, pos);
    chunked.chunkSize = hexToDecimal(chunked.strHex);
    if (chunked.chunkSize == 0) {
        std::cout << "ENDDDDDD\n\n";
        chunked.isFinished = true;
        endOfRequest = true;
        close(chunked.fd);
        return 1;
    }
    body.erase(0, pos + 2);
    chunked.flag = 1;
    return 0;
}


void writeData(std::string& body, ChunkedData& chunked, size_t chunkDataSize) {
    write(chunked.fd, body.c_str(), chunkDataSize);
    chunked.bytesRead += chunkDataSize;
    body.erase(0, chunkDataSize);
}


void endChunkedData(std::string& body, ChunkedData& chunked) {
    if (body.size() >= 2)
        body.erase(0, 2);
    chunked.flag = 0;
    chunked.bytesRead = 0;
}
