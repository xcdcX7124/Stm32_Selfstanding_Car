#include "JoyStick.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali_scheme.h"

adc_oneshot_unit_handle_t ADC1_UnitHandler;

adc_cali_handle_t ADC1_cali_Channel3_Handler;
adc_cali_handle_t ADC1_cali_Channel4_Handler;
adc_cali_handle_t ADC1_cali_Channel5_Handler;
adc_cali_handle_t ADC1_cali_Channel6_Handler;

void JoyStick_Init(void)
{

    adc_oneshot_unit_init_cfg_t ADC1_cfgStucture = {
        // 配置ADC1
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
        .unit_id = ADC_UNIT_1,
    };

    adc_oneshot_new_unit(&ADC1_cfgStucture, &ADC1_UnitHandler);

    adc_oneshot_chan_cfg_t ADC_channel_cfg_Structure = {
        // 配置ADC通道
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    adc_oneshot_config_channel(ADC1_UnitHandler, ADC_CHANNEL_3, &ADC_channel_cfg_Structure); // 将配置应用于要使用的通道
    adc_oneshot_config_channel(ADC1_UnitHandler, ADC_CHANNEL_4, &ADC_channel_cfg_Structure);
    adc_oneshot_config_channel(ADC1_UnitHandler, ADC_CHANNEL_5, &ADC_channel_cfg_Structure);
    adc_oneshot_config_channel(ADC1_UnitHandler, ADC_CHANNEL_6, &ADC_channel_cfg_Structure);

    adc_cali_curve_fitting_config_t ADC1_Cali_Channel3_Structure = {
        // 配置各个通道的校准配置
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .chan = ADC_CHANNEL_3,
        .unit_id = ADC_UNIT_1,
    };
    adc_cali_curve_fitting_config_t ADC1_Cali_Channel4_Structure = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .chan = ADC_CHANNEL_4,
        .unit_id = ADC_UNIT_1,
    };
    adc_cali_curve_fitting_config_t ADC1_Cali_Channel5_Structure = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .chan = ADC_CHANNEL_5,
        .unit_id = ADC_UNIT_1,
    };
    adc_cali_curve_fitting_config_t ADC1_Cali_Channel6_Structure = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .chan = ADC_CHANNEL_6,
        .unit_id = ADC_UNIT_1,
    };

    adc_cali_create_scheme_curve_fitting(&ADC1_Cali_Channel3_Structure, &ADC1_cali_Channel3_Handler); // 创建每个通道的校准句柄

    adc_cali_create_scheme_curve_fitting(&ADC1_Cali_Channel4_Structure, &ADC1_cali_Channel4_Handler);

    adc_cali_create_scheme_curve_fitting(&ADC1_Cali_Channel5_Structure, &ADC1_cali_Channel5_Handler);

    adc_cali_create_scheme_curve_fitting(&ADC1_Cali_Channel6_Structure, &ADC1_cali_Channel6_Handler);
}

int JoyStick_GetLeftHorizon(void)
{ // 输出对应通道的校准值，校准后数值单位为mV，范围0到3100（ADC量程限制），返回值除以33转换为百分比值
    int Value = 0;
    adc_oneshot_get_calibrated_result(ADC1_UnitHandler, ADC1_cali_Channel3_Handler, ADC_CHANNEL_3, &Value);
    Value = Value / 31;
    if (Value >= 100)
        Value = 100;
    if (Value <= 0)
        Value = 0;
    if (Value >= 51 && Value <= 54)
        Value = 0;
    else if (Value < 51)
        Value = (Value - 51) / 0.51;
    else if (Value > 54)
        Value = (Value - 54) / 0.46;
    return -Value;
}

int JoyStick_GetLeftVertical(void)
{
    int Value = 0;
    adc_oneshot_get_calibrated_result(ADC1_UnitHandler, ADC1_cali_Channel4_Handler, ADC_CHANNEL_4, &Value);
    Value = Value / 31;
    if (Value >= 100)
        Value = 100;
    if (Value <= 0)
        Value = 0;
    if (Value >= 51 && Value <= 54)
        Value = 0;
    else if (Value < 51)
        Value = (Value - 51) / 0.51;
    else if (Value > 54)
        Value = (Value - 54) / 0.46;
    return -Value;
}

int JoyStick_GetRightHorizon(void)
{
    int Value = 0;
    adc_oneshot_get_calibrated_result(ADC1_UnitHandler, ADC1_cali_Channel5_Handler, ADC_CHANNEL_5, &Value);
    Value = Value / 31;
    if (Value >= 100)
        Value = 100;
    if (Value <= 0)
        Value = 0;
    if (Value >= 51 && Value <= 54)
        Value = 0;
    else if (Value < 51)
        Value = (Value - 51) / 0.51;
    else if (Value > 54)
        Value = (Value - 54) / 0.46;
    return -Value;
}

int JoyStick_GetRightVertical(void)
{
    int Value = 0;
    adc_oneshot_get_calibrated_result(ADC1_UnitHandler, ADC1_cali_Channel6_Handler, ADC_CHANNEL_6, &Value);
    Value = Value / 31;
    if (Value >= 100)
        Value = 100;
    if (Value <= 0)
        Value = 0;
    if (Value >= 51 && Value <= 54)
        Value = 0;
    else if (Value < 51)
        Value = (Value - 51) / 0.51;
    else if (Value > 54)
        Value = (Value - 54) / 0.46;
    return -Value;
}
