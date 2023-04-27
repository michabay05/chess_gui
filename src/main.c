#include "board.h"
#include "fen.h"
#include "gui_defs.h"
#include "precalculate.h"
#include "bitboard.h"
#include "move.h"

#include <string.h>

typedef enum {
  GUI,
  TERM,
  DEBUG
} Mode;

Mode parse_cmd_args(int argc, char **argv) {
  Mode selected = GUI; // GUI is the default mode
  if (argc >= 2) {
    if (!strcmp(argv[1], "Term"))
      selected = TERM;
    else if (!strcmp(argv[1], "Debug"))
      selected = DEBUG;
  }
  return selected;
}

#include "move_gen.h"
int test_main(void) {
  FENInfo f = parse_fen("r4rk1/1b3ppp/pq2pn2/1p6/3P4/B7/2PNQPPP/1R3RK1 w - - 0 1");
  Board b = {0};
  board_set_from_fen(&b, f);
  board_print(&b);
  MoveList ml = {0};
  movelist_generate(&ml, &b);
  movelist_print_list(ml);
  return 0;
}

void init(void) {
  attack_init();
}

int main(int argc, char *argv[]) {
  init();
  switch (parse_cmd_args(argc, argv)) {
  case GUI:
    gui_main();
    break;
  case TERM:
    printf("********** TODO: Terminal mode is still not implemented.\n");
    return 1;
    // TODO: Add term main();
  case DEBUG:
    return test_main();
  };
  return 0;
}
