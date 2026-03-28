#include <cstdlib>   
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <chrono>
#include <unordered_map>
#include <climits>
#include <vector>
#include <cmath>
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
        return std::hash<uint64_t>()(s.b.white) ^ 
              (std::hash<uint64_t>()(s.b.black) << 1) ^ 
              (std::hash<bool>()(s.is_white) << 2);
    }
};

struct NodeData {
    double w;
    int n;
};

std::unordered_map<State, NodeData, StateHash> H;
State ROOT_STATE;
uint32_t global_seed = 123456789;

Move64_t get_mcts_move(Board64_t start_board, bool is_white) {
    H.clear();
    
    ROOT_STATE.b = start_board;
    ROOT_STATE.is_white = is_white;
    H[ROOT_STATE] = {0.0, 0};

    auto start_time = std::chrono::steady_clock::now();
    int iter = 0;

    while (true) { 
        // Time management: maximize the 1 second limit (950ms to leave a buffer)
        if ((iter & 255) == 0) { 
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
            if (elapsed > 950) break; 
        }
        iter++;

        // 1. Selection & Expansion
        std::vector<State> path;
        State current = ROOT_STATE;
        path.push_back(current);
        
        while (true) {
            if (current.b.white_win() || current.b.black_win()) break;
            
            std::vector<Move64_t> M = current.is_white ? 
                Lfr_t(current.b.white_left(), current.b.white_forward(), current.b.white_right()).get_white_moves() : 
                Lfr_t(current.b.black_left(), current.b.black_forward(), current.b.black_right()).get_black_moves();
            
            if (M.empty()) break;
            
            bool unexpanded = false;
            State unvisited;
            
            // Look for unexpanded children
            for (Move64_t m : M) {
                State s_prime = current;
                s_prime.b.apply_move(m, current.is_white);
                s_prime.is_white = !current.is_white;
                
                if (H.find(s_prime) == H.end()) {
                    unvisited = s_prime;
                    unexpanded = true;
                    break;
                }
            }
            
            if (unexpanded) {
                current = unvisited;
                H[current] = {0.0, 0};
                path.push_back(current);
                break;
            }
            
            // All children expanded, select via UCT
            double max_uct = -1.0;
            State best_child = current;
            double log_N = std::log(H[current].n);
            
            for (Move64_t m : M) {
                State s_prime = current;
                s_prime.b.apply_move(m, current.is_white);
                s_prime.is_white = !current.is_white;
                
                double w = H[s_prime].w;
                double n = H[s_prime].n;
                
                // Maximize White wins if White's turn, maximize Black wins if Black's turn
                double exploit = current.is_white ? (w / n) : ((n - w) / n);
                double explore = 0.4 * std::sqrt(log_N / n);
                double uct = exploit + explore;
                
                if (uct > max_uct) {
                    max_uct = uct;
                    best_child = s_prime;
                }
            }
            
            current = best_child;
            path.push_back(current);
        }
        
        // 2. Simulation (Playout)
        double r = 0.0;
        if (current.b.white_win()) {
            r = 1.0;
        } else if (current.b.black_win()) {
            r = 0.0;
        } else {
            Board64_t sim_b = current.b;
            global_seed = rand_xorshift(global_seed);
            sim_b.seed = global_seed; // Break determinism across identical node visits
            sim_b.seq_playout(current.is_white);
            r = sim_b.white_win() ? 1.0 : 0.0;
        }
        
        // 3. Backpropagation
        for (State& p : path) {
            H[p].n += 1;
            H[p].w += r;
        }
    }
    
    // Choose the most robust child (highest visit count)
    std::vector<Move64_t> M = is_white ? 
        Lfr_t(start_board.white_left(), start_board.white_forward(), start_board.white_right()).get_white_moves() : 
        Lfr_t(start_board.black_left(), start_board.black_forward(), start_board.black_right()).get_black_moves();
    
    Move64_t best_move = M[0];
    int max_visits = -1;

    for (Move64_t m : M) {
        State s_prime = ROOT_STATE;
        s_prime.b.apply_move(m, is_white);
        s_prime.is_white = !is_white;
        
        if (H.find(s_prime) != H.end()) {
            if (H[s_prime].n > max_visits) { 
                max_visits = H[s_prime].n; 
                best_move = m; 
            }
        }
    }
    return best_move;
}

std::string genmove(Board64_t& _board, int _color) {
    Move64_t m;
  
    if(_color == WHITE) {
        m = get_mcts_move(_board, true);
    } else if (_color == BLACK) {
        m = get_mcts_move(_board, false);
    } else {
        return std::string("resign");
    }
    
    std::string stri = pos_to_coord(m.pi);
    std::string strf = pos_to_coord(m.pf);
    return stri+std::string("-")+strf;
}

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
