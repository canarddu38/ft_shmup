NAME		= ft_shmup
CPP_FILES	= boxes.cpp \
				laser.cpp \
				main.cpp \
				maths.cpp \
				menu.cpp \
				spaceship.cpp
CPP_OBJ		= $(CPP_FILES:.cpp=.o)
CPP			= c++
CPP_FLAGS	= -Wall -Wextra -Werror
INCLUDE		= -lncurses

all: $(NAME)

$(NAME): $(CPP_OBJ)
	$(CPP) $(CPP_FLAGS) $(INCLUDE) $^ -o $@

%.o: %.cpp
	$(CPP) $(CPP_FLAGS) -c $^ -o $@

clean: 
	$(RM) -rf $(CPP_OBJ)

fclean: clean
	$(RM) -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re