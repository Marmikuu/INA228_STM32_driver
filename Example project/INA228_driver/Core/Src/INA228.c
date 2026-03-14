/**
*
*	Created on: Nov 4, 2025
*	Modified on: Mar 14 2026
*
*	INA 228 sensor driver for STM32 MCU
*
*	Author: Marcin Mikula
*	github.com/Marmikuu
*
*	version 1.2
*
*/

#include "INA228.h"

///// replace stdio.h with FreeRTOS-friendly mpaland's lightweight printf.h implementation if needed
#include <stdio.h>


HAL_StatusTypeDef INA228_Reset(INA228_Handle_t *hina228)
{
    uint16_t reset = (1<<15);
    uint8_t data[2];

    data[0] = reset >> 8;
    data[1] = (reset & 0xff);

    if (HAL_I2C_Mem_Write(hina228->hi2c, ((hina228->i2c_addr)<<1), CONFIG, I2C_MEMADD_SIZE_8BIT, data, 2, 1000)!=HAL_OK)
    {
        return HAL_ERROR;
    }

    else
    {
        return HAL_OK;
    }
}


HAL_StatusTypeDef INA228_Config(INA228_Handle_t *hina228)
{
	if (INA228_Reset(hina228)!=HAL_OK)
	{
		return HAL_ERROR;
	}

	float current_LSB = hina228->max_current_Amps/524288.f;


	uint16_t shunt_cal_value = (uint16_t)((13107.2f * current_LSB) * (1e6f * hina228->R_shunt_Ohms));
    uint8_t data[2];
    data[0] = shunt_cal_value >> 8; /// MSB first - big endian
    data[1] = shunt_cal_value & 0xff;


    //// Write SHUNT_CAL register
    if (HAL_I2C_Mem_Write(hina228->hi2c, ((hina228->i2c_addr)<<1), SHUNT_CAL, I2C_MEMADD_SIZE_8BIT, data, 2, 1000) != HAL_OK)
    {
        return HAL_ERROR;
    }

    uint16_t config_value = (hina228 -> mode << 12) | /// Operating mode
    		(hina228->conv_time_vbus_vshunt << 9) |		/// vbus conversion time
    		(hina228->conv_time_vbus_vshunt << 6)  | /// vshunt conv. time
			(hina228->conv_time_temp <<3)     |   ///// 	temperature conv. time
			(hina228->averaging);

    data[0] = config_value >> 8; /// MSB first - big endian
    data[1] = config_value & 0xff;

    //// Write ADC_CONFIG register
    if (HAL_I2C_Mem_Write(hina228->hi2c, ((hina228->i2c_addr)<<1), ADC_CONFIG, I2C_MEMADD_SIZE_8BIT, data, 2, 1000) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;

}



HAL_StatusTypeDef INA228_Init( INA228_Handle_t *hina228, I2C_HandleTypeDef *hi2c, uint16_t i2c_addr, uint16_t averaging, uint16_t conv_time_vbus_vshunt, uint16_t conv_time_temp, uint16_t mode, float R_shunt_Ohms, float max_current_Amps)
{
	hina228->hi2c = hi2c;
	hina228 ->i2c_addr = i2c_addr;
	hina228->averaging = averaging;
	hina228 ->conv_time_vbus_vshunt = conv_time_vbus_vshunt;
	hina228 -> conv_time_temp = conv_time_temp;
	hina228 ->mode = mode;
	hina228 ->R_shunt_Ohms = R_shunt_Ohms;
	hina228 ->max_current_Amps = max_current_Amps;

    if (INA228_Config(hina228) != HAL_OK)
    {
    	return HAL_ERROR;
    }
    else
    {
		return HAL_OK;
	}

}


HAL_StatusTypeDef INA228_ReadShuntVoltage(INA228_Handle_t *hina228, float *voltage)
{
    uint16_t vShuntRaw;
    uint8_t data[2];

    // Read shunt voltage register
    if (HAL_I2C_Mem_Read(hina228->hi2c, ((hina228->i2c_addr)<<1), VSHUNT,I2C_MEMADD_SIZE_8BIT , data, 2, 1000) != HAL_OK)
    {
      return HAL_ERROR;
    }
    vShuntRaw = (data[0] << 8) | data[1];

	float vShunt_LSB = 312.5e-9f;
    // Calculate shunt voltage in volts
    *voltage = (float)vShuntRaw * vShunt_LSB;

    return HAL_OK;
}




HAL_StatusTypeDef INA228_ReadCurrent(INA228_Handle_t *hina228, float *current)
{
    uint32_t current_raw; /// CURRENT register readings
    int32_t current_signed;
    uint8_t data[3];

    // Read CURRENTT register and get current_raw value
    if (HAL_I2C_Mem_Read(hina228->hi2c,((hina228->i2c_addr)<<1), CURRENT, I2C_MEMADD_SIZE_8BIT , data, 3, 1000) != HAL_OK)
    {
        return HAL_ERROR;
    }

    current_raw = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | ((uint32_t)data[2]); /// Most significant byte first
    current_raw >>= 4;

    if (current_raw &  (1<<19))
    	current_signed = current_raw | 0xfff00000;
    else
    	current_signed = current_raw;

    float current_LSB = hina228->max_current_Amps/524288.f;

    //// Calculate current in Amps
    *current = (float)current_signed * current_LSB;

    return HAL_OK;
}



HAL_StatusTypeDef INA228_ReadPower(INA228_Handle_t *hina228, float *power)
{
    uint32_t power_raw;
    uint8_t data[3];

    // Read power register
    if (HAL_I2C_Mem_Read(hina228->hi2c, ((hina228->i2c_addr)<<1), POWER, I2C_MEMADD_SIZE_8BIT , data, 3, 1000) != HAL_OK)
    {
        return HAL_ERROR;
    }

    power_raw = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | ((uint32_t)data[2]) ;

    // Calculate power in Watts
	float current_LSB = hina228->max_current_Amps/524288.f;

    *power = (float)power_raw * current_LSB * 3.2f;

    return HAL_OK;
}


HAL_StatusTypeDef INA228_ReadBusVoltage(INA228_Handle_t *hina228, float *voltage)
{
    uint32_t vBus_raw;
    int32_t vBus_signed;
    uint8_t data[3];

    // Read bus voltage register
    if (HAL_I2C_Mem_Read(hina228->hi2c, ((hina228->i2c_addr)<<1), VBUS, I2C_MEMADD_SIZE_8BIT , data, 3, 1000) != HAL_OK)
    {
        return HAL_ERROR;
    }

    vBus_raw = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | ((uint32_t)data[2]); /// Most significant byte first

    vBus_raw >>= 4;

    if (vBus_raw &  (1<<19))
    	vBus_signed = vBus_raw | 0xfff00000;
    else
    	vBus_signed = vBus_raw;


    float vBus_LSB = 195.3125e-6;

    // Calculate bus voltage in Volts
    *voltage = (float)vBus_signed * vBus_LSB;

    return HAL_OK;
}



HAL_StatusTypeDef INA228_ReadDieTemp(INA228_Handle_t *hina228, float *temp)
{
	uint16_t temp_raw;
	int16_t temp_signed;
	uint8_t data[2];

	if (HAL_I2C_Mem_Read(hina228->hi2c, (hina228->i2c_addr)<<1,DIETEMP , I2C_MEMADD_SIZE_8BIT, data, 2, 1000) != HAL_OK)
	{
		return HAL_ERROR;
	}

	temp_raw = ((uint16_t)data[0] << 8 | (uint16_t)data[1]); /// MSB first
	temp_signed = (int16_t)temp_raw;

	float temp_LSB = 0.0078125f; ////7.8125 m°C/LSB = 0.0078125 °C/LSB

	*temp = (float)temp_signed * temp_LSB;

	return HAL_OK;
}
