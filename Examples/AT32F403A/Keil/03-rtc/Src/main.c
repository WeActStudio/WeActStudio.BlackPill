#include "at32f403a_407_clock.h"

#include "delay.h"

#include "usbd_core.h"
#include "usbd_int.h"

#include "cdc_class.h"
#include "cdc_desc.h"

#include "rtc.h"

#include "board.h"

#include "string.h"

usbd_core_type usb_core_dev;
uint8_t usb_buffer[256];
uint8_t rtc_buffer[256];
char const weekday_table[7][10] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  uint16_t data_len;

  uint32_t timeout;

  uint8_t send_zero_packet = 0;
  
  calendar_type time_struct;
    
  /* config nvic priority group */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    
  system_clock_config();
  
  delay_init();
    
  button_init();
  button_exint_init();
  led_init();
    
  /* config calendar */
  time_struct.year  = 2022;
  time_struct.month = 5;
  time_struct.date  = 10;
  time_struct.hour  = 12;
  time_struct.min   = 0;
  time_struct.sec   = 0;
  rtc_init(&time_struct);

  /* select usb 48m clcok source */
  usb_clock48m_select(USB_CLK_HICK);
    
    /* enable usb clock */
  crm_periph_clock_enable(CRM_USB_PERIPH_CLOCK, TRUE);

  /* enable usb interrupt */
  nvic_irq_enable(USBFS_L_CAN1_RX0_IRQn, 0, 0);

  /* usb core init */
  usbd_core_init(&usb_core_dev, USB, &cdc_class_handler, &cdc_desc_handler, 0);

  /* enable usb pull-up */
  usbd_connect(&usb_core_dev);
  
  while(1)
  {
    // print rtc data
    if(rtc_flag_get(RTC_TS_FLAG) != RESET)
    {
      led_toggle();
        
      /* get time */
      rtc_time_get();

      /* print time */
      sprintf((char *)rtc_buffer,"%d/%d/%d %02d:%02d:%02d %s\r\nWeAct Studio\r\n", calendar.year, calendar.month, calendar.date \
                                                                , calendar.hour, calendar.min, calendar.sec, weekday_table[calendar.week]);

      /* wait for the register write to complete */
      rtc_wait_config_finish();

      /* clear the rtc second flag */
      rtc_flag_clear(RTC_TS_FLAG);

      /* wait for the register write to complete */
      rtc_wait_config_finish();
      
      if(usb_core_dev.conn_state == USB_CONN_STATE_CONFIGURED)
      {
          timeout = 500000;
          do
          {
            /* send data to host */
            if(usb_vcp_send_data(&usb_core_dev, rtc_buffer, strlen((char *)rtc_buffer)) == SUCCESS)
            {
              break;
            }
          }while(timeout --);
      }
    }
    
    /* get usb vcp receive data */
    data_len = usb_vcp_get_rxdata(&usb_core_dev, usb_buffer);

    if(data_len > 0 || send_zero_packet == 1)
    {

      /* bulk transfer is complete when the endpoint does one of the following
         1 has transferred exactly the amount of data expected
         2 transfers a packet with a payload size less than wMaxPacketSize or transfers a zero-length packet
      */
      if(data_len > 0)
        send_zero_packet = 1;

      if(data_len == 0)
        send_zero_packet = 0;

      timeout = 500000;
      do
      {
        /* send data to host */
        if(usb_vcp_send_data(&usb_core_dev, usb_buffer, data_len) == SUCCESS)
        {
          break;
        }
      }while(timeout --);
    }
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
