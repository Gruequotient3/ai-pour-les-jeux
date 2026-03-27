#include <cstdlib>   
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <chrono>
#include "bkbb64.h"

#define WHITE 0
#define BLACK 1


int maxi(Board64_t& s, int d) {
  if (s.white_win()) return 1000000 - d;
  if (s.black_win()) return -1000000 + d;

  if (d == MINIMAX_MAX_DEPTH) {
    return s.eval(true);
  }
  std::vector<Move64_t> M;
  M = Lfr_t(s.white_left(), s.white_forward(), s.white_right()).get_white_moves();

  if (M.size() == 0) {
    return s.eval(true);
  }

  int best_v = -INT_MAX;
  for (Move64_t m: M) {
    Board64_t cp_s = s;
    cp_s.apply_white_move(m);
    int v = mini(cp_s, d+1);
    if (v > best_v) {
      best_v = v;
    }
  }
  return best_v;
}

int mini(Board64_t& s, int d) {
  if (s.white_win()) return 1000000 - d; 
  if (s.black_win()) return -1000000 + d; 

  if (d == MINIMAX_MAX_DEPTH) {
    return s.eval(true);
  }
  std::vector<Move64_t> M;

  M = Lfr_t(s.black_left(), s.black_forward(), s.black_right()).get_black_moves();
  
  if (M.size() == 0) {
    return s.eval(true);
  }

  int best_v = INT_MAX;
  for (Move64_t m: M) {
    Board64_t cp_s = s;
    cp_s.apply_black_move(m);
    int v = maxi(cp_s, d+1);
    if (v < best_v) {
      best_v = v;
    }
  }
  return best_v;
}



inline Move64_t Board64_t::get_min_max_white_move(const Lfr_t &lfr) {
  Move64_t m;
  std::vector<Move64_t> M = lfr.get_white_moves();
  int best_v = -INT_MAX;
    
  m = M[0];
  for (Move64_t mt : M){
    Board64_t cp_s = *this;
    cp_s.apply_white_move(mt);
    int v = mini(cp_s, 1);
    if (v > best_v) {
      best_v = v;
      m = mt;
    }
  }
  return m;
}
inline Move64_t Board64_t::get_min_max_white_move() {
    uint64_t wl = white_left();
    uint64_t wf = white_forward();
    uint64_t wr = white_right();
    return get_min_max_white_move(Lfr_t(wl, wf, wr));
}
inline void Board64_t::min_max_white_move() {
    apply_white_move(get_rand_white_move());
}

inline Move64_t Board64_t::get_min_max_black_move(const Lfr_t &lfr) {
  Move64_t m;
  std::vector<Move64_t> M = lfr.get_black_moves();
  int best_v = INT_MAX;
    
  m = M[0];
  for (Move64_t mt : M){
    Board64_t cp_s = *this;
    cp_s.apply_black_move(mt);
    int v = maxi(cp_s, 1); 
    if (v < best_v) {
      best_v = v;
      m = mt;
    }
  }
  return m;
}
inline Move64_t Board64_t::get_min_max_black_move() {
    uint64_t wl = black_left();
    uint64_t wf = black_forward();
    uint64_t wr = black_right();
    return get_min_max_black_move(Lfr_t(wl, wf, wr));
}
inline void Board64_t::min_max_black_move() {
    apply_black_move(get_min_max_white_move());
}

inline Move64_t Board64_t::get_min_max_move(const Lfr_t &lfr, bool _white){
    if (_white)
        return get_min_max_white_move(lfr);
    else
        return get_min_max_black_move(lfr);
}
inline Move64_t Board64_t::get_min_max_move(bool _white){
    if (_white)
        return get_min_max_white_move();
    else
        return get_min_max_black_move();
}
inline void Board64_t::min_max_move(bool _white){
    if (_white)
        min_max_white_move();
    else
        min_max_black_move();   
}

std::string genmove(Board64_t& _board, int _color) {
  Move64_t m;
  
  if(_color == WHITE) {
    m = _board.get_min_max_white_move();
  } else if (_color == BLACK) {
    m = _board.get_min_max_black_move();
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
  bool debug = false;
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
