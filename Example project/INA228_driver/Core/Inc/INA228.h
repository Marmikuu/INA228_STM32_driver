/**
*
*	INA 228 sensor driver
*	Marcin Mikula
*	github.com/Marmikuu
*
*/
#ifndef _INA228_H
#define _INA228_H


#ifdef STM32L4
#include "stm32l4xx_hal.h" ///Import HAL library for STM32L476 Nucleo
#else
			////////// Replace with your MCU's HAL library
#endif

#include "main.h"

//********** REGISTERS Adresses **************
/// (Reg. names according to INA228 datasheet)

/// Config registers
#define CONFIG 0x00 // Configuration 		 16 bit size
#define ADC_CONFIG 0x01 // ADC configuration 16 bit
#define SHUNT_CAL 0x02 /// Shunt Calibration  16 bit
#define SHUNT_TEMPCO 0x03 // Shunt Temperature Coefficient 16

//// Measurement registers
#define VSHUNT 0x04 /// Shunt Voltage Measurement 24 bit // + or -
#define VBUS 0x05 /// Bus Voltage Measurement 24 bit /// + or -
#define DIETEMP 0x06 /// Temperature Measurement 16b /// + or -
#define CURRENT 0x07 /// Current Result 24b
#define POWER 0x08 // Power Result 24b
#define ENERGY 0x09 /// Energy Result 40b
#define CHARGE 0x0A // Charge Result 40b

//// Alert and advanced config registers
#define DIAG_ALRT 0x0B //// Diagnostic Flags and Alert 16b
#define SOVL 0x0C /// Shunt Ovrvoltage Threshold 16b
#define SUVL 0x0D /// Shunt Undervoltage Threshold 16b
#define BOVL 0x0E // Bus Overvoltage Threshold 16b
#define BUVL 0x0F /// Bus Undervoltage Threshold 16b

#define TEMP_LIMIT 0x10 /// Temperature Over-Limit Threshold 16b
#define PWR_LIMIT 0x11 // Power Over-Limit Threshold 16b
#define MANUFACTURER_ID 0x3E /// Manufacturer ID 16b
#define DEVICE_ID 0x3F /// Device ID 16b


// ******* OPERATING MODE - ADC_CONFIG register (bits 15-12)

#define MODE_POWER_OFF 0x00 // Shutdown

//// Triggered
#define MODE_TRIG_BUS 0x01 // Triggered bus voltage
#define MODE_TRIG_SHUNT 0x02
#define MODE_TRIG_SHUNT_BUS 0x03
#define MODE_TRIG_TEMP 0x04 /// Triggered temperature
#define MODE_TRIG_TEMP_BUS 0x05
#define MODE_TRIG_TEMP_SHUNT 0x06
#define MODE_TRIG_TEMP_SHUNT_BUS 0x07 // Triggered temp + bus + shunt

/// Continuous
#define MODE_CONT_BUS 0x09 // Continuous bus voltage
#define MODE_CONT_SHUNT 0x0A
#define MODE_CONT_SHUNT_BUS 0x0B
#define MODE_CONT_TEMP 0x0C // Continuous temperature
#define MODE_CONT_TEMP_BUS 0x0D
#define MODE_CONT_TEMP_SHUNT 0x0E
#define MODE_CONT_TEMP_SHUNT_BUS 0x0F // Continuous temp+shunt+bus


///************** ADC CONVERSION TIME ********************
/// ADC_CONFIG register (bits 11-9 VBUS, 8-6 VSHUNT, 5-3 TEMP)

// Conversion time for VBUS and VSHUNT and temperature
// Can be set separately for each measurement

#define CONV_TIME_50us 0x00 // 50 µs
#define CONV_TIME_84us 0x01 // 84 µs
#define CONV_TIME_150us 0x02 // 150 µs
#define CONV_TIME_280us 0x03 // 280 µs
#define CONV_TIME_540us 0x04 // 540 µs
#define CONV_TIME_1052us 0x05 // 1052 µs
#define CONV_TIME_2074us 0x06 // 2074 µs
#define CONV_TIME_4120us 0x07 // 4120 µs


/// ****************  ADC AVERAGING **********************
#define AVG_1 0x00 /// 1 sample
#define AVG_4 0x01 /// 4 samples
#define AVG_16 0x02
#define AVG_64 0x03
#define AVG_128 0x04
#define AVG_256 0x05
#define AVG_512 0x06
#define AVG_1024 0x07 /// 1024 samples

//*************  ADC RANGE ******************
#define ADC_RANGE_0 // +-163.84mV, 312.5 nV/LSB
#define ADC_RANGE_1 // +-40.96mV, 78.125 nV/LSB

///*********** SHUNT VALUE ******************
#define R_SHUNT_DEFAULT 0.015 // 0.015 ohm default Adaruit shunt

typedef struct
{
    I2C_HandleTypeDef *hi2c;
    uint16_t i2c_addr;
    uint16_t averaging;
    uint16_t conv_time_vbus_vshunt;
    uint16_t conv_time_temp;
    uint16_t mode;
    float R_shunt_Ohms;
    float max_current_Amps;

} INA228_Handle_t;

HAL_StatusTypeDef INA228_Init( INA228_Handle_t *hina228, I2C_HandleTypeDef *hi2c, uint16_t i2c_addr, uint16_t averaging, uint16_t conv_time_vbus_vshunt,uint16_t conv_time_temp, uint16_t mode, float R_shunt_Ohms, float max_current_Amps);
HAL_StatusTypeDef INA228_Reset(INA228_Handle_t *hina228);
HAL_StatusTypeDef INA228_Config(INA228_Handle_t *hina228);

HAL_StatusTypeDef INA228_ReadShuntVoltage(INA228_Handle_t *hina228, float *shunt_voltage);
HAL_StatusTypeDef INA228_ReadCurrent(INA228_Handle_t *hina228, float *current);
HAL_StatusTypeDef INA228_ReadPower(INA228_Handle_t *hina228, float *power);
HAL_StatusTypeDef INA228_ReadBusVoltage(INA228_Handle_t *hina228, float *bus_voltage);


//// DIAGNOSTIC FLAGS (To be continued)
#endif
