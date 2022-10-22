CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter $(shell pkg-config gtk+-3.0 --cflags)
LFLAGS = -lm $(shell pkg-config gtk+-3.0 --libs)

NAME = exprinter

SRC = main.c ExprTree.c
DEPS = ExprTree.h

OBJS = main.o ExprTree.o

YOU : $(SRC) $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(NAME)
