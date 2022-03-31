main: main.cc ../src/libwoz.cc ../src/libwoz.h | Makefile
	g++ -fsanitize=undefined -Wall -g -Og -o $@ -I../src $(filter %.cc, $^)
