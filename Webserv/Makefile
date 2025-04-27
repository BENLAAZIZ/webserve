NAME = webserv
CPP = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
SRC = main.cpp src/Server.cpp src/Request.cpp  src/Response.cpp src/Config.cpp \
	src/Client.cpp post/chunked.cpp post/boundary.cpp pars/ConfigParser.cpp \
	pars/parss.cpp pars/CgiHandler.cpp
OBJ = $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp include/web.h include/CgiHandler.hpp include/Client.hpp include/Config.hpp include/ConfigParser.hpp include/Request.hpp include/Response.hpp include/Server.hpp
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean