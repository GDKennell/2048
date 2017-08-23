//
//  board.h
//  hello
//
//  Created by Grant Kennell on 8/22/17.
//
//

#ifndef board_h
#define board_h

#include <stdio.h>

typedef __int64_t board_col_t;
typedef __uint64_t board_t;
typedef __uint64_t uint64_t;

board_col_t board_raw_column( board_t board_t, int col_num);

void board_set_exp( board_t *board, int x, int y, int exp);

void board_set_value( board_t *board, int x, int y, int val);


#endif /* board_h */
