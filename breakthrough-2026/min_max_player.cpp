#include <cstdlib>   
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <chrono>
#include "bkbb64.h"

#define WHITE 0
#define BLACK 1

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
