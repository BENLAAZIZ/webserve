
#include "../include/web.h"

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
    contentTypes[".cpp"] = "text/x-c";
    contentTypes[".xml"] = "application/xml";
    contentTypes[".pdf"] = "application/pdf";
    contentTypes[".mp4"] = "video/mp4";
    contentTypes[".webm"] = "video/webm";
    contentTypes[".ogg"] = "audio/ogg";
    contentTypes[".mp3"] = "audio/mpeg";
    contentTypes[".wav"] = "audio/wav";
    contentTypes[".svg"] = "image/svg+xml";
    contentTypes[".ttf"] = "font/ttf";
    contentTypes[".woff"] = "font/woff";
    contentTypes[".woff2"] = "font/woff2";
    contentTypes[".eot"] = "application/vnd.ms-fontobject";
    contentTypes[".otf"] = "font/otf";
    contentTypes[".ico"] = "image/x-icon";
    contentTypes[".zip"] = "application/zip";
    contentTypes[".tar"] = "application/x-tar";
    contentTypes[".gz"] = "application/gzip";
    contentTypes[".7z"] = "application/x-7z-compressed";
    contentTypes[".rar"] = "application/x-rar-compressed";
    contentTypes[".exe"] = "application/x-msdownload";
    contentTypes[".bat"] = "application/x-msdos-program";
    contentTypes[".csv"] = "text/csv";
    contentTypes[".yaml"] = "application/x-yaml";
    contentTypes[".md"] = "text/markdown";
    contentTypes[".rtf"] = "application/rtf";
    contentTypes[".ppt"] = "application/vnd.ms-powerpoint";
    contentTypes[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    contentTypes[".doc"] = "application/msword";
    contentTypes[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    contentTypes[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    contentTypes[".odt"] = "application/vnd.oasis.opendocument.text";
    contentTypes[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    contentTypes[".odp"] = "application/vnd.oasis.opendocument.presentation";
    contentTypes[".psd"] = "image/vnd.adobe.photoshop";
    contentTypes[".ai"] = "application/pdf";
    contentTypes[".indd"] = "application/indesign";
    contentTypes[".apk"] = "application/vnd.android.package-archive";
    contentTypes[".dmg"] = "application/x-apple-diskimage";
    contentTypes[".iso"] = "application/x-iso9660-image";
    contentTypes[".bin"] = "application/octet-stream";
    contentTypes[".flac"] = "audio/flac";
    contentTypes[".mkv"] = "video/x-matroska";
    contentTypes[".mov"] = "video/quicktime";
    contentTypes[".avi"] = "video/x-msvideo";
    contentTypes[".torrent"] = "application/x-bittorrent";
    contentTypes[".m4v"] = "video/x-m4v";
    contentTypes[".vtt"] = "text/vtt";

    for (std::map<std::string, std::string>::iterator it = contentTypes.begin(); it != contentTypes.end(); ++it) {
        if (it->second == contentType) {
            return it->first;
        }
    }
    return "";
}

void openFile(const std::string& contentType, ChunkedData& chunked, const std::string& my_root) {
    std::string filename = my_root + "/docs/upload/";
    filename += generateRandomFilename(10) + getExtension(contentType);
    chunked.fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
}

int  getChunkedSize(std::string& body, ChunkedData& chunked, std::size_t pos, bool& endOfRequest) {
    chunked.strHex = body.substr(0, pos);
    chunked.chunkSize = hexToDecimal(chunked.strHex);
    if (chunked.chunkSize == 0) {
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
