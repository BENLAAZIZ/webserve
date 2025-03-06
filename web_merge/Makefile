# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/06 14:02:19 by hben-laz          #+#    #+#              #
#    Updated: 2025/03/06 14:07:29 by hben-laz         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME = webserv
CPP = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
SRC = main.cpp src/Server.cpp src/Request.cpp src/Config.cpp src/Client.cpp 
OBJ = $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp main.cpp include/Server.hpp include/Request.hpp include/Config.hpp include/Client.hpp
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean