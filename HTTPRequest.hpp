/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 20:06:18 by hben-laz          #+#    #+#             */
/*   Updated: 2025/02/22 18:03:55 by hben-laz         ###   ########.fr       */
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
	std::size_t	code;
	std::string	message;
} StatusCode;

class HTTPRequest 
{
	private:
		std::string	method;
		std::string	path;
		std::string	version;
		std::string	extension;
		StatusCode statusCode;
		std::map<std::string, std::string> headers;
		std::string	body; 
		std::string content_type;
		std::string boundary;
		size_t content_length;
		bool transferEncodingExist;
		
		bool bodyFlag;
		bool flag_end_of_headers;
		// int statusCode;
		bool headersParsed;
	public:
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
		std::size_t	getContentLength() const;
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
		// Methods
		void		sendErrorResponse(int errorCode);
		bool		parseFirstLine(const std::string& line);
		bool		parseHeader(std::string& line);
};

	

#endif