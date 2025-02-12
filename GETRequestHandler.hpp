
#ifndef GETREQUESTHANDLER_HPP
#define GETREQUESTHANDLER_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>

#include "requestPars.hpp"

class GETRequestHandler {
	public:
		std::string handleRequest(const std::string& rawRequest);
		std::string generateErrorResponse(int statusCode, const std::string& message);
		std::string generateSuccessResponse(const std::string& content);
};

#endif