CC=g++
CFLAGS=-std=c++11 -Wall -O3

nb_playout_per_sec: bkbb64.h nb_playout_per_sec.cpp
	$(CC) $(CFLAGS) nb_playout_per_sec.cpp -o $@

rand_player: bkbb64.h rand_player.cpp
	$(CC) $(CFLAGS) rand_player.cpp -o $@

min_max_gsbsclme: bkbb64.h min_max_gsbsclme.cpp
	$(CC) $(CFLAGS) min_max_gsbsclme.cpp -o $@

clean:
	rm -rf *~ nb_playout_per_sec rand_player
