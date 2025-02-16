/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 20:06:18 by hben-laz          #+#    #+#             */
/*   Updated: 2025/02/16 18:06:55 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <map>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <exception>

typedef struct StatusCode 
{
	std::size_t code;
	std::string message;
} StatusCode;

class HTTPRequest 
{
	private:
		std::string method;
		std::string path;
		std::string version;
		std::string extension;
		StatusCode statusCode;
		std::map<std::string, std::string> headers;
		// const std::map<std::string, std::string>& getHeaders() const;
		int contentLength;
		std::string body; 
		bool bodyFlag;

	public:
		HTTPRequest();
		~HTTPRequest();
		HTTPRequest(const HTTPRequest& other);
		HTTPRequest& operator=(const HTTPRequest& other);
		bool parseRequest(const std::string& rawRequest) ;
		std::string trim(const std::string& str);
		// Getters
		std::string getMethod() const ;
		std::string getpath() const ;
		std::string getVersion() const ;
		std::string getExtension() const ;
		int getStatusCode() const ;
		std::string getHeader(const std::string& key) const ;
		const std::map<std::string, std::string>& getHeaders() const ;
		std::string getStatusCodeMessage() const;
		void sendErrorResponse(int errorCode);
		//*************
		bool parseRequestLine(const std::string& line);
		bool parseHeader(const std::string& line);
		std::size_t getContentLength() const;
		void setBody(const std::string& body);
		//***************
		void setBodyFlag(bool flag) ;

		bool getBodyFlag() const ;

		//-----------------------------
		void setMethod(const std::string& method);
		void setPath(const std::string& path);
		void setVersion(const std::string& version);

		void setContentLength(int contentLength);
};

	

#endif