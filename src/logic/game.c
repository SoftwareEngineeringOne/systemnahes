#include "logic/game.h"

#include "hal_low/nvic.h"
#include "hal_low/uart.h"
#include "hal_low/random.h"
#include "hal_low/timer.h"
#include "hal_high/input_buf.h"
#include "logic/input.h"
#include "presentation/cell.h"
#include "presentation/cursor.h"
#include "presentation/field.h"
#include "presentation/print.h"
#include "presentation/ui.h"
#include "logic/time.h"
#include "logic/bot.h"
#include "helper/math.h"

volatile uint8_t current_turn = 0;

void game_run()
{
    init();

    uint8_t input;
    uint8_t last_update = 0;
    Player winner = None;
    while(true)
    {
        while(!input_getNext(&input_buf, &input))
        {
            // If true, Move was forced due to inactivity
            if(current_turn != last_update)
            {
                last_update = current_turn;
                handleForcedMoveUpdate();
                break;
            }
            ui_updateTimer(REMAINING_TIME);
            __WFI();
        }

        const bool should_redraw_field = handleInput(&input);

        winner = checkForWinner();

        if(winner != None || input == 'q' || current_turn >= (CELLS_PER_COL * CELLS_PER_ROW) / 2)
        {
            timer_stop(TIMER0);
            cell_redraw(selected_cell);
            break;
        }

        if(should_redraw_field)
        {
            redrawField();
        }

        cell_select(selected_cell);
    }


    cursor_moveTo(1, CELLS_PER_COL * cell_height + 1);
    switch(winner)
    {
        case Human:
            println("Congratulations for winning!");
            break;
        case Computer:
            println("Better luck next time!");
            break;
        default:;
    }
    println("Press any key to return to the menu...");
    while(input_isEmpty(&input_buf))
    {
        __WFI();
    }
}

void game_onTimeOut()
{
    last_marked_human = bot_makeHumanTurn(cells);
    last_marked_bot = bot_makeTurn(cells);
    current_turn++;
}

void init()
{
    current_turn = 0;
    last_marked_human = NULL;
    last_marked_bot = NULL;
    input_init(&input_buf);
    rng_init();
    print(HIDE_CURSOR);


    for(int y = 0; y < CELLS_PER_COL; y++)
    {
        for(int x = 0; x < CELLS_PER_ROW; x++)
        {
            cells[y][x] = (Cell){
                .col = x,
                .row = y,
                .marked_by = None,
            };
        }
    }

    selected_cell = &cells[0][0];

    clearConsole();
    time_init();
    field_redraw();
    cell_select(selected_cell);
    ui_displayTimer(REMAINING_TIME);
    ui_displayTurn(current_turn, "Player");
}

bool handleInput(const uint8_t *input)
{
    switch(*input)
    {
        case '\e':
            input_handleEscapeSequence(cells, &selected_cell);
            break;
        case ' ':
            if(selected_cell->marked_by == None)
            {
                selected_cell->marked_by = Human;
                last_marked_bot = bot_makeTurn(cells);
                last_marked_human = selected_cell;
                cell_redraw(last_marked_bot);
                current_turn++;
                ui_updateTurn(current_turn, "Player");
                time_finishRound();
            }
            break;
        case '+':
            cell_increaseSize();
            return true;
        case '-':
            cell_decreaseSize();
            return true;
        default:;
    }
    return false;
}

void redrawField()
{
    clearConsole();
    ui_displayTurn(current_turn, "Player");
    ui_displayTimer(REMAINING_TIME);
    field_redraw();
    cell_redrawAll(cells);
}

void handleForcedMoveUpdate()
{
    cell_select(last_marked_bot);
    cell_select(last_marked_human);
    ui_updateTurn(current_turn, "Player");

    selected_cell = last_marked_human;
}

Player checkForWinner()
{
    if(checkIfPlayerWon(last_marked_human, Human))
    {
        return Human;
    }
    if(checkIfPlayerWon(last_marked_bot, Computer))
    {
        return Computer;
    }
    return None;
}

bool checkIfPlayerWon(const Cell *cell, const Player player)
{
    if(cell == NULL)
    {
        return false;
    }
    uint8_t row = cell->row, col = cell->col;
    bool vertical_match = true, horizontal_match = true;
    uint8_t diagonal_matches_needed = min(CELLS_PER_ROW, CELLS_PER_COL);
    // start at 1 for the originating field
    uint8_t diagonal_1_match = 1, diagonal_2_match = 1;
    for(int8_t i = 0; i < (max(CELLS_PER_COL, CELLS_PER_ROW)); i++)
    {
        if(i < CELLS_PER_COL && cells[i][col].marked_by != player)
        {
            vertical_match = false;
        }
        if(i < CELLS_PER_ROW && cells[row][i].marked_by != player)
        {
            horizontal_match = false;
        }

        if(i != 0 && row - i >= 0 && row + i < CELLS_PER_ROW
           && cells[row - i][col + i].marked_by == player)
        {
            diagonal_1_match++;
        }

        if(i != 0 && row + i < CELLS_PER_COL && col - i >= 0
           && cells[row + i][col - i].marked_by == player)
        {
            diagonal_1_match++;
        }

        if(i != 0 && row - i >= 0 && col - i >= 0 && cells[row - i][col - i].marked_by == player)
        {
            diagonal_2_match++;
        }

        if(i != 0 && row + i < CELLS_PER_COL && row + i < CELLS_PER_ROW
           && cells[row + i][col + i].marked_by == player)
        {
            diagonal_2_match++;
        }
        // uint8_t col_offset = (cell->col > cell->row) ? cell->col - cell->row : 0;
        // uint8_t row_offset = (cell->row > cell->col) ? cell->row - cell->col : 0;
        // if(i + col_offset < CELLS_PER_ROW && i + row_offset < CELLS_PER_COL
        //    && cells[i + row_offset][i + col_offset].marked_by == player)
        // {
        //     diagonal_1_match++;
        // }
        //
        // col_offset = cell->col + cell->row < CELLS_PER_ROW
        //     ? CELLS_PER_ROW - 1 - (cell->col + cell->row)
        //     : 0;
        // row_offset = cell->col + cell->row < CELLS_PER_COL
        //     ? CELLS_PER_COL - 1 - (cell->row + (CELLS_PER_ROW - cell->col))
        //     : 0;
        // uint8_t col = CELLS_PER_ROW - 1 - (i + col_offset);
        // if(col < CELLS_PER_ROW && i + row_offset < CELLS_PER_COL
        //    && cells[i + row_offset][col].marked_by == player)
        // {
        //     diagonal_2_match++;
        // }
    }
    return vertical_match || horizontal_match || diagonal_1_match == diagonal_matches_needed
        || diagonal_2_match == diagonal_matches_needed;
}
