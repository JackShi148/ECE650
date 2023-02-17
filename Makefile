all: ringmaster player

ringmaster: ringmaster.cpp actions.cpp potato.h
	g++ -g -ggdb3 -o ringmaster ringmaster.cpp actions.cpp
player: player.cpp actions.h potato.h
	g++ -g -ggdb3 -o player player.cpp actions.cpp
clean:
	rm -rf ringmaster player