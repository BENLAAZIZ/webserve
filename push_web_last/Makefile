# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/06 14:02:19 by hben-laz          #+#    #+#              #
#    Updated: 2025/04/09 00:23:44 by hben-laz         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME = webserv
CPP = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
SRC = main.cpp src/Server.cpp src/Request.cpp src/Response.cpp src/Config.cpp src/Client.cpp  post/chunked.cpp post/boundary.cpp
OBJ = $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp main.cpp include/web.h
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean