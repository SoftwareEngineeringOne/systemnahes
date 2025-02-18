#include "def.h"
#include "hal_high/input_buf.h"
#include "hal_low/random.h"
#include "hal_low/uart.h"
#include "hal_low/nvic.h"
#include "presentation/print.h"
#include "logic/game.h"

extern volatile InputBuffer input_buf;


void menu_todo()
{
    clearConsole();
    println("Not yet implemented, any key to continue...");
    while(input_isEmpty(&input_buf))
    {
        __WFI();
    }
}


int main(void)
{
    uart_init(true);
    input_init(&input_buf);


    uint8_t input;
    while(true)
    {
        clearConsole();
        print("\e[H");
        println("Arrows to move");
        println("1. Play");
        println("2. Settings");
        println("3. Credits");
        println("4. Quit (not really hahahahaha)");

        while(!input_getNext(&input_buf, &input))
        {
            __WFI();
        }

        bool should_quit = false;
        switch(input)
        {
            case '1':
                game_run();
                break;
            case '2':
                menu_todo();
                break;
            case '3':
                menu_todo();
                break;
            case '4':
                should_quit = true;
                break;
            default:
                continue;
        }

        if(should_quit)
        {
            break;
        }
    }
    println("Thanks for playing!");
    println("For now you can press \"CTR + A\" and then \"x\" to exit qemu!");

    print(SHOW_CURSOR);
}


/* static void waitRelease(void) */
/* { */
/*     bool start = false; */
/*     while(!start) {} */
/* } */
/**/
/* static void randomNumberTest(void) */
/* { */
/*     // Initialize the Module */
/*     rng_init(); */
/**/
/*     while(true) */
/*     { */
/*         uint8_t randomNumber = rng_getRandomValue_waiting(); */
/**/
/*         // uart_writeNumber(randomNumber); */
/*         // uart_writeString("\n"); */
/*     } */
/* } */
/**/
/* static void timerTest(void) */
/* { */
/*     // timer_init( TIMER0 ); */
/**/
/*     timer_init_detailed(TIMER0, 15, TIMER_MODE_TIMER, TIMER_BIT_MODE_32); */
/**/
/*     timer_captureCompareSet(TIMER0, CC0, 2000, true); */
/**/
/*     timer_start(TIMER0); */
/**/
/*     while(true) */
/*     { */
/*         timer_capture(TIMER0, CC1); */
/**/
/*         uint32_t captureValue = timer_captureCompareGet(TIMER0, CC1); */
/**/
/*         // uart_writeNumber( captureValue ); */
/*     } */
/* } */
