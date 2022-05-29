#include "run_in_spim.h"
#include "board.h"
#include "delay.h"

void run_in_spim_test(void)
{
    while (1)
    {
        led_toggle();
        delay_ms(100);
    }
}
