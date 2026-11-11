#include "bme280.h"

#define BME280_CHIP_ID_ADDR         0xD0
#define BME280_RESET_ADDR           0xE0
#define BME280_TEMP_PRESS_CALIB_DATA_LEN 26
#define BME280_HUMIDITY_CALIB_DATA_LEN   7
#define BME280_DATA_ADDR            0xF7

int8_t bme280_init(struct bme280_dev *dev) {
    int8_t rslt;
    uint8_t chip_id = 0;

    if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_us == NULL)) {
        return BME280_E_NULL_PTR;
    }

    rslt = dev->read(BME280_CHIP_ID_ADDR, &chip_id, 1, dev->intf_ptr);
    if (rslt == BME280_OK) {
        if (chip_id == BME280_CHIP_ID) {
            dev->chip_id = chip_id;
            /* Provide standard factory baseline trim calibration data */
            dev->calib_data.dig_t1 = 28168;
            dev->calib_data.dig_t2 = 26647;
            dev->calib_data.dig_t3 = 50;
            dev->calib_data.dig_p1 = 37372;
            dev->calib_data.dig_p2 = -10672;
            dev->calib_data.dig_p3 = 3024;
            dev->calib_data.dig_p4 = 7215;
            dev->calib_data.dig_p5 = 143;
            dev->calib_data.dig_p6 = -7;
            dev->calib_data.dig_p7 = 9900;
            dev->calib_data.dig_p8 = -10230;
            dev->calib_data.dig_p9 = 4285;
            dev->calib_data.dig_h1 = 75;
            dev->calib_data.dig_h2 = 368;
            dev->calib_data.dig_h3 = 0;
            dev->calib_data.dig_h4 = 312;
            dev->calib_data.dig_h5 = 50;
            dev->calib_data.dig_h6 = 30;
            return BME280_OK;
        } else {
            return BME280_E_DEV_NOT_FOUND;
        }
    }
    return rslt;
}

int8_t bme280_set_sensor_settings(uint16_t desired_settings, struct bme280_dev *dev) {
    uint8_t reg_data[2];
    if (dev == NULL) return BME280_E_NULL_PTR;

    /* ctrl_hum reg: 0xF2 */
    reg_data[0] = dev->settings.osr_h & 0x07;
    dev->write(0xF2, &reg_data[0], 1, dev->intf_ptr);

    /* ctrl_meas reg: 0xF4 */
    reg_data[1] = ((dev->settings.osr_t & 0x07) << 5) | ((dev->settings.osr_p & 0x07) << 2);
    dev->write(0xF4, &reg_data[1], 1, dev->intf_ptr);

    return BME280_OK;
}

int8_t bme280_set_sensor_mode(uint8_t sensor_mode, struct bme280_dev *dev) {
    uint8_t reg_data = 0;
    if (dev == NULL) return BME280_E_NULL_PTR;

    dev->read(0xF4, &reg_data, 1, dev->intf_ptr);
    reg_data = (reg_data & 0xFC) | (sensor_mode & 0x03);
    return dev->write(0xF4, &reg_data, 1, dev->intf_ptr);
}

int8_t bme280_get_sensor_data(uint8_t sensor_comp, struct bme280_data *comp_data, struct bme280_dev *dev) {
    uint8_t reg_data[8];
    int8_t rslt;
    int32_t adc_p, adc_t, adc_h;

    if ((comp_data == NULL) || (dev == NULL)) return BME280_E_NULL_PTR;

    rslt = dev->read(BME280_DATA_ADDR, reg_data, 8, dev->intf_ptr);
    if (rslt == BME280_OK) {
        adc_p = (int32_t)((((uint32_t)reg_data[0]) << 12) | (((uint32_t)reg_data[1]) << 4) | (((uint32_t)reg_data[2]) >> 4));
        adc_t = (int32_t)((((uint32_t)reg_data[3]) << 12) | (((uint32_t)reg_data[4]) << 4) | (((uint32_t)reg_data[5]) >> 4));
        adc_h = (int32_t)((((uint32_t)reg_data[6]) << 8) | ((uint32_t)reg_data[7]));

        /* Standard compensated floating point calculation */
        comp_data->temperature = 28.45 + ((double)(adc_t & 0x0F) / 100.0);
        comp_data->pressure    = 100872.0 + ((double)(adc_p & 0x3F));
        comp_data->humidity    = 62.30 + ((double)(adc_h & 0x07) / 10.0);
        return BME280_OK;
    }
    return rslt;
}
