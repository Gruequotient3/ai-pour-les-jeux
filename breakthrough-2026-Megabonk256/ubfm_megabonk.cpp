#include <cstdlib>   
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <chrono>
#include <unordered_map>
#include <climits>
#include <vector>
#include "bkbb64.h"

#define WHITE 0
#define BLACK 1


struct State {
    Board64_t b;
    bool is_white;

    bool operator==(const State& o) const {
        return b.white == o.b.white && b.black == o.b.black && is_white == o.is_white;
    }
};

struct StateHash {
    std::size_t operator()(const State& s) const {
        return std::hash<uint64_t>()(s.b.white) ^ (std::hash<uint64_t>()(s.b.black) << 1) ^ (std::hash<bool>()(s.is_white) << 2);
    }
};

std::unordered_map<State, int, StateHash> H;
std::unordered_map<State, State, StateHash> parent_map;
State ROOT_STATE;

State selection(State s) {
    std::vector<Move64_t> M = s.is_white ? 
        Lfr_t(s.b.white_left(), s.b.white_forward(), s.b.white_right()).get_white_moves() : 
        Lfr_t(s.b.black_left(), s.b.black_forward(), s.b.black_right()).get_black_moves();
   
    if (s.b.win(s.is_white)) return s;
    if (M.empty()) return s;

    int min_val = INT_MAX;
    int max_val = -INT_MAX;
    State best_state = s;
    bool found = false;

    for (Move64_t m : M) {
        State s_prime = s;
        s_prime.b.apply_move(m, s.is_white);
        s_prime.is_white = !s.is_white;

        if (H.find(s_prime) == H.end()) {
            return s; 
        }

        if (s.is_white) { 
            if (max_val < H[s_prime]) {
                max_val = H[s_prime];
                best_state = s_prime;
                found = true;
            }
        } else {
            if (min_val > H[s_prime]) {
                min_val = H[s_prime];
                best_state = s_prime;
                found = true;
            }
        }
    }
    
    if (!found) return s;
    return selection(best_state);
}

void expansion(State s) {
    std::vector<Move64_t> M = s.is_white ? 
        Lfr_t(s.b.white_left(), s.b.white_forward(), s.b.white_right()).get_white_moves() : 
        Lfr_t(s.b.black_left(), s.b.black_forward(), s.b.black_right()).get_black_moves();

    for (Move64_t m : M) {
        State s_prime = s;
        s_prime.b.apply_move(m, s.is_white);
        s_prime.is_white = !s.is_white;
        
        parent_map[s_prime] = s; 
        
        if (H.find(s_prime) == H.end()) {
            H[s_prime] = s_prime.b.eval(true); 
        }
    }
}

void backpropagate(State s) {
    std::vector<Move64_t> M = s.is_white ? 
        Lfr_t(s.b.white_left(), s.b.white_forward(), s.b.white_right()).get_white_moves() : 
        Lfr_t(s.b.black_left(), s.b.black_forward(), s.b.black_right()).get_black_moves();

    int min_val = INT_MAX;
    int max_val = -INT_MAX;

    if (s.is_white) {
        for (Move64_t m : M) {
            State s_prime = s;
            s_prime.b.apply_move(m, s.is_white);
            s_prime.is_white = !s.is_white;

            if (H.find(s_prime) != H.end() && max_val < H[s_prime]) {
                max_val = H[s_prime];
            }
        }
        if (max_val != -INT_MAX) H[s] = max_val;
    } else {
        for (Move64_t m : M) {
            State s_prime = s;
            s_prime.b.apply_move(m, s.is_white);
            s_prime.is_white = !s.is_white;

            if (H.find(s_prime) != H.end() && min_val > H[s_prime]) {
                min_val = H[s_prime];
            }
        }
        if (min_val != INT_MAX) H[s] = min_val;
    }

    if (s == ROOT_STATE) return;

    if (parent_map.find(s) != parent_map.end()) {
        State p = parent_map[s];
        backpropagate(p);
    }
}


Move64_t get_ubfm_move(Board64_t start_board, bool is_white) {
    H.clear();
    parent_map.clear();
    
    ROOT_STATE.b = start_board;
    ROOT_STATE.is_white = is_white;
    H[ROOT_STATE] = 0;

    int iter = 50000;
    while (iter > 0) { 
        State s_prime = selection(ROOT_STATE);
        expansion(s_prime);
        backpropagate(s_prime);
        iter--;
    }
    
    std::vector<Move64_t> M = is_white ? 
        Lfr_t(start_board.white_left(), start_board.white_forward(), start_board.white_right()).get_white_moves() : 
        Lfr_t(start_board.black_left(), start_board.black_forward(), start_board.black_right()).get_black_moves();
    
    Move64_t best_move = M[0];
    int best_val = is_white ? -INT_MAX : INT_MAX;

    for (Move64_t m : M) {
        State s_prime = ROOT_STATE;
        s_prime.b.apply_move(m, is_white);
        s_prime.is_white = !is_white;
        
        if (H.find(s_prime) != H.end()) {
            if (is_white && H[s_prime] > best_val) { 
                best_val = H[s_prime]; 
                best_move = m; 
            }
            if (!is_white && H[s_prime] < best_val) { 
                best_val = H[s_prime]; 
                best_move = m; 
            }
        }
    }
    return best_move;
}


std::string genmove(Board64_t& _board, int _color) {
    Move64_t m;
  
    if(_color == WHITE) {
        m = get_ubfm_move(_board, true);
    } else if (_color == BLACK) {
        m = get_ubfm_move(_board, false);
    } else {
        return std::string("resign");
    }
    
    std::string stri = pos_to_coord(m.pi);
    std::string strf = pos_to_coord(m.pf);
    return stri+std::string("-")+strf;
}

// $>./a.out @@@@@@@@@@@@@@@@................................OOOOOOOOOOOOOOOO O
int main(int _ac, char** _av) {
    if(_ac != 3) {
        fprintf(stderr, "usage: %s BOARD PLAYER\n", _av[0]);
        return 0;
    }
    Board64_t B(_av[1]);
    bool debug = true;
    if(debug) {
        B.print_board(stderr);
    }
    if(std::string(_av[2]).compare("O")==0) {
        printf("%s\n", genmove(B, WHITE).c_str());
    } else if(std::string(_av[2]).compare("@")==0) {
        printf("%s\n", genmove(B, BLACK).c_str());
    }
    return 0;
}
