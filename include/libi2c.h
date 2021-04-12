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

#define ACK_VAL         (0x00)
#define NACK_VAL        (0x01)

#define WRITE_BIT       I2C_MASTER_WRITE
#define READ_BIT        I2C_MASTER_READ

#define NO_BUF          (0x00)
#define STD_BUF         (0xff)

#define PORT_0           I2C_NUM_0
#define PORT_1           I2C_NUM_1

#ifndef __cplusplus
#define noop            (void)0
#define assert(x)       ((!x || x <= 0) ? exit(1) : noop)
#endif


typedef uint8_t u8;
typedef u8 i2c_buf_size_t;
typedef u8 i2c_addr_t;

/**
 * @struct i2c_bus_t
 * @var i2c_config::conf
 *  ESP-IDF i2c configuration struct
 * @var i2c_config::port
 *  i2c port number
 * @var i2c_config_t::rx
 *  receive buffer size. Leave 0 for master. It doesn't need a buffer
 * @var i2c_config_t::tx
 *  transmission buffer size. Leave 0 for master. It doesn't need a buffer
 * @see i2c_config_t
 * @see i2c_port_t
 * @see i2c_buf_size_t
 */
struct i2c_bus_t {
    i2c_config_t conf;
    i2c_port_t port;
    i2c_buf_size_t rx;
    i2c_buf_size_t tx;
};

/**
 * @brief i2c_config struct "constructor" for master mode
 */
//#define INIT_I2C_BUS_CONFIG_DEFAULT(X) struct i2c_bus_t X = { .conf = {.mode = I2C_MODE_MASTER; .sda_io_num = 21; .scl_io_num = 22; .sda_pullup_en = GPIO_PULLUP_ENABLE; .scl_pullup_en = GPIO_PULLUP_ENABLE; .master.clk_speed = 400000;  /* 400 kHz */}; .port = PORT_0; .rx = NO_BUF;  /* Leave 0 for master. It doesn't need a buffer */ .tx = NO_BUF;  /* Leave 0 for master. It doesn't need a buffer */ };
#define init_i2c_bus_default_master() ((struct i2c_bus_t) { \
    .conf = { \
        .mode = I2C_MODE_MASTER, \
        .sda_io_num = 18, \
        .scl_io_num = 19, \
        .sda_pullup_en = GPIO_PULLUP_ENABLE, \
        .scl_pullup_en = GPIO_PULLUP_ENABLE, \
        .master.clk_speed = 400000,  /* 400 kHz */ \
        }, \
    .port = PORT_1, \
    .rx = NO_BUF,  /* Leave 0 for master. It doesn't need a buffer */ \
    .tx = NO_BUF,  /* Leave 0 for master. It doesn't need a buffer */ \
    })

/**
 * @brief i2c_config struct "constructor" for slave mode
 */
//#define INIT_I2C_BUS_CONFIG_DEFAULT(X) struct i2c_bus_t X = { .conf = {.mode = I2C_MODE_MASTER; .sda_io_num = 21; .scl_io_num = 22; .sda_pullup_en = GPIO_PULLUP_ENABLE; .scl_pullup_en = GPIO_PULLUP_ENABLE; .master.clk_speed = 400000;  /* 400 kHz */}; .port = PORT_0; .rx = NO_BUF;  /* Leave 0 for master. It doesn't need a buffer */ .tx = NO_BUF;  /* Leave 0 for master. It doesn't need a buffer */ };
#define init_i2c_bus_default_slave(addr) ((struct i2c_bus_t) { \
    .conf = { \
        .mode = I2C_MODE_SLAVE, \
        .sda_io_num = 4, \
        .scl_io_num = 5, \
        .sda_pullup_en = GPIO_PULLUP_ENABLE, \
        .scl_pullup_en = GPIO_PULLUP_ENABLE, \
        .slave.addr_10bit_en = 0, \
        .slave.slave_addr = addr, \
        }, \
    .port = PORT_0, \
    .rx = STD_BUF,  /* Leave 0 for master. It doesn't need a buffer */ \
    .tx = STD_BUF,  /* Leave 0 for master. It doesn't need a buffer */ \
    })

/**
 * @struct i2c_dev_handle_t
 * @var i2c_dev_handle_t::port
 *  i2c bus number to which the slave is connected
 * @var i2c_dev_handle_t::addr
 *  slave address (7-bit)
 * @see i2c_port_t
 * @see i2c_addr_t
 */
struct i2c_dev_handle_t {
    i2c_port_t port;
    i2c_addr_t addr;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief initialize i2c communication
 * @param conf configuration struct
 * @return void
 */
void i2c_init(const struct i2c_bus_t *conf);

/**
 * @brief read a series of len bytes and save them into an array. Only for master.
 * @param dev pointer to dev handle structure
 * @param size number of bytes to read. Length of data array
 * @param data pointer to an array of uint8_t, where the data will be stored
 * @return error code
 */
esp_err_t i2c_read_bytes(const struct i2c_dev_handle_t *dev, u8 *data, u8 size);

/**
 * @brief read one byte and return it
 * @param dev pointer to dev handle structure
 * @return the read byte
 */
u8 i2c_read_byte(const struct i2c_dev_handle_t *dev);

/**
 * @brief send a series of bytes to the slave
 * @param dev pointer to dev handle structure
 * @param size data's length
 * @param data array of uint8_t
 * @return void
 */
esp_err_t i2c_write_bytes(const struct i2c_dev_handle_t *dev, const u8 *data, u8 size);

/**
 * @brief send a byte to the slave
 * @param dev pointer to dev handle structure
 * @param data byte to send
 * @return void
 */
esp_err_t i2c_write_byte(const struct i2c_dev_handle_t *dev, u8 data);

/**
 * @brief select a register. Make the register "pointer" on the slave points to reg
 * @param reg register's address on the slave
 * @return void
 */
void i2c_select_register(u8 reg);

/**
 * @brief delete i2c driver and free memory
 */
void i2c_deinit(void);

#ifdef __cplusplus
}
#endif

#endif  // __LIBI2C_H