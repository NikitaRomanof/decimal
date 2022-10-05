OS:= $(shell uname -s)
ifeq ($(OS), Darwin)
LIBS=-lcheck
else
LIBS=-lcheck -lpthread -lrt -lsubunit -lm
endif
CC=gcc
WFLAGS=#-Wall -Wextra -Werror
FLAGS=--coverage
DEBUG=-g
LFLAGS=-c

S2=s21_decimal
S3=report
S4=test_s21_decimal
EXE4=$(S4).out

.PHONY: all clean test $(S2).a gcov_report rebuild cpplint leaks cpp 

all: $(S2).a

$(S2).a: $(S2).o
	ar r $@ $^ 
	cp $@ lib$@

$(S2).o:	
	$(CC) $(DEBUG) $(LFLAGS) $(S2).c -o $@
	
test: clean $(S4)
	./$(EXE4)

ifeq ($(OS), Darwin)
$(S4): $(S2).a $(S4).o 
	$(CC) $(DEBUG) $^ -o $(EXE4) $(LIBS)
else
$(S4): $(S2).a
	$(CC) $(S4).c $(S2).a -o $(EXE4) $(LIBS)
endif

$(S4).o:
	$(CC) $(DEBUG) $(LFLAGS) $(S4).c -o $@

test_coverage: clean $(S4)_coverage
	./$(EXE4)

$(S4)_coverage: $(S2)_coverage.a $(S4)_coverage.o 
	$(CC) $(DEBUG) $(FLAGS) $^ -o $(EXE4) $(LIBS)
$(S4)_coverage.o:
	$(CC) $(DEBUG) $(LFLAGS) $(FLAGS) $(S4).c -o $@
$(S2)_coverage.a:
	$(CC) $(DEBUG) $(LFLAGS) $(FLAGS) $(S2).c -o $@

gcov_report: test_coverage
	lcov -t "$(EXE4)" -o $(S3).info -c -d .
	genhtml -o $(S3) $(S3).info	
	open -a "Google Chrome" $(S3)/index.html

clean:
	rm -f *.a *.gcda *.gcno *.o *.info *.out ../.DS_Store
	rm -f -r report* *.dSYM

rebuild: clean all

cpplint:
	cp ../materials/linters/CPPLINT.cfg .
	python3 ../materials/linters/cpplint.py --extensions=c *.c
	python3 ../materials/linters/cpplint.py --extensions=h *.h
	rm -f CPPLINT.cfg

leaks: clean $(S4)
ifeq ($(OS), Darwin)
	CK_FORK=no leaks -atExit -- ./$(EXE4)	
else
	CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all ./$(EXE4)
endif

cpp:
	cppcheck --enable=all --suppress=missingIncludeSystem *.c *.h
