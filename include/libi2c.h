//
// libi2c.h
// Author: Francesco Mecatti
// I2C library and tools for ESP32 - ESP-IDF framework
//

#ifndef __LIBI2C_H
#define __LIBI2C_H

#include <driver/i2c.h>
#include <stdint.h>

/**
 * @brief initialize i2c communication
 * @param conf configuration struct
 * @return void
 */
static void i2c_init(struct i2c_config_t *conf);

/**
 * @brief read a series of len bytes and save them into an array
 * @param len number of bytes to read. Length of data array
 * @param data pointer to an array of uint8_t, where the data will be stored
 * @return void
 */
static void read_bytes(uint8_t len, uint8_t *data);

/**
 * @brief read one byte and return it
 * @return the read byte
 */
static uint8_t read_byte();

/**
 * @brief send a series of bytes to the slave
 * @param len data's length
 * @param data array of uint8_t
 * @return void
 */
static void write_bytes(uint8_t len, uint8_t *data);

/**
 * @brief send a byte to the slave
 * @param data byte to send
 * @return void
 */
static void write_byte(uint8_t data);

#endif  // __LIBI2C_H