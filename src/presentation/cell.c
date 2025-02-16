#include "presentation/cursor.h"
#include "presentation/field.h"
#include "presentation/cell.h"
#include "presentation/print.h"

uint8_t CELL_WIDTH = 8;
uint8_t CELL_HEIGHT = 5;

void cell_select(Cell *cell)
{
    static uint16_t prev_top = 0;
    static uint16_t prev_left = 0;
    static bool has_prev = false;

    // with +1 to account for top/left divider
    uint16_t top = (cell->row * (CELL_HEIGHT - 1)) + 1;
    uint16_t left = (cell->col * CELL_WIDTH) + 1;
    // with -2 account for dividers
    if(has_prev)
    {
        redrawCell(prev_top, prev_left, RESET);
    }
    else
    {
        has_prev = true;
    }

    redrawCell(top, left, INVERSE);

    prev_top = top;
    prev_left = left;
}

void redrawCell(uint16_t top, uint16_t left, char *modifier)
{
    uint16_t bot = top + (CELL_HEIGHT - 2);
    uint16_t right = left + (CELL_WIDTH - 2);

    for(uint16_t i = top; i < bot; i++)
    {
        cursor_moveTo(left + FIELD_X_OFFSET, i + FIELD_Y_OFFSET);
        print(modifier);

        for(uint8_t j = 0; j < (CELL_WIDTH - 1); j++)
        {
            print(" ");
        }


        cursor_moveTo(right + FIELD_X_OFFSET, i + FIELD_Y_OFFSET);
        print(RESET);
    }
}

bool cell_moveUp(Cell *selected_field)
{
    if(selected_field->row == 0)
    {
        return false;
    }
    selected_field->row -= 1;
    return true;
}

bool cell_moveDown(Cell *selected_field)
{
    if(selected_field->row == (CELLS_PER_COL - 1))
    {
        return false;
    }
    selected_field->row += 1;
    return true;
}

bool cell_moveLeft(Cell *selected_field)
{
    if(selected_field->col == 0)
    {
        return false;
    }
    selected_field->col -= 1;
    return true;
}

bool cell_moveRight(Cell *selected_field)
{
    if(selected_field->col == (CELLS_PER_ROW - 1))
    {
        return false;
    }
    selected_field->col += 1;
    return true;
}
