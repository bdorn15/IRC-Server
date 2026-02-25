# ================================ SETTINGS ================================== #
NAME = ircserv

# compile and link
CPP = c++
CPPFLAGS = -Wall -Wextra -Werror -fpermissive -std=c++20 -g
DEPFLAGS = -MMD -MP

# Dir structure
OBJ_DIR = obj/
SRC_DIR = src/

# ================================ SOURCES =================================== #
# Wildcards not permitted by norm, but nice and easy for testing
HEADERS	= $(wildcard inc/*.hpp)
SRC		= $(wildcard src/*.cpp)
OBJ		= $(SRC:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)
DEP		= $(OBJ:.o=.d)

# ================================ RULES ===================================== #
# $@ = target.
# $< = first prerequisite. $^ = all prerequisites $? = prereq newer than target
all : $(NAME)

$(NAME) : $(OBJ) #$(HEADERS)
	$(CPP) $(CPPFLAGS) $(OBJ) -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp #$(HEADERS)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@ -I./inc

-include $(DEP)

clean :
	rm -rf $(OBJ_DIR)

fclean : clean
	rm -f $(NAME)

re: fclean all

# .PHONY tells make that the commands are not files.
# Make doesn't worry about whether they are actual files or not. It will always
#     run these unless other targets are present and up-to-date.
.PHONY: all clean fclean re
