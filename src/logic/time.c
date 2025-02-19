//
// Created by paul on 2/17/25.
//

#include "logic/time.h"
#include "logic/game.h"
#include "presentation/print.h"
#include "presentation/cursor.h"
#include "hal_low/timer.h"

volatile int time_sumTicks = 0;
volatile int time_roundTicks = 0;

void time_init()
{
    timer_init_detailed(TIMER0, TIME_POWER, TIMER_MODE_TIMER, TIMER_BIT_MODE_32);
    timer_captureCompareSet(TIMER0, CC0, 2000, true);
    timer_start(TIMER0);
}

void time_onInterrupt()
{
    time_sumTicks++;
    time_roundTicks = (time_roundTicks + 1) % TICKS_PER_ROUND;
    if(time_roundTicks == TICKS_PER_ROUND - 1) {
        game_onTimeOut();
    }
}

void time_finishRound() {
    time_roundTicks = 0;
}
