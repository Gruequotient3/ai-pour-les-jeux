import sys
import os
from clingo.control import Control

symbol_to_color = {".": None, "B": "black", "W": "white"}
color_to_symbol = {"black": "B", "white": "W"}

default_boardsize = 15
default_board = (
"............B.."
"......W..W.BBB."
".WWW.B.W..WWW.."
"..BBB.BB.BB.B.."
"...........B..."
"..BBBBBBBB..W.."
".WWWWW....B...."
"...B...B.B..W.."
".W..B...B.B...."
"...B.W.B....W.."
".WWWW.B.B..W..."
"......B.BW..W.."
".W.B..B.B..BW.."
"....W....W..W.."
"..............."
)


def validate_board(boardsize, board):
    if len(board) != boardsize * boardsize:
        raise ValueError(
            f"board length must be {boardsize * boardsize}, got {len(board)}"
        )
    for c in board:
        if c not in symbol_to_color:
            raise ValueError("board must only use '.', 'B', and 'W'")


def build_program_facts(boardsize, board):
    buf = f"#const boardsize={boardsize}.\n"
    for i in range(boardsize):
        for j in range(boardsize):
            color = symbol_to_color[board[i * boardsize + j]]
            if color is not None:
                buf = buf + f"given({i + 1},{j + 1},{color}).\n"
    return buf


def parse_cell_atom(atom):
    payload = atom[5:-1]
    row, col, color = payload.split(",")
    return int(row), int(col), color


def print_yinyang_board(model, board, boardsize):
    solved_board = list(board)
    atoms = format(model).split()
    for atom in atoms:
        if not atom.startswith("cell"):
            continue
        row, col, color = parse_cell_atom(atom)
        row = row - 1
        col = col - 1
        solved_board[row * boardsize + col] = color_to_symbol[color]

    for i in range(boardsize):
        print("".join(solved_board[i * boardsize : (i + 1) * boardsize]))


def main():
    boardsize = default_boardsize
    board = default_board

    if len(sys.argv) == 3:
        boardsize = int(sys.argv[1])
        board = sys.argv[2]
    elif len(sys.argv) != 1:
        print(f"usage: {sys.argv[0]} [BOARDSIZE BOARD]")
        print("BOARD uses '.' for unknown, 'B' for black clues, 'W' for white clues.")
        sys.exit(1)

    try:
        validate_board(boardsize, board)
    except ValueError as exc:
        print(exc)
        sys.exit(1)

    ctl = Control()
    ctl.configuration.solve.models = 1
    ctl.add("base", [], build_program_facts(boardsize, board))
    lp_file = os.path.join(os.path.dirname(__file__), "yinyang_002.lp")
    ctl.load(lp_file)
    ctl.ground()

    has_model = {"value": False}

    def on_model(model):
        has_model["value"] = True
        print_yinyang_board(model, board, boardsize)

    ctl.solve(on_model=on_model)

    if not has_model["value"]:
        print("UNSAT")


if __name__ == "__main__":
    main()
