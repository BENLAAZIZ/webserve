

#pragma once

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

#include "Server.hpp"
#include "Client.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include "CGI.hpp"

// #include "MimeTypes.hpp"
