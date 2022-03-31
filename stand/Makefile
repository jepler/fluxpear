main: main.cc ../libwoz.cc ../libwoz.h | Makefile
	g++ -fsanitize=undefined -Wall -g -Og -o $@ -I.. $(filter %.cc, $^)
