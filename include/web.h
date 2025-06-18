

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
#include <cstring>      // for memset, strerror
#include <netdb.h>      // for getaddrinfo, addrinfo
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <exception>
#include <dirent.h> // for directory handling

#include <sys/time.h>
#include <cstdlib> // strtol
#include <ctime>
#include <csignal>


#include <sys/stat.h>
#include <fstream>
#include <sys/wait.h>
#include <algorithm>

#include "ConfigParser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "CgiHandler.hpp"