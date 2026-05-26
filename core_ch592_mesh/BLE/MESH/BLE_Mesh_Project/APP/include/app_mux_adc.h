#ifndef APP_MUX_ADC_H
#define APP_MUX_ADC_H

#include <stdint.h>
#include "CH59x_adc.h"

/* PB13/PB14: analog mux address lines; PB15: mux enable (adjust polarity below if needed) */
#define MUX_ADC_SEL_PIN_LO       ((uint32_t)(1 << 13))
#define MUX_ADC_SEL_PIN_HI       ((uint32_t)(1 << 14))
#define MUX_ADC_EN_PIN           ((uint32_t)(1 << 15))

/* PA8 = AIN12 on CH592; use datasheet ADC channel column if reading is wrong */
#ifndef MUX_ADC_EXT_CHANNEL
#define MUX_ADC_EXT_CHANNEL      CH_EXTIN_12
#endif

/* 1: EN high = mux on; 0: EN low = mux on (many CD405x / similar use /E active-low) */
#ifndef MUX_ADC_EN_ACTIVE_HIGH
#define MUX_ADC_EN_ACTIVE_HIGH   0
#endif

#define MUX_ADC_CHANNEL_NUM      4

/*
 * Debug single mux channel: keep (-1) for normal 4-CH scan.
 * Set to 0..3 to fix mux address lines to one channel only (others in g_mux_adc_raw cleared).
 */
#ifndef MUX_ADC_DEBUG_FIXED_CH
#define MUX_ADC_DEBUG_FIXED_CH   -1
#endif

/* TMOS task: periodic scan of 4 mux channels into g_mux_adc_raw */
#define MUX_ADC_SAMPLE_EVT          (0x0001u)
#ifndef MUX_ADC_SAMPLE_PERIOD_MS
#define MUX_ADC_SAMPLE_PERIOD_MS    50u
#endif

/* After mux GPIO change, wait this long (TMOS timer, not DelayUs) before ADC sample */
#ifndef MUX_ADC_SETTLE_MS
#define MUX_ADC_SETTLE_MS           2u
#endif

extern uint8_t MuxADC_TaskID;
extern uint16_t g_mux_adc_raw[MUX_ADC_CHANNEL_NUM];

void MuxADC_Init(void);
void MuxADC_TaskStart(void);
void MuxADC_SetMuxEnable(uint8_t on);
void MuxADC_SelectChannel(uint8_t ch);
uint16_t MuxADC_ReadSingleRaw(uint8_t ch);
void MuxADC_ReadAllChannels(void); /* sync: discard conversions only, no timed settle */

#endif
