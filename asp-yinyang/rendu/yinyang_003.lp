r(1..boardsize).
c(1..boardsize).
color(black;white).

#defined given/3.

1 { cell(R,C,black); cell(R,C,white) } 1 :- r(R), c(C).
cell(R,C,K) :- given(R,C,K).
:- given(R,C,K1), cell(R,C,K2), K1 != K2.

adj(R,C,R+1,C) :- r(R), c(C), R < boardsize.
adj(R,C,R-1,C) :- r(R), c(C), R > 1.
adj(R,C,R,C+1) :- r(R), c(C), C < boardsize.
adj(R,C,R,C-1) :- r(R), c(C), C > 1.

idx(R,C, R*boardsize + C) :- r(R), c(C).
has_smaller(K, I) :- color(K), cell(R,C,K), idx(R,C,I), cell(R2,C2,K), idx(R2,C2,I2), I2 < I.
root(K, R, C) :- color(K), cell(R,C,K), idx(R,C,I), not has_smaller(K, I).

has_same_neighbor(R,C) :- cell(R,C,K), cell(R2,C2,K), adj(R,C,R2,C2).
:- cell(R,C,_), not has_same_neighbor(R,C), boardsize > 1.

:- r(R), c(C), R < boardsize, C < boardsize, color(K),
   cell(R,C,K), cell(R+1,C,K), cell(R,C+1,K), cell(R+1,C+1,K).

:- r(R), c(C), R < boardsize, C < boardsize, color(K1), color(K2), K1 != K2,
   cell(R,C,K1), cell(R+1,C+1,K1),
   cell(R+1,C,K2), cell(R,C+1,K2).

reach(K,R,C) :- root(K,R,C).
reach(K,R2,C2) :- reach(K,R1,C1), adj(R1,C1,R2,C2), cell(R2,C2,K).
:- cell(R,C,K), not reach(K,R,C).

#show cell/3.
