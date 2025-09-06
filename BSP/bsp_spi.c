/*
 * @Author: fmy 921736537@qq.com
 * @Date: 2023-06-04 23:03:22
 * @LastEditors: fmy 921736537@qq.com
 * @LastEditTime: 2023-08-03 21:34:37
 * @FilePath: \GD32F303\BSP\bsp_spi.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "bsp_spi.h"

void GD_SPI1_Mspinit(void)
{
  /* SPI1_SCK(PB13), SPI1_MISO(PB14) and SPI2_MOSI(PB15) GPIO pin configuration */
  gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_13 | GPIO_PIN_15);
  gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_14);
}
void GD_SPI2_Mspinit(void)
{

  gpio_pin_remap_config(GPIO_SPI2_REMAP, ENABLE);
  /* SPI2_SCK(PC10), SPI2_MISO(PC11) and SPI2_MOSI(PC12) GPIO pin configuration */
  gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_10 | GPIO_PIN_12);
  gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_11);
}
void GD_SPI1_Init(void)
{

  spi_parameter_struct spi_init_s;
  GD_SPI1_Mspinit();
  spi_i2s_deinit(SPI1);
  /* SPI1 parameter config */
  spi_init_s.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
  spi_init_s.device_mode = SPI_MASTER;
  spi_init_s.frame_size = SPI_FRAMESIZE_16BIT;
  spi_init_s.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
  spi_init_s.nss = SPI_NSS_SOFT;
  spi_init_s.prescale = SPI_PSC_16;
  spi_init_s.endian = SPI_ENDIAN_MSB;
  spi_init(SPI1, &spi_init_s);

  /* enable SPI2 */
  spi_enable(SPI1);
}
void GD_SPI2_Init(void)
{

  spi_parameter_struct spi_init_s;
  GD_SPI2_Mspinit();
  spi_i2s_deinit(SPI2);
  /* SPI2 parameter config */
  spi_init_s.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
  spi_init_s.device_mode = SPI_MASTER;
  spi_init_s.frame_size = SPI_FRAMESIZE_16BIT;
  spi_init_s.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
  spi_init_s.nss = SPI_NSS_SOFT;
  spi_init_s.prescale = SPI_PSC_4;
  spi_init_s.endian = SPI_ENDIAN_MSB;
  spi_init(SPI2, &spi_init_s);

  /* enable SPI2 */
  spi_enable(SPI2);
}

uint16_t SPI_RW_byte(uint8_t spi_id, uint16_t byte)
{
  if (spi_id == 1)
  {
    /* loop while data register in not emplty */
    while (RESET == spi_i2s_flag_get(SPI1, SPI_FLAG_TBE))
      ;
    /* send byte through the SPI0 peripheral */
    spi_i2s_data_transmit(SPI1, byte);
    /* wait to receive a byte */
    while (RESET == spi_i2s_flag_get(SPI1, SPI_FLAG_RBNE))
      ;
    /* return the byte read from the SPI bus */
    return (spi_i2s_data_receive(SPI1));
  }
  else if (spi_id == 2)
  {
    /* loop while data register in not emplty */
    while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE))
      ;
    /* send byte through the SPI0 peripheral */
    spi_i2s_data_transmit(SPI2, byte);
    /* wait to receive a byte */
    while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE))
      ;
    /* return the byte read from the SPI bus */
    return (spi_i2s_data_receive(SPI2));
  }
  else
    return 0;
}
