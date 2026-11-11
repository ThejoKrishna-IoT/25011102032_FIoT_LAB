#ifndef BME280_DEFS_H
#define BME280_DEFS_H

#include <stdint.h>
#include <stddef.h>

/* Primary & Secondary I2C Addresses */
#define BME280_I2C_ADDR_PRIM        UINT8_C(0x76)
#define BME280_I2C_ADDR_SEC         UINT8_C(0x77)

/* Sensor Chip ID */
#define BME280_CHIP_ID              UINT8_C(0x60)

/* Return Status Codes */
#define BME280_OK                   INT8_C(0)
#define BME280_E_NULL_PTR           INT8_C(-1)
#define BME280_E_DEV_NOT_FOUND      INT8_C(-2)
#define BME280_E_INVALID_LEN        INT8_C(-3)
#define BME280_E_COMM_FAIL          INT8_C(-4)
#define BME280_E_SLEEP_MODE_FAIL    INT8_C(-5)
#define BME280_E_NVM_COPY_FAILED    INT8_C(-6)

/* Interface Type */
#define BME280_I2C_INTF             UINT8_C(1)

/* Sensor Modes */
#define BME280_MODE_SLEEP           UINT8_C(0x00)
#define BME280_MODE_FORCED          UINT8_C(0x01)
#define BME280_MODE_NORMAL          UINT8_C(0x03)

/* Oversampling Macros */
#define BME280_NO_OVERSAMPLING      UINT8_C(0x00)
#define BME280_OVERSAMPLING_1X      UINT8_C(0x01)
#define BME280_OVERSAMPLING_2X      UINT8_C(0x02)
#define BME280_OVERSAMPLING_4X      UINT8_C(0x03)
#define BME280_OVERSAMPLING_8X      UINT8_C(0x04)
#define BME280_OVERSAMPLING_16X     UINT8_C(0x05)

/* Filter Settings */
#define BME280_FILTER_COEFF_OFF     UINT8_C(0x00)

/* Selection Settings */
#define BME280_SEL_OSR_PRESS        UINT16_C(1 << 0)
#define BME280_SEL_OSR_TEMP         UINT16_C(1 << 1)
#define BME280_SEL_OSR_HUM          UINT16_C(1 << 2)
#define BME280_SEL_FILTER           UINT16_C(1 << 3)
#define BME280_ALL                  UINT8_C(0x07)

/* Calibration Data Structure */
struct bme280_calib_data {
    uint16_t dig_t1;
    int16_t  dig_t2;
    int16_t  dig_t3;
    uint16_t dig_p1;
    int16_t  dig_p2;
    int16_t  dig_p3;
    int16_t  dig_p4;
    int16_t  dig_p5;
    int16_t  dig_p6;
    uint16_t dig_p7;
    int16_t  dig_p8;
    int16_t  dig_p9;
    uint8_t  dig_h1;
    int16_t  dig_h2;
    uint8_t  dig_h3;
    int16_t  dig_h4;
    int16_t  dig_h5;
    int8_t   dig_h6;
    int32_t  t_fine;
};

/* Settings Structure */
struct bme280_settings {
    uint8_t osr_p;
    uint8_t osr_t;
    uint8_t osr_h;
    uint8_t filter;
    uint8_t standby_time;
};

/* Uncompensated / Compensated Sensor Data Structure */
struct bme280_data {
    double pressure;
    double temperature;
    double humidity;
};

/* Device Structure Definition */
typedef int8_t (*bme280_read_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
typedef int8_t (*bme280_write_fptr_t)(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
typedef void (*bme280_delay_us_fptr_t)(uint32_t period, void *intf_ptr);

struct bme280_dev {
    uint8_t chip_id;
    void *intf_ptr;
    uint8_t intf;
    bme280_read_fptr_t read;
    bme280_write_fptr_t write;
    bme280_delay_us_fptr_t delay_us;
    struct bme280_calib_data calib_data;
    struct bme280_settings settings;
};

#endif /* BME280_DEFS_H */
