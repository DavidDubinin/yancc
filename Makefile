COMPILER :=g++
FLAGS := -Wall -Wextra
LIBS := -lb15fdrv

main: main.cpp
	$(COMPILER) main.cpp $(LIBS) $(FLAGS) -o main.o

clean:
	rm *.o *.out