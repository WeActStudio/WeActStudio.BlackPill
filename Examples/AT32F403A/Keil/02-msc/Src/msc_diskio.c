/**
  **************************************************************************
  * @file     msc_diskio.c
  * @version  v2.0.9
  * @date     2022-04-25
  * @brief    usb mass storage disk function
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
#include "msc_diskio.h"
#include "msc_bot_scsi.h"
#include "spim.h"

/** @addtogroup AT32F403A_periph_examples
  * @{
  */

/** @addtogroup 403A_USB_device_msc
  * @{
  */
uint32_t sector_size = 2048;
uint32_t msc_flash_size;

// If you have spi flash, change MSC_SUPPORT_MAX_LUN to =2
uint8_t scsi_inquiry[MSC_SUPPORT_MAX_LUN][SCSI_INQUIRY_DATA_LENGTH] =
{
  /* lun = 0 */
  {
    0x00,         /* peripheral device type (direct-access device) */
    0x80,         /* removable media bit */
    0x00,         /* ansi version, ecma version, iso version */
    0x01,         /* respond data format */
    SCSI_INQUIRY_DATA_LENGTH - 5, /* additional length */
    0x00, 0x00, 0x00, /* reserved */
    'A', 'T', '3', '2', ' ', ' ', ' ', ' ', /* vendor information "AT32" */
    'D', 'i', 's', 'k', '0', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', /* Product identification "Disk" */
    '2', '.', '0', '0'  /* product revision level */
  },
  #if MSC_SUPPORT_MAX_LUN == 2
  /* lun = 1 */
  {
    0x00,         /* peripheral device type (direct-access device) */
    0x80,         /* removable media bit */
    0x00,         /* ansi version, ecma version, iso version */
    0x01,         /* respond data format */
    SCSI_INQUIRY_DATA_LENGTH - 5, /* additional length */
    0x00, 0x00, 0x00, /* reserved */
    'A', 'T', '3', '2', ' ', ' ', ' ', ' ', /* vendor information "AT32" */
    'D', 'i', 's', 'k', '0', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', /* Product identification "Disk" */
    '2', '.', '0', '0'  /* product revision level */
  }
  #endif
};

/**
  * @brief  get disk inquiry
  * @param  lun: logical units number
  * @retval inquiry string
  */
uint8_t *get_inquiry(uint8_t lun)
{
  if(lun < MSC_SUPPORT_MAX_LUN)
    return (uint8_t *)scsi_inquiry[lun];
  else
    return NULL;
}

/**
  * @brief  disk read
  * @param  lun: logical units number
  * @param  addr: logical address
  * @param  read_buf: pointer to read buffer
  * @param  len: read length
  * @retval status of usb_sts_type
  */
usb_sts_type msc_disk_read(uint8_t lun, uint32_t addr, uint8_t *read_buf, uint32_t len)
{
  uint32_t i = 0;
  uint32_t flash_addr;
  switch(lun)
  {
    case 0:
      flash_addr = addr + USB_FLASH_ADDR_OFFSET;
      for(i = 0; i < len; i ++)
      {
        read_buf[i] = *((uint8_t *)flash_addr);
        flash_addr += 1;
      }
    break;
    case 1:
      flash_addr = addr + USB_SPIFLASH_ADDR_OFFSET;
      for(i = 0; i < len; i +=4)
      {
        *(uint32_t *)read_buf = *(uint32_t *)flash_addr;
        flash_addr += 4;
          read_buf += 4;
      }
    break;
  }

      
  return USB_OK;
}

/**
  * @brief  disk write
  * @param  lun: logical units number
  * @param  addr: logical address
  * @param  buf: pointer to write buffer
  * @param  len: write length
  * @retval status of usb_sts_type
  */
usb_sts_type msc_disk_write(uint8_t lun, uint32_t addr, uint8_t *buf, uint32_t len)
{
  uint32_t flash_addr;
  uint32_t i = 0, page_len, tolen = len;
  uint32_t erase_addr;
  switch(lun)
  {
    case 0:
      flash_addr = addr + USB_FLASH_ADDR_OFFSET;
      erase_addr = flash_addr;
      page_len = SECTOR_SIZE_2K;
      flash_unlock();
      while(tolen >= page_len)
      {
        flash_sector_erase(erase_addr);
        tolen -= page_len;
        erase_addr += page_len;
      }
      for(i = 0; i < len; i ++)
      {
        flash_byte_program(flash_addr+i, buf[i]);
      }
      flash_lock();
      break;
    case 1:
      flash_addr = addr + USB_SPIFLASH_ADDR_OFFSET;
      erase_addr = flash_addr;
      page_len = SECTOR_SIZE_4K;
      while(flash_flag_get(FLASH_SPIM_OBF_FLAG));
      flash_spim_unlock();
      while(FLASH->ctrl3_bit.oplk);
      while(tolen >= page_len)
      {
        flash_sector_erase(erase_addr);
        tolen -= page_len;
        erase_addr += page_len;
      }
      for(i = 0; i < len; i +=4)
      {
        flash_word_program(flash_addr+i, *(uint32_t *)(buf + i));
      }
      flash_spim_lock();
      break;
  }
  return USB_OK;
}

/**
  * @brief  disk capacity
  * @param  lun: logical units number
  * @param  blk_nbr: pointer to number of block
  * @param  blk_size: pointer to block size
  * @retval status of usb_sts_type
  */
usb_sts_type msc_disk_capacity(uint8_t lun, uint32_t *blk_nbr, uint32_t *blk_size)
{
  uint32_t flash_s = *((uint32_t *)0x1FFFF7E0);
  msc_flash_size = (flash_s << 10) - (USB_FLASH_ADDR_OFFSET - FLASH_BASE);

  if(flash_s < 256)
  {
    sector_size = SECTOR_SIZE_1K;
  }
  else
  {
    sector_size = SECTOR_SIZE_2K;
  }

  switch(lun)
  {
    case INTERNAL_FLASH_LUN:
      *blk_nbr = msc_flash_size / sector_size;
      *blk_size = sector_size;
      break;
    case SPI_FLASH_LUN:
      *blk_nbr = SPIFLASH_SIZE / SECTOR_SIZE_4K;
      *blk_size = SECTOR_SIZE_4K;
      break;
    case SD_LUN:
      break;
    default:
      break;
  }
  return USB_OK;
}

/**
  * @}
  */

/**
  * @}
  */
