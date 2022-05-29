/*---------------------------------------
- WeAct Studio Official Link
- taobao: weactstudio.taobao.com
- aliexpress: weactstudio.aliexpress.com
- github: github.com/WeActTC
- gitee: gitee.com/WeAct-TC
- blog: www.weact-tc.cn
---------------------------------------*/

#include "at32f403a_407_clock.h"

#include "delay.h"

#include "board.h"

/**
 * @brief  main function.
 * @param  none
 * @retval none
 */
int main(void)
{
  system_clock_config();

  delay_init();

  button_init();
  button_exint_init();
  led_init();

  led_init();

  while (1)
  {
    led_toggle();
    delay_ms(g_speed * DELAY);
  }
}

/**
 * @}
 */

/**
 * @}
 */
