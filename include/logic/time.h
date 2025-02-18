//
// Created by paul on 2/17/25.
//

#ifndef TIME_H
#define TIME_H

#define TICKS_PER_ROUND 8

extern volatile int time_sumTicks;
extern volatile int time_roundTicks;

void time_init();

void time_onInterrupt();

void time_finishRound();

#endif// TIME_H
