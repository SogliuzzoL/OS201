SRCS := $(wildcard *.c) $(wildcard *.s)
DEPS := Makefile $(SRCS) $(wildcard *.h)
OPTS := -g -Wall

# this exercise requires -static
OPTS += -static

all: test

tp: $(DEPS)
	gcc $(OPTS) $(SRCS) -o $@

test: tp
	./tp
