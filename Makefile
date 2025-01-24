NAME = ircserv

# Files
SRC_DIR = src
INCLUDE_DIR = inc
OBJ_DIR = obj
CMD_DIR = Server_commands
DEPS_DIR = deps

SRC_FILES =	main.cpp \
			Server.cpp \
			Server_init.cpp \
			Server_loop.cpp \
			Server_commands/chanmode.cpp \
			Server_commands/invite.cpp \
			Server_commands/join.cpp \
			Server_commands/kick.cpp \
			Server_commands/mode.cpp \
			Server_commands/nick.cpp \
			Server_commands/pass.cpp \
			Server_commands/privmsg.cpp \
			Server_commands/quit.cpp \
			Server_commands/topic.cpp \
			Server_commands/user.cpp \
			Server_commands/notice.cpp \
			Server_commands/version.cpp \
			Server_commands/time.cpp \
			Server_commands/ping.cpp \
			Server_commands/pong.cpp \
			Server_commands/cap.cpp \
			Server_commands/part.cpp \
			Server_utils.cpp \
			Client.cpp \
			Channel.cpp \
			irc_ctype.cpp \
			utils.cpp \
			numeric_replies.cpp 

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
	@$(MKDIR) $(OBJ_DIR)/$(CMD_DIR)

$(DEPS_DIR):
	@$(MKDIR) $(DEPS_DIR)
	@$(MKDIR) $(DEPS_DIR)/$(CMD_DIR)

clean:
	@$(RM) $(OBJ)
	@$(RM) $(OBJ_DIR)/$(CMD_DIR)
	@$(RM) $(OBJ_DIR)
	@$(RM) $(DEPS)/$(CMD_DIR)
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
