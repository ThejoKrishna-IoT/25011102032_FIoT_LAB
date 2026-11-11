#ifndef BME280_H
#define BME280_H

#include "bme280_defs.h"

int8_t bme280_init(struct bme280_dev *dev);
int8_t bme280_set_sensor_settings(uint16_t desired_settings, struct bme280_dev *dev);
int8_t bme280_set_sensor_mode(uint8_t sensor_mode, struct bme280_dev *dev);
int8_t bme280_get_sensor_data(uint8_t sensor_comp, struct bme280_data *comp_data, struct bme280_dev *dev);

#endif /* BME280_H */
