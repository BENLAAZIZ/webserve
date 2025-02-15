NAME= webserv
CPP= c++
CFLAGS= -Wall -Wextra -Werror -std=c++98
SRC= webserv.cpp server.cpp HTTPRequest.cpp
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