#include "at32f403a_407_clock.h"

#include "delay.h"

#include "board.h"

#include "run_in_spim.h"

#include "spim.h"

// SPIM Download Algorithm Select Type2_Remap1
/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();
    
  spim_init(); 
    
  delay_init();
    
  button_init();
  button_exint_init();
  led_init();
  
  run_in_spim_test();
    
  while(1)
  {

  }
}

/**
  * @}
  */

/**
  * @}
  */
