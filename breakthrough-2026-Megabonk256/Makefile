CC=g++
CFLAGS=-std=c++11 -Wall -O3

min_max_gsbsclme: bkbb64.h min_max_gsbsclme.cpp
	$(CC) $(CFLAGS) min_max_gsbsclme.cpp -o $@

ubfm_megabonk: bkbb64.h ubfm_megabonk.cpp 
	$(CC) $(CFLAGS) ubfm_megabonk.cpp -o $@

mcts_megabonk: bkbb64.h mcts_megabonk.cpp 
	$(CC) $(CFLAGS) mcts_megabonk.cpp -o $@

clean:
	rm -rf *~ nb_playout_per_sec rand_player min_max_gsbsclme ubfm_megabonk mcts_megabonk
