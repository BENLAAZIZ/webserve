NAME= webserv
CPP= c++
CFLAGS= -Wall -Wextra -Werror -std=c++98
SRC= main.cpp Server.cpp Request.cpp Response.cpp Config.cpp CGI.cpp Client.cpp 
OBJ= $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp Server.hpp HTTPRequest.hpp
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean