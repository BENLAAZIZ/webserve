

NAME = a.out
CPP = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
SRC = main.cpp ASpell.cpp  ATarget.cpp  Warlock.cpp Dummy.cpp  Fwoosh.cpp 
OBJ = $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp ASpell.hpp ATarget.hpp Warlock.hpp Dummy.hpp Fwoosh.hpp
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean