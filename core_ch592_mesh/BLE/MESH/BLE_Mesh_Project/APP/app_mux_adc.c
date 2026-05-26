#include "CONFIG.h"
#include "CH59x_gpio.h"
#include "CH59x_adc.h"
#include "app_mux_adc.h"

uint8_t  MuxADC_TaskID = INVALID_TASK_ID;
uint16_t g_mux_adc_raw[MUX_ADC_CHANNEL_NUM];

#define MUX_ADC_SETTLE_EVT          (0x0002u)

#if !((MUX_ADC_DEBUG_FIXED_CH) >= 0 && (MUX_ADC_DEBUG_FIXED_CH) < MUX_ADC_CHANNEL_NUM)
static uint8_t mux_scan_idx;
#endif

static void adc_prepare_channel(void)
{
    ADC_ChannelCfg(MUX_ADC_EXT_CHANNEL);
}

/* After mux GPIO change: settle by dummy conversions (no DelayUs). */
static void adc_discard_conversions(uint8_t n)
{
    uint8_t i;

    adc_prepare_channel();
    for(i = 0; i < n; i++)
        ADC_ExcutSingleConver();
}

static void mux_apply_pins(uint8_t ch)
{
    uint8_t sel = ch & 3;

#if MUX_ADC_EN_ACTIVE_HIGH
    GPIOB_SetBits(MUX_ADC_EN_PIN);
#else
    GPIOB_ResetBits(MUX_ADC_EN_PIN);
#endif
    if(sel & 1)
        GPIOB_SetBits(MUX_ADC_SEL_PIN_LO);
    else
        GPIOB_ResetBits(MUX_ADC_SEL_PIN_LO);
    if(sel & 2)
        GPIOB_SetBits(MUX_ADC_SEL_PIN_HI);
    else
        GPIOB_ResetBits(MUX_ADC_SEL_PIN_HI);
}

void MuxADC_SetMuxEnable(uint8_t on)
{
#if MUX_ADC_EN_ACTIVE_HIGH
    if(on)
        GPIOB_SetBits(MUX_ADC_EN_PIN);
    else
        GPIOB_ResetBits(MUX_ADC_EN_PIN);
#else
    if(on)
        GPIOB_ResetBits(MUX_ADC_EN_PIN);
    else
        GPIOB_SetBits(MUX_ADC_EN_PIN);
#endif
}

void MuxADC_SelectChannel(uint8_t ch)
{
    mux_apply_pins(ch);
}

void MuxADC_Init(void)
{
    GPIOAGPPCfg(ENABLE, bAIN12);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_Floating);

    GPIOB_ModeCfg(MUX_ADC_SEL_PIN_LO | MUX_ADC_SEL_PIN_HI | MUX_ADC_EN_PIN, GPIO_ModeOut_PP_5mA);

    ADC_ExtSingleChSampInit(SampleFreq_4, ADC_PGA_0);

#if (MUX_ADC_DEBUG_FIXED_CH) >= 0 && (MUX_ADC_DEBUG_FIXED_CH) < MUX_ADC_CHANNEL_NUM
    mux_apply_pins((uint8_t)MUX_ADC_DEBUG_FIXED_CH);
#else
    mux_apply_pins(0);
#endif
    MuxADC_SetMuxEnable(1);
    adc_discard_conversions(4);
}

uint16_t MuxADC_ReadSingleRaw(uint8_t ch)
{
    mux_apply_pins(ch);
    adc_discard_conversions(2);
    adc_prepare_channel();
    return ADC_ExcutSingleConver();
}

void MuxADC_ReadAllChannels(void)
{
    uint8_t i;

    MuxADC_SetMuxEnable(1);
    for(i = 0; i < MUX_ADC_CHANNEL_NUM; i++)
        g_mux_adc_raw[i] = MuxADC_ReadSingleRaw(i);
}

static uint16_t mux_sample_after_settle(void)
{
    adc_discard_conversions(2);
    adc_prepare_channel();
    return ADC_ExcutSingleConver();
}

static uint16_t MuxADC_ProcessEvent(uint8_t task_id, uint16_t events)
{
    (void)task_id;

#if (MUX_ADC_DEBUG_FIXED_CH) >= 0 && (MUX_ADC_DEBUG_FIXED_CH) < MUX_ADC_CHANNEL_NUM

    if(events & MUX_ADC_SETTLE_EVT)
    {
        uint8_t i;
        uint16_t v = mux_sample_after_settle();

        for(i = 0; i < MUX_ADC_CHANNEL_NUM; i++)
            g_mux_adc_raw[i] = 0;
        g_mux_adc_raw[(uint8_t)MUX_ADC_DEBUG_FIXED_CH] = v;

        tmos_start_task(MuxADC_TaskID, MUX_ADC_SAMPLE_EVT, MUX_ADC_SAMPLE_PERIOD_MS);
        return events ^ MUX_ADC_SETTLE_EVT;
    }

    if(events & MUX_ADC_SAMPLE_EVT)
    {
        MuxADC_SetMuxEnable(1);
        mux_apply_pins((uint8_t)MUX_ADC_DEBUG_FIXED_CH);
        tmos_start_task(MuxADC_TaskID, MUX_ADC_SETTLE_EVT, MUX_ADC_SETTLE_MS);
        return events ^ MUX_ADC_SAMPLE_EVT;
    }

#else

    if(events & MUX_ADC_SETTLE_EVT)
    {
        g_mux_adc_raw[mux_scan_idx] = mux_sample_after_settle();

        mux_scan_idx++;
        if(mux_scan_idx < MUX_ADC_CHANNEL_NUM)
        {
            mux_apply_pins(mux_scan_idx);
            tmos_start_task(MuxADC_TaskID, MUX_ADC_SETTLE_EVT, MUX_ADC_SETTLE_MS);
        }
        else
        {
            tmos_start_task(MuxADC_TaskID, MUX_ADC_SAMPLE_EVT, MUX_ADC_SAMPLE_PERIOD_MS);
        }
        return events ^ MUX_ADC_SETTLE_EVT;
    }

    if(events & MUX_ADC_SAMPLE_EVT)
    {
        MuxADC_SetMuxEnable(1);
        mux_scan_idx = 0;
        mux_apply_pins(0);
        tmos_start_task(MuxADC_TaskID, MUX_ADC_SETTLE_EVT, MUX_ADC_SETTLE_MS);
        return events ^ MUX_ADC_SAMPLE_EVT;
    }

#endif
    return 0;
}

void MuxADC_TaskStart(void)
{
    MuxADC_TaskID = TMOS_ProcessEventRegister(MuxADC_ProcessEvent);
    tmos_start_task(MuxADC_TaskID, MUX_ADC_SAMPLE_EVT, MUX_ADC_SAMPLE_PERIOD_MS);
}
