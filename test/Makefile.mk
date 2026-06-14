
INC=%%%%

INCLIB=$(INC)/../lib

UNAME := $(shell uname)

CFLAGS= -I$(INC) -O3 -I.. -g

NAME= mlx-test
NAME_NEW= mlx-test-new
NAME_MOUSE= mlx-test-mouse
NAME_COMP= mlx-test-components
NAME_WIN= mlx-test-new-win
SRC = main.c
SRC_NEW = main_test_new.c
SRC_MOUSE = main_test_mouse.c
SRC_COMP = main_test_components.c
SRC_WIN = new_win.c
OBJ = $(SRC:%.c=%.o)
OBJ_NEW = $(SRC_NEW:%.c=%.o)
OBJ_MOUSE = $(SRC_MOUSE:%.c=%.o)
OBJ_COMP = $(SRC_COMP:%.c=%.o)
OBJ_WIN = $(SRC_WIN:%.c=%.o)

LFLAGS = -L.. -lmlx -L$(INCLIB) -lXext -lX11 -lm

ifeq ($(UNAME), Darwin)
	# mac
	CC = clang
else ifeq ($(UNAME), FreeBSD)
	# FreeBSD
	CC = clang
else
	#Linux and others...
	CC	= gcc
	LFLAGS += -lbsd
endif

all: $(NAME) $(NAME_NEW) $(NAME_MOUSE) $(NAME_COMP) $(NAME_WIN)

$(NAME): $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LFLAGS)

$(NAME_NEW): $(OBJ_NEW)
	$(CC) -o $(NAME_NEW) $(OBJ_NEW) $(LFLAGS)

$(NAME_MOUSE): $(OBJ_MOUSE)
	$(CC) -o $(NAME_MOUSE) $(OBJ_MOUSE) $(LFLAGS)

$(NAME_COMP): $(OBJ_COMP)
	$(CC) -o $(NAME_COMP) $(OBJ_COMP) $(LFLAGS)

$(NAME_WIN): $(OBJ_WIN)
	$(CC) -o $(NAME_WIN) $(OBJ_WIN) $(LFLAGS)

show:
	@printf "UNAME		: $(UNAME)\n"
	@printf "NAME  		: $(NAME)\n"
	@printf "NAME_NEW  	: $(NAME_NEW)\n"
	@printf "NAME_MOUSE  : $(NAME_MOUSE)\n"
	@printf "NAME_COMP   : $(NAME_COMP)\n"
	@printf "CC		: $(CC)\n"
	@printf "CFLAGS		: $(CFLAGS)\n"
	@printf "LFLAGS		: $(LFLAGS)\n"
	@printf "SRC		:\n	$(SRC)\n"
	@printf "SRC_NEW	:\n	$(SRC_NEW)\n"
	@printf "SRC_MOUSE	:\n	$(SRC_MOUSE)\n"
	@printf "SRC_COMP	:\n	$(SRC_COMP)\n"
	@printf "OBJ		:\n	$(OBJ)\n"
	@printf "OBJ_NEW	:\n	$(OBJ_NEW)\n"
	@printf "OBJ_MOUSE	:\n	$(OBJ_MOUSE)\n"
	@printf "OBJ_COMP	:\n	$(OBJ_COMP)\n"

clean:
	rm -f $(NAME) $(NAME_NEW) $(NAME_MOUSE) $(NAME_COMP) $(NAME_WIN) $(OBJ) $(OBJ_NEW) $(OBJ_MOUSE) $(OBJ_COMP) $(OBJ_WIN) *~ core *.core

re: clean all
