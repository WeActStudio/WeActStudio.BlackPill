#include "spim.h"

void spim_init(void)
{
  gpio_init_type gpio_init_struct;

  /* enable the clock */
  crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

  /* init spim io */
  gpio_default_para_init(&gpio_init_struct);
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_pins = GPIO_PINS_8;
  gpio_init(GPIOA, &gpio_init_struct);
  gpio_init_struct.gpio_pins = GPIO_PINS_1 | GPIO_PINS_6 | GPIO_PINS_7 | GPIO_PINS_10 | GPIO_PINS_11;
  gpio_init(GPIOB, &gpio_init_struct);

  /* enable spim, and select pb10, pb11 as spim io */
  gpio_pin_remap_config(EXT_SPIM_GMUX_1001, TRUE);

  /* in this example, use on-board w25q64j as spim flash */
  flash_spim_model_select(FLASH_SPIM_MODEL2);

  /* unlock the spim flash program erase controller */
  while(flash_flag_get(FLASH_SPIM_OBF_FLAG));
  flash_spim_unlock();
  while(FLASH->ctrl3_bit.oplk);

  /* if the data written to spim flash need to be scrambled, please specify the scrambled range */
  flash_spim_encryption_range_set(0);

  return;
}

#if 0

#include <string.h>

#define SPIM_SECTOR_SIZE    4096
#define SPIM_TEST_ADDR      0x08400000

uint8_t write_buffer[4096];  /* the content writed to spim flash */
uint8_t read_buffer[4096];   /* the content read from spim flash */

/**
  * @brief  read one sector from a specified address in spim flash
  * @param  address: specifies the sector starting address
  * @param  sector_size: sector size in bytes
  * @param  buf: the content read from spim flash
  * @retval none
  */
void sector_read(uint32_t address, uint32_t sector_size, uint8_t *buf)
{
  while(sector_size)
  {
    *(uint32_t *)buf = *(uint32_t *)address;
    sector_size -= sizeof(uint32_t);
    address += sizeof(uint32_t);
    buf += sizeof(uint32_t);
  }
}

/**
  * @brief  operate spim flash
  * @param  none
  * @retval none
  */
void spim_operate(void)
{
  uint16_t i = 0;

  /* configures the spim flash */
  spim_init();

  /* fill the content to be writed to spim flash */
  for(i = 0; i < SPIM_SECTOR_SIZE; i++)
  {
    write_buffer[i] = i % 256;
  }

  /* erase an spim flash sector */
  flash_sector_erase(SPIM_TEST_ADDR);

  /* read an spim flash sector */
  memset(read_buffer, 0, SPIM_SECTOR_SIZE);
  sector_read(SPIM_TEST_ADDR, SPIM_SECTOR_SIZE, read_buffer);

  /* check if the desired sector are erased */
  for(i = 0; i < SPIM_SECTOR_SIZE; i++)
  {
    if(read_buffer[i] != 0xff)
    {
      /* test spim fail */
      return;
    }
  }

  /* program an spim flash sector */
  i = 0;
  while(i < SPIM_SECTOR_SIZE)
  {
    flash_word_program(SPIM_TEST_ADDR + i, *(uint32_t *)(write_buffer + i));
    i = i + 4;
  }

  /* read an spim flash sector */
  memset(read_buffer, 0, SPIM_SECTOR_SIZE);
  sector_read(SPIM_TEST_ADDR, SPIM_SECTOR_SIZE, read_buffer);

  /* check if reading result and writing content are the same */
  for(i = 0; i < SPIM_SECTOR_SIZE; i++)
  {
    if(read_buffer[i] != write_buffer[i])
    {
      /* test spim fail */
      while(1)
      {

      }
      return;
    }
  }


}
#endif

