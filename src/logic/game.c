#include "logic/game.h"

#include "hal_high/input_buf.h"
#include "hal_low/nvic.h"
#include "hal_low/random.h"
#include "hal_low/timer.h"
#include "util/math.h"
#include "logic/bot.h"
#include "logic/input.h"
#include "logic/time.h"
#include "presentation/cell.h"
#include "presentation/cursor.h"
#include "presentation/field.h"
#include "presentation/print.h"
#include "presentation/ui.h"
#include "util/conversion.h"

volatile bool force_ui_update = false;

void game_run(const Mode mode)
{
    init(mode);

    uint8_t input;
    Player winner;
    while(true)
    {
        while(!input_getNext(&input_buf, &input))
        {
            // If true, Move was forced due to inactivity
            if(force_ui_update)
            {
                handleForcedMoveUpdate();
                force_ui_update = false;
                break;
            }
            ui_updateTimer(REMAINING_TIME, TICKS_PER_ROUND);
            __WFI();
        }

        const bool should_redraw_field = handleInput(&input);

        winner = checkForWinner();

        cell_select(selected_cell);
        if(winner != None || input == 'q' || turn_number >= CELLS_PER_COL * CELLS_PER_ROW / 2)
        {
            timer_stop(TIMER0);
            break;
        }

        if(should_redraw_field)
        {
            redrawField();
        }
    }
    printEndScreen(winner);
}

void game_onTimeOut()
{
    force_ui_update = true;
    if(game_mode == PVE)
    {
        last_marked_circle = bot_markRandomCell(cells, Circle);
        last_marked_cross = bot_markRandomCell(cells, Cross);
        turn_number++;
    }
    else if(current_player == Circle)
    {
        last_marked_circle = bot_markRandomCell(cells, Circle);
        current_player = Cross;
        turn_number++;
    }
    else if(current_player == Cross)
    {
        last_marked_cross = bot_markRandomCell(cells, Cross);
        current_player = Circle;
    }
    time_finishRound();
}

void init(const Mode mode)
{
    turn_number = FIRST_TURN;
    last_ui_update = turn_number;
    game_mode = mode;
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

    if(selected_cell != NULL)
    {
        cell_select(selected_cell);
    }
    selected_cell = &cells[0][0];

    clearConsole();
    time_init();
    redrawField();
}

bool handleInput(const uint8_t *input)
{
    switch(*input)
    {
        case '\e':
            input_handleEscapeSequence(cells, &selected_cell);
            break;
        case ' ':
            if(selected_cell->marked_by != None)
            {
                break;
            }

            if(game_mode == PVE)
            {
                selected_cell->marked_by = Circle;
                last_marked_cross = bot_markRandomCell(cells, Cross);
                last_marked_circle = selected_cell;
                cell_redraw(last_marked_cross);
                turn_number++;
            }
            else
            {
                selected_cell->marked_by = current_player;
                if(current_player == Circle)
                {
                    last_marked_circle = selected_cell;
                }
                else
                {
                    last_marked_cross = selected_cell;
                    turn_number++;
                }
                current_player = current_player == Circle ? Cross : Circle;
            }

            ui_updateTurn(turn_number, current_player);
            last_ui_update = turn_number;
            time_finishRound();
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
    ui_displayTurn(turn_number, current_player);
    ui_displayTimer(REMAINING_TIME, TICKS_PER_ROUND);
    field_redraw();
    cell_redrawAll(cells);
    cell_select(selected_cell);
}

void handleForcedMoveUpdate()
{
    cell_select(last_marked_cross);
    cell_select(last_marked_circle);
    ui_updateTurn(turn_number, current_player);

    if(game_mode == PVE)
    {
        selected_cell = last_marked_circle;
    }
    else
    {
        selected_cell = current_player == Circle ? last_marked_cross : last_marked_circle;
    }
}

Player checkForWinner()
{
    if(checkIfPlayerWon(last_marked_circle, Circle))
    {
        return Circle;
    }
    if(checkIfPlayerWon(last_marked_cross, Cross))
    {
        return Cross;
    }
    return None;
}

bool checkIfPlayerWon(const Cell *cell, const Player player)
{
    if(cell == NULL)
    {
        return false;
    }

    // VICTORY ALGORITHM
    // Vertical / Horizontal:
    // The algorithm starts at the border and iterates to the other side. It goes in 2 different
    // directions and counts on 2 axes. It runs in one of the directions till a cell is reached that
    // is not marked by our player. Diagonal: The algorithm starts at the given cell coordinates. It
    // goes in 4 different direction and counts on 2 axes. Every step in one of the directions
    // increments the dedicated axis until a cell is reached that is not marked by our player.
    //
    // The return value is true when one axis or more axes has / have reached the win condition
    // (line to the other side).
    //  UP      UP     UP          | axis       | direction 1 | direction 2 |
    // LEFT           RIGHT        |------------+-------------+-------------|
    //        \ | /                | vertical   | down        |             |
    // LEFT  ---+---> RIGHT        | horizontal | right       |             |
    //        / | \                | diagonal_1 | up right    | down left   |
    // DOWN     v     DOWN         | diagonal_2 | up left     | down right  |
    // LEFT    DOWN   RIGHT
    const uint8_t diagonal_win_condition = min(CELLS_PER_COL, CELLS_PER_ROW);
    const uint8_t max_cells = max(CELLS_PER_COL, CELLS_PER_ROW);
    const uint8_t row = cell->row, col = cell->col;
    const uint8_t max_row = CELLS_PER_COL - 1, max_col = CELLS_PER_ROW - 1;
    uint8_t diagonal_1_sum = 1, diagonal_2_sum = 1;
    bool vertical_match = true, horizontal_match = true;
    bool diagonal_1_up_right = true, diagonal_1_down_left = true, diagonal_2_up_left = true,
         diagonal_2_down_right = true;
    for(uint8_t i = 1, n = 0; n < max_cells; i++, n++)
    {
        // --- VERTICAL ---
        // downwards
        if(n <= max_row && cells[n][col].marked_by != player)
        {
            vertical_match = false;
        }

        // --- HORIZONTAL ---
        // to the right
        if(n <= max_col && cells[row][n].marked_by != player)
        {
            horizontal_match = false;
        }

        // --- 1st DIAGONAL ---
        // upwards to the right
        if(diagonal_1_up_right && row - i >= 0 && col + i <= max_col
           && cells[row - i][col + i].marked_by == player)
        {
            diagonal_1_sum++;
        }
        else
        {
            diagonal_1_up_right = false;
        }
        // downwards to the left
        if(diagonal_1_down_left && row + i <= max_row && col - i >= 0
           && cells[row + i][col - i].marked_by == player)
        {
            diagonal_1_sum++;
        }
        else
        {
            diagonal_1_down_left = false;
        }

        // --- 2nd DIAGONAL ---
        // upwards to the left
        if(diagonal_2_up_left && row - i >= 0 && col - i >= 0
           && cells[row - i][col - i].marked_by == player)
        {
            diagonal_2_sum++;
        }
        else
        {
            diagonal_2_up_left = false;
        }
        // downwards to the right
        if(diagonal_2_down_right && row + i <= max_row && col + i <= max_col
           && cells[row + i][col + i].marked_by == player)
        {
            diagonal_2_sum++;
        }
        else
        {
            diagonal_2_down_right = false;
        }
    }
    if(vertical_match || horizontal_match || diagonal_1_sum >= diagonal_win_condition
       || diagonal_2_sum >= diagonal_win_condition)
    {
        return true;
    }
    return false;
}

void printEndScreen(Player winner)
{
    cursor_moveTo(1, CELLS_PER_COL * cell_height);
    switch(winner)
    {
        case Circle:
            print(BOLD);
            print(FG_GREEN);
            if(game_mode == PVE)
            {
                println("Congratulations, you have beaten the Computer!");
            }
            else
            {
                println("Congratulations, circle has won!");
            }
            print(RESET);
            break;
        case Cross:
            print(BOLD);
            print(FG_MAGENTA);
            if(game_mode == PVE)
            {
                println("Better luck next time! The Computer has beaten you...");
            }
            else
            {
                println("Congratulation, cross has won!");
            }
            print(RESET);
            break;
        default:;
    }
    print(ITALIC);
    println("Press any key to see the statistics...");
    print(RESET);

    uint8_t input;
    while(!input_getNext(&input_buf, &input))
    {
        __WFI();
    }

    clearConsole();

    print("Turns needed:\t");
    print(BOLD);
    println_int(turn_number);
    print(RESET);

    print("Total ticks:\t");
    print(BOLD);
    println_int(time_sumTicks);
    print(RESET);

    print("Ø ticks / round:\t");
    print(BOLD);
    println_int(time_sumTicks / turn_number);
    print(RESET);

    print(ITALIC);
    println("Press any key to return to the menu...");
    print(RESET);
    while(!input_getNext(&input_buf, &input))
    {
        __WFI();
    }
}
