//
// Created by paul on 2/10/25.
//

#ifndef FIELD_H
#define FIELD_H

#include <stdint.h>
#include <stddef.h>

#define CELLS_PER_ROW 8
#define CELLS_PER_COL 3

#define FIELD_X_OFFSET 1
#define FIELD_Y_OFFSET 3

#define UNICODE// if not UNICODE then ascii automatically

#ifdef UNICODE
#define CORNER_TOP_LEFT "╭"
#define CORNER_BOT_LEFT "╰"
#define CORNER_TOP_RIGHT "╮"
#define CORNER_BOT_RIGHT "╯"

#define T_LEFT "├"
#define T_RIGHT "┤"
#define T_TOP "┬"
#define T_BOT "┴"

#define CROSS "┼"

#define SIDE "│"
#define TOP_BOTTOM "─"
#define EMPTY " "

#define DIAGONAL_FW "╱"
#define DIAGONAL_BW "╲"
#define DIAGONAL_MID "╳"
#else
#define CORNER_TOP_LEFT "+"
#define CORNER_BOT_LEFT "+"
#define CORNER_TOP_RIGHT "+"
#define CORNER_BOT_RIGHT "+"

#define T_LEFT "+"
#define T_RIGHT "+"
#define T_TOP "+"
#define T_BOT "+"

#define CROSS "+"

#define SIDE "|"
#define TOP_BOTTOM "-"
#define EMPTY " "

#define DIAGONAL_FW "/"
#define DIAGONAL_BW "\\"
#define DIAGONAL_MID "X"
#endif

/**
 * @brief Initial drawing of the playing field
 **/
void field_redraw();

static void printRow(char **row, size_t row_length);

#endif// FIELD_H
