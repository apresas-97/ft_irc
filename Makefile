NAME = ircserv

# Files
SRC_DIR = src
INCLUDE_DIR = inc
OBJ_DIR = obj
DEPS_DIR = deps

SRC_FILES =	main.cpp \
			Server.cpp \
			Server_init.cpp \
			Server_loop.cpp \
			Client.cpp \
			Channel.cpp \
			commands.cpp \
			irc_ctype.cpp \
			numeric_replies.cpp \
			server_commands/invite.cpp \
			server_commands/join.cpp \
			server_commands/kick.cpp \
			server_commands/mode.cpp \
			server_commands/nick.cpp \
			server_commands/pass.cpp \
			server_commands/priv.cpp \
			server_commands/quit.cpp \
			server_commands/topic.cpp \
			server_commands/user.cpp

SRC = $(addprefix $(SRC_DIR)/,$(SRC_FILES))

OBJ = $(addprefix $(OBJ_DIR)/,$(SRC_FILES:%.cpp=%.o))
DEPS = $(addprefix $(DEPS_DIR)/,$(SRC_FILES:%.cpp=%.d))

# Libraries and Headers
LIBS =
HDRS =

# Compiler + flags
CC = c++
WFLAGS = -Wall -Wextra -Werror -pedantic
CPPSTD = -std=c++98
CFLAGS = $(WFLAGS) $(CPPSTD) -g
DFLAGS = -MD -MF
INCLUDE = -I $(INCLUDE_DIR)

# Commands and utils
RM = rm -rf
MKDIR = mkdir -p
AR = ar rcs
MUTE = &> /dev/null
MK = Makefile

PORT ?= ""
PASSWORD ?= ""

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(MK) | $(OBJ_DIR) $(DEPS_DIR)
	$(CC) $(CFLAGS) $(DFLAGS) $(DEPS_DIR)/$*.d -c $< -o $@ $(INCLUDE)

$(OBJ_DIR):
	@$(MKDIR) $(OBJ_DIR)

$(DEPS_DIR):
	@$(MKDIR) $(DEPS_DIR)

clean:
	@$(RM) $(OBJ)
	@$(RM) $(OBJ_DIR)
	@$(RM) $(DEPS)
	@$(RM) $(DEPS_DIR)

fclean: clean
	@$(RM) $(NAME)

re: fclean all

run: all
	./$(NAME) $(PORT) $(PASSWORD)

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) $(PORT) $(PASSWORD))

.PHONY: all clean fclean re run valgrind
-include $(DEPS)
