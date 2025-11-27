CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c11
LDLIBS = -lncurses 

OBJDIR = obj
SHELL = /bin/bash

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

DISTNAME = $(NAME)-$(DISTVERSION)
DISTVERSION = 1.0.0
DISTFILES = $(SRCS) Doxyfile FSM.jpg Makefile

OPENOS = vi 
ifeq ($(shell uname -s), Linux) 
	OPENOS = xdg-open 
endif 
ifeq ($(shell uname -s), Darwin) 
	OPENOS = open 
endif 

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin) 
    CCFLAGS = $(shell pkg-config --cflags check) 
    LDLIBS = $(shell pkg-config --libs check) -lm -lncurses    
else
	CCFLAGS = -I/usr/include
    LDLIBS = -lcheck -lm -lsubunit -lncurses 
endif

SRCS = $(shell find . -name "*.c" -not -path "./TESTS/*" -not -path "./$(OBJDIR)/*" | sed 's|^\./||')
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(filter-out tests/asd.c,$(SRCS)))

TEST_SRCS := $(wildcard brick_game/tetris/*.c)
TEST_OBJS := $(patsubst %.c, %.o, $(TEST_SRCS))

NAME = tetris

all: $(NAME) dist dvi gcov_report 

$(NAME): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

install: $(NAME)
	mkdir -p $(BINDIR)
	cp $(NAME) $(BINDIR)/
	@echo "Installed $(NAME) to $(BINDIR)"

uninstall:
	rm -f $(BINDIR)/$(NAME)
	@echo "Removed $(NAME) from $(BINDIR)"

dist:
	tar czvf $(DISTNAME).tar.gz $(DISTFILES)

doxygen:
	doxygen Doxyfile

dvi: doxygen 
	@$(OPENOS) doc/html/files.html

test: tests/asd.o s21_brick_game.check
	$(CC) -o test tests/asd.o s21_brick_game.check $(LDLIBS)
	./test

s21_brick_game.check: $(TEST_OBJS)
	$(CC)  $(CCFLAGS) -c $(CFLAGS) $(TEST_SRCS) 
	ar rc s21_brick_game.check $(TEST_OBJS) 
	ranlib s21_brick_game.check

tests/asd.o: tests/asd.c
	$(CC) $(CFLAGS) $(CCFLAGS) -c tests/asd.c -o tests/asd.o

tests/asd.c:
	checkmk clean_mode=1 tests/asd.check > tests/asd.c

gcov_report: tests/asd.c $(TEST_SRCS)
	$(CC) --coverage $(TEST_SRCS) tests/asd.c -o s21_test $(CCFLAGS) $(LDLIBS)
	./s21_test
	gcov -l -p -c *.gcno
	-rm -rf s21_test-asd*
	gcovr --html --html-details -o gcov_report.html
	open gcov_report.html 


debug: $(filter-out $(OBJDIR)/gui/cli/main.o, $(OBJS)) $(OBJDIR)/gui/cli/main.o
	$(CC) $(LDFLAGS) -g $^ -o debug $(LDLIBS)

rebuild: clean all

clang:
	cp ../materials/linters/.clang-format .clang-format
	clang-format -n $(shell find . -name "*.c" -o -name "*.h" | grep -v $(OBJDIR) | grep -v TESTS)
	-rm -rf .clang-format

valgrind_test:
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME)

cppcheck:
	cppcheck --enable=all --suppress=missingInclude --inconclusive $(CCFLAGS) /usr/local/include .

clean:
	-rm -rf $(OBJDIR) $(NAME) debug doc highscore.txt s21* s21*.* test  gcov*.* obj *.o *.gz *.check tests/asd.o tests/asd.c

.PHONY: clean clang all rebuild debug doxygen latex dvi
