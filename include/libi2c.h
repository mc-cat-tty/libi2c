/**
 * @file libi2c.h
 * @author Francesco Mecatti
 * @date 8 Mar 2021
 * @brief I2C library and tools for ESP32 - ESP-IDF framework
 */

#ifndef __LIBI2C_H
#define __LIBI2C_H

#include <driver/i2c.h>
#include <stdint.h>
#include <stdbool.h>

#define ACK_CHECK_EN    (0x01)
#define ACK_CHECK_DIS   (0x00)

#define ACK_VAL         (0x01)
#define NACK_VAL        (0x00)

typedef uint8_t u8;

/**
 * @struct i2c_config
 * @var i2c_config::esp_idf_conf
 *  ESP-IDF i2c configuration struct
 * @var i2c_config is_master
 *  true if this device is the communication's master
 * @var i2c_config::is_slave
 *  true if this device is the communication's slave
 * @see i2c_config_t
 */
struct i2c_config {
    i2c_config_t esp_idf_conf;
    bool is_master;
    bool is_slave;
};

/**
 * @brief i2c_config struct "constructor"
 */
#define INIT_I2C_CONFIG(X) (struct i2c_config X = {
    .esp_idf_conf = {
        .sda_pullup_en = GPIO_PULLUP_ENABLE;
        .scl_pullup_en = GPIO_PULLUP_ENABLE;
    },
    .is_master = false;
    .is_slave = false;
})

/**
 * @brief initialize i2c communication
 * @param conf configuration struct
 * @return void
 */
static void i2c_init(struct i2c_config *conf);

/**
 * @brief read a series of len bytes and save them into an array
 * @param len number of bytes to read. Length of data array
 * @param data pointer to an array of uint8_t, where the data will be stored
 * @return void
 */
static void i2c_read_bytes(u8 len, u8 *data);

/**
 * @brief read one byte and return it
 * @return the read byte
 */
static u8 i2c_read_byte();

/**
 * @brief send a series of bytes to the slave
 * @param len data's length
 * @param data array of uint8_t
 * @return void
 */
static void i2c_write_bytes(u8 len, u8 *data);

/**
 * @brief send a byte to the slave
 * @param data byte to send
 * @return void
 */
static void i2c_write_byte(u8 data);

/**
 * @brief select a register. Make the register "pointer" on the slave points to reg
 * @param reg register's address on the slave
 * @return void
 */
static void i2c_select_register(u8 reg);

#endif  // __LIBI2C_H