/*
 * LTC6811.h
 *
 *  Created on: Nov 8, 2025
 *      Author: racin
 */

#ifndef INC_LTC6811_H_
#define INC_LTC6811_H_

#include "main.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern SPI_HandleTypeDef hspi3;

static const uint16_t pec15Table[256] = {
										  0x0000,0xC599,0xCEAB,0x0B32,0xD8CF,0x1D56,0x1664,0xD3FD,
										  0xF407,0x319E,0x3AAC,0xFF35,0x2CC8,0xE951,0xE263,0x27FA,
										  0xAD97,0x680E,0x633C,0xA6A5,0x7558,0xB0C1,0xBBF3,0x7E6A,
										  0x5990,0x9C09,0x973B,0x52A2,0x815F,0x44C6,0x4FF4,0x8A6D,
										  0x5B2E,0x9EB7,0x9585,0x501C,0x83E1,0x4678,0x4D4A,0x88D3,
										  0xAF29,0x6AB0,0x6182,0xA41B,0x77E6,0xB27F,0xB94D,0x7CD4,
										  0xF6B9,0x3320,0x3812,0xFD8B,0x2E76,0xEBEF,0xE0DD,0x2544,
										  0x02BE,0xC727,0xCC15,0x098C,0xDA71,0x1FE8,0x14DA,0xD143,
										  0xF3C5,0x365C,0x3D6E,0xF8F7,0x2B0A,0xEE93,0xE5A1,0x2038,
										  0x07C2,0xC25B,0xC969,0x0CF0,0xDF0D,0x1A94,0x11A6,0xD43F,
										  0x5E52,0x9BCB,0x90F9,0x5560,0x869D,0x4304,0x4836,0x8DAF,
										  0xAA55,0x6FCC,0x64FE,0xA167,0x729A,0xB703,0xBC31,0x79A8,
										  0xA8EB,0x6D72,0x6640,0xA3D9,0x7024,0xB5BD,0xBE8F,0x7B16,
										  0x5CEC,0x9975,0x9247,0x57DE,0x8423,0x41BA,0x4A88,0x8F11,
										  0x057C,0xC0E5,0xCBD7,0x0E4E,0xDDB3,0x182A,0x1318,0xD681,
										  0xF17B,0x34E2,0x3FD0,0xFA49,0x29B4,0xEC2D,0xE71F,0x2286,
										  0xA213,0x678A,0x6CB8,0xA921,0x7ADC,0xBF45,0xB477,0x71EE,
										  0x5614,0x938D,0x98BF,0x5D26,0x8EDB,0x4B42,0x4070,0x85E9,
										  0x0F84,0xCA1D,0xC12F,0x04B6,0xD74B,0x12D2,0x19E0,0xDC79,
										  0xFB83,0x3E1A,0x3528,0xF0B1,0x234C,0xE6D5,0xEDE7,0x287E,
										  0xF93D,0x3CA4,0x3796,0xF20F,0x21F2,0xE46B,0xEF59,0x2AC0,
										  0x0D3A,0xC8A3,0xC391,0x0608,0xD5F5,0x106C,0x1B5E,0xDEC7,
										  0x54AA,0x9133,0x9A01,0x5F98,0x8C65,0x49FC,0x42CE,0x8757,
										  0xA0AD,0x6534,0x6E06,0xAB9F,0x7862,0xBDFB,0xB6C9,0x7350,
										  0x51D6,0x944F,0x9F7D,0x5AE4,0x8919,0x4C80,0x47B2,0x822B,
										  0xA5D1,0x6048,0x6B7A,0xAEE3,0x7D1E,0xB887,0xB3B5,0x762C,
										  0xFC41,0x39D8,0x32EA,0xF773,0x248E,0xE117,0xEA25,0x2FBC,
										  0x0846,0xCDDF,0xC6ED,0x0374,0xD089,0x1510,0x1E22,0xDBBB,
										  0x0AF8,0xCF61,0xC453,0x01CA,0xD237,0x17AE,0x1C9C,0xD905,
										  0xFEFF,0x3B66,0x3054,0xF5CD,0x2630,0xE3A9,0xE89B,0x2D02,
										  0xA76F,0x62F6,0x69C4,0xAC5D,0x7FA0,0xBA39,0xB10B,0x7492,
										  0x5368,0x96F1,0x9DC3,0x585A,0x8BA7,0x4E3E,0x450C,0x8095
};
/**
 * @defgroup SCTL_DEFINE
 * @{
 */
#define SCTL_ON 0x08;
#define SCTL_OFF 0x00;
/** @}*/

/**
 * @defgroup MD_DEFINE
 * @{
 */
#define MD_SPEC 0
#define MD_FAST 1
#define MD_NORMAL 2
#define MD_FILTERED 3
/** @}*/

/**
 * @defgroup CELL_CH_DEFINE
 * @{
 */
#define CELL_CH_ALL 0
#define CELL_CH_1and7 1
#define CELL_CH_2and8 2
#define CELL_CH_3and9 3
#define CELL_CH_4and10 4
#define CELL_CH_5and11 5
#define CELL_CH_6and12 6
/** @}*/

/**
 * @defgroup AUX_CHG_DEFINE
 * @{
 */
#define AUX_CH_ALL 0
#define AUX_CH_GPIO1 1
#define AUX_CH_GPIO2 2
#define AUX_CH_GPIO3 3
#define AUX_CH_GPIO4 4
#define AUX_CH_GPIO5 5
#define AUX_CH_VREF2 6
/** @}*/

/**
 * @defgroup ST_DEFINE
 * @{
 */
#define ST_1 1 //Selftest 1
#define ST_2 2 //Selftest 2
/** @}*/

/**
 * @defgroup DCP_DEFINE
 * @{
 */
#define DCP_DISABLED 0 //Discharge during measurement not permitted
#define DCP_ENABLED 1  //Discharge during measurement permitted
/** @}*/
/**
 * @defgroup CHST
 * @{
 */
#define CHST_all 0
#define CHST_SC 1
#define CHST_ITMP 2
#define CHST_VA 3
#define CHST_VD 4

void LTC6811_initialize(void);
void set_adc(uint8_t MD, uint8_t DCP, uint8_t CH, uint8_t CHG, uint8_t CHST);
void set_selftest(uint8_t MD, uint8_t ST);
void LTC6811_adcv(void);
void LTC6811_adax(void);
void LTC6811_cvst(void);
void LTC6811_axst(void);
void LTC6811_adstat(void);
uint8_t LTC6811_rdADSTAT(uint8_t reg, uint8_t *data);
int8_t LTC6811_rdstatb(uint8_t total_ic, uint16_t OV_flag[] ,uint16_t UV_flag[], uint8_t r_statb[][6]);
uint8_t LTC6811_rdcv(uint8_t reg, uint16_t cell_codes[][12]);
int8_t LTC6811_rdstat(uint8_t addr, uint8_t *data);
void LTC6811_rdcv_reg(uint8_t reg, uint8_t *data);
uint8_t LTC6811_rdaux(uint8_t reg, uint16_t aux_codes[][6]);
void LTC6811_rdaux_reg(uint8_t reg, uint8_t *data);
void LTC6811_rdstat_reg(uint8_t reg, uint8_t total_ic, uint8_t data[]);
void LTC6811_clrcell(void);
void LTC6811_clrstat(void);
void LTC6811_clraux(void);
void LTC6811_rdcfg(void);
void LTC6811_wrcfg(uint8_t config[][6]);
//int8_t LTC6811_rdcfg(uint8_t nIC, uint8_t r_config[][8]);
void LTC6811_wrpwm(uint8_t nIC, uint8_t dutyCycle[][12]);
void LTC6811_wrsctrl(uint8_t nIC, uint8_t sctl[][12]);
int8_t LTC6811_rdsctrl(uint8_t nIC, uint8_t r_sctl[][6]);
void wakeup_idle(void);
void wakeup_sleep(void);
uint16_t pec15_calc(uint8_t len, uint8_t *data);
void nopDelay(uint32_t cnt);

//int8_t LTC6811_rdstatb(uint8_t addr, uint8_t *data);


/**
 * @brief Write an array of bytes out of the SPI port (Used as abstraction for compatibility)
 *
 * @param len length of the data array
 * @param data pointer to the data array
 */

static inline void spi_write_array(uint8_t len, uint8_t data[])
{
  HAL_SPI_Transmit(&hspi3, data, len, HAL_MAX_DELAY);
}

/**
 * @brief Write and then read a data array over the SPI port
 *
 * @param tx_Data pointer to the data array to be transmitted
 * @param tx_len length of the data array to be transmitted
 * @param rx_data pointer to the data array to be received
 * @param rx_len length of the data array to be received
 */

static inline void spi_write_read(uint8_t tx_Data[], uint8_t tx_len,
                                  uint8_t *rx_data, uint8_t rx_len)
{
    uint8_t tmp_tx[64] = {0};
    uint8_t tmp_rx[64] = {0};

    // Kommando vorne rein
    memcpy(tmp_tx, tx_Data, tx_len);

    // Ein Transfer: cmd senden + dummy clocks für rx
    HAL_SPI_TransmitReceive(&hspi3, tmp_tx, tmp_rx, tx_len + rx_len, HAL_MAX_DELAY);

    // Antwort steht nach den cmd-bytes
    memcpy(rx_data, &tmp_rx[tx_len], rx_len);
}


/*static inline void spi_write_read(uint8_t tx_Data[], uint8_t tx_len, uint8_t *rx_data, uint8_t rx_len)
{
  HAL_SPI_Transmit(&hspi3, tx_Data, tx_len, HAL_MAX_DELAY);
  HAL_SPI_Receive(&hspi3, rx_data, rx_len, HAL_MAX_DELAY);
}
*/
/*static inline void spi_write_read(uint8_t tx_Data[], uint8_t tx_len, uint8_t *rx_data, uint8_t rx_len)
{
  HAL_SPI_Transmit(&hspi3, tx_Data, tx_len, HAL_MAX_DELAY);
  HAL_SPI_Receive(&hspi3, rx_data, rx_len, HAL_MAX_DELAY);

}
*/


#endif /* INC_LTC6811_H_ */
