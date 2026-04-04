#include <cstdlib>   
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <chrono>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include "bkbb64.h"

#define WHITE 0
#define BLACK 1

// True random starting seed so the bot plays differently every match
uint32_t global_seed = std::random_device{}();

// Fast global arrays for Playout Policy Adaptation (PPA)
int P_white[4096] = {0};
int P_black[4096] = {0};

struct Node {
    Board64_t b;
    bool is_white; 
    double w;      
    int n;         
    int parent;
    Move64_t move_from_parent;
    
    std::vector<Move64_t> unexpanded_moves;
    std::vector<int> children;
    bool is_terminal;
    
    Node(Board64_t _b, bool _is_white, int _parent, Move64_t _move) 
        : b(_b), is_white(_is_white), w(0.0), n(0), parent(_parent), move_from_parent(_move) {
        
        if (b.white_win() || b.black_win()) {
            is_terminal = true;
        } else {
            unexpanded_moves = is_white ? 
                Lfr_t(b.white_left(), b.white_forward(), b.white_right()).get_white_moves() : 
                Lfr_t(b.black_left(), b.black_forward(), b.black_right()).get_black_moves();
            
            is_terminal = unexpanded_moves.empty();
        }
    }
};

inline int encode_move(Move64_t m) {
    return (__builtin_ctzll(m.pi) << 6) | __builtin_ctzll(m.pf);
}

// Playout with 1-Ply Win Detection AND epsilon-greedy PPA
void ppa_seq_playout(Board64_t& b, bool is_white, std::vector<Move64_t>& seq) {
    while (true) {
        if (b.white_win() || b.black_win()) break;
        
        std::vector<Move64_t> M = is_white ? 
            Lfr_t(b.white_left(), b.white_forward(), b.white_right()).get_white_moves() : 
            Lfr_t(b.black_left(), b.black_forward(), b.black_right()).get_black_moves();
        
        if (M.empty()) {
            if (is_white) b.white = 0; else b.black = 0;
            break;
        }
        
        // --- THE FIX: 1-Ply Win Detector ---
        // Breakthrough rules: White wins on Row 8 (0x00...ff), Black wins on Row 1 (0xff...00)
        uint64_t win_mask = is_white ? 0x00000000000000ffULL : 0xff00000000000000ULL;
        bool found_win = false;
        Move64_t best_m = M[0];
        
        for (Move64_t m : M) {
            if (m.pf & win_mask) {
                best_m = m;
                found_win = true;
                break;
            }
        }
        
        // If there is no immediate win, fall back to PPA logic
        if (!found_win) {
            global_seed = rand_xorshift(global_seed);
            
            if ((global_seed % 100) < 25) {
                best_m = M[global_seed % M.size()]; // Explore
            } else {
                int max_p = -1;
                for (Move64_t m : M) {
                    int enc = encode_move(m);
                    int p_val = is_white ? P_white[enc] : P_black[enc];
                    if (p_val > max_p) {
                        max_p = p_val;
                        best_m = m;
                    }
                }
            }
        }
        
        seq.push_back(best_m);
        b.apply_move(best_m, is_white);
        is_white = !is_white;
    }
}

Move64_t get_mcts_move(Board64_t start_board, bool is_white) {
    // --- Instant Win Check at Root ---
    std::vector<Move64_t> root_moves = is_white ? 
        Lfr_t(start_board.white_left(), start_board.white_forward(), start_board.white_right()).get_white_moves() : 
        Lfr_t(start_board.black_left(), start_board.black_forward(), start_board.black_right()).get_black_moves();
        
    uint64_t root_win_mask = is_white ? 0x00000000000000ffULL : 0xff00000000000000ULL;
    for (Move64_t m : root_moves) {
        if (m.pf & root_win_mask) return m; // Why think for 1s if we can win right now?
    }

    std::vector<Node> tree;
    tree.reserve(100000000); 

    Move64_t dummy_move;
    dummy_move.pi = 0; dummy_move.pf = 0;
    tree.emplace_back(start_board, is_white, -1, dummy_move);

    int iter = 0;
    auto start_time = std::chrono::steady_clock::now();
    
    while (true) {
        if ((iter & 255) == 0) {
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
            if (elapsed > 950) break; 
        }
        iter++;

        // 1. Selection
        int current = 0; 
        while (!tree[current].is_terminal && tree[current].unexpanded_moves.empty()) {
            double max_uct = -1.0;
            std::vector<int> best_children;
            double log_N = std::log(tree[current].n);
            
            for (int child_idx : tree[current].children) {
                double w = tree[child_idx].w;
                double n = tree[child_idx].n;
                
                double exploit = tree[current].is_white ? (w / n) : ((n - w) / n);
                double explore = 0.4 * std::sqrt(log_N / n); 
                double uct = exploit + explore;
                
                if (uct > max_uct + 1e-6) {
                    max_uct = uct;
                    best_children.clear();
                    best_children.push_back(child_idx);
                } else if (std::abs(uct - max_uct) <= 1e-6) {
                    best_children.push_back(child_idx);
                }
            }
            
            if (best_children.empty()) break; 
            
            global_seed = rand_xorshift(global_seed);
            current = best_children[global_seed % best_children.size()];
        }

        // 2. Expansion
        if (!tree[current].is_terminal && !tree[current].unexpanded_moves.empty()) {
            global_seed = rand_xorshift(global_seed);
            int move_idx = global_seed % tree[current].unexpanded_moves.size();
            
            Move64_t m = tree[current].unexpanded_moves[move_idx];
            tree[current].unexpanded_moves[move_idx] = tree[current].unexpanded_moves.back();
            tree[current].unexpanded_moves.pop_back();
            
            Board64_t next_b = tree[current].b;
            next_b.apply_move(m, tree[current].is_white);
            
            int new_node_idx = tree.size();
            tree.emplace_back(next_b, !tree[current].is_white, current, m);
            tree[current].children.push_back(new_node_idx);
            
            current = new_node_idx;
        }

        // 3. Simulation with PPA & Win Detection
        double r = 0.0;
        std::vector<Move64_t> seq;
        
        if (tree[current].b.white_win()) {
            r = 1.0;
        } else if (tree[current].b.black_win()) {
            r = 0.0;
        } else if (tree[current].is_terminal) {
            r = tree[current].is_white ? 0.0 : 1.0; 
        } else {
            Board64_t sim_b = tree[current].b;
            global_seed = rand_xorshift(global_seed);
            sim_b.seed = global_seed;
            ppa_seq_playout(sim_b, tree[current].is_white, seq);
            r = sim_b.white_win() ? 1.0 : 0.0;
        }

        // PPA Adaptation Phase
        bool turn_is_white = tree[current].is_white;
        for (Move64_t m : seq) {
            int enc = encode_move(m);
            if (r == 1.0) { 
                if (turn_is_white) P_white[enc]++;
                else P_black[enc] = std::max(0, P_black[enc] - 1);
            } else { 
                if (!turn_is_white) P_black[enc]++;
                else P_white[enc] = std::max(0, P_white[enc] - 1);
            }
            turn_is_white = !turn_is_white;
        }

        // 4. Backpropagation
        int node = current;
        while (node != -1) {
            tree[node].n += 1;
            tree[node].w += r;
            node = tree[node].parent;
        }
    }

    // Return most visited child
    int best_visits = -1;
    Move64_t best_move = tree[0].children.empty() ? dummy_move : tree[tree[0].children[0]].move_from_parent;
    
    for (int child_idx : tree[0].children) {
        if (tree[child_idx].n > best_visits) {
            best_visits = tree[child_idx].n;
            best_move = tree[child_idx].move_from_parent;
        }
    }

    if (best_visits == -1 && !root_moves.empty()) best_move = root_moves[0];

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
    if(std::string(_av[2]).compare("O")==0) {
        printf("%s\n", genmove(B, WHITE).c_str());
    } else if(std::string(_av[2]).compare("@")==0) {
        printf("%s\n", genmove(B, BLACK).c_str());
    }
    return 0;
}
