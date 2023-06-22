all: build run

build:
	g++ main.cpp -I../include  -o app_demo

run:
	valgrind --leak-check=full ./app_demo