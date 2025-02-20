/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 20:06:18 by hben-laz          #+#    #+#             */
/*   Updated: 2025/02/20 22:44:01 by hben-laz         ###   ########.fr       */
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
		StatusCode sstatusCode;
		std::map<std::string, std::string> headers;
		// const std::map<std::string, std::string>& getHeaders() const;
		int contentLength;
		std::string body; 
		bool bodyFlag;
		bool flag_end_of_headers;

		int statusCode;
		bool headersParsed;


	public:
		HTTPRequest();
		~HTTPRequest();
		HTTPRequest(const HTTPRequest& other);
		HTTPRequest& operator=(const HTTPRequest& other);
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

		std::size_t getContentLength() const;
		void setBody(const std::string& body);
		std::string getBody() const;
		//***************

		//-----------------------------
		void setMethod(const std::string& method);
		void setPath(const std::string& path);
		void setVersion(const std::string& version);

		void setContentLength(int contentLength);

		void setHeader(const std::string& key, const std::string& value);

		bool parseFirstLine(const std::string& line);


		bool getFlagEndOfHeaders() const;
		void setFlagEndOfHeaders(bool flag);
		bool getBodyFlag() const;
		void setBodyFlag(bool flag);
};

	

#endif