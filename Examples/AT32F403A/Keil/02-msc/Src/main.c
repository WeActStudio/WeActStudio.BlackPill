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

#include "spim.h"

#include "usbd_core.h"
#include "msc_class.h"
#include "msc_desc.h"
#include "usbd_int.h"

#include "board.h"

usbd_core_type usb_core_dev;

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

  /* For SPI FLASH */
  spim_init();

  /* select usb 48m clcok source */
  usb_clock48m_select(USB_CLK_HICK);

  /* enable usb clock */
  crm_periph_clock_enable(CRM_USB_PERIPH_CLOCK, TRUE);

  /* enable usb interrupt */
  nvic_irq_enable(USBFS_L_CAN1_RX0_IRQn, 0, 0);

  /* usb core init */
  usbd_core_init(&usb_core_dev, USB, &msc_class_handler, &msc_desc_handler, 0);

  /* enable usb pull-up */
  usbd_connect(&usb_core_dev);

  while (1)
  {
    led_toggle();
    delay_ms(g_speed * DELAY);
  }
}

/**
 * @brief  this function handles usb interrupt.
 * @param  none
 * @retval none
 */
void USBFS_L_CAN1_RX0_IRQHandler(void)
{
  usbd_irq_handler(&usb_core_dev);
}

/**
 * @brief  usb delay millisecond function.
 * @param  ms: number of millisecond delay
 * @retval none
 */
void usb_delay_ms(uint32_t ms)
{
  /* user can define self delay function */
  delay_ms(ms);
}

/**
 * @brief  usb delay microsecond function.
 * @param  us: number of microsecond delay
 * @retval none
 */
void usb_delay_us(uint32_t us)
{
  delay_us(us);
}

/**
 * @}
 */

/**
 * @}
 */
