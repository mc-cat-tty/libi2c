/**
 * @file libi2c.c
 * @author Francesco Mecatti
 * @date 8 Mar 2021
 * @brief I2C library and tools for ESP32 - ESP-IDF framework
 */

#include <libi2c.h>

struct i2c_bus_t tmp_conf;

/**
 * @brief check if a pointer is null. Use in combination with assert()
 * @param ptr pointer, input argument
 * @return true if ptr is not null, false otherwise
 */
static bool ptr_check(void *ptr) {
    return ptr != NULL;
}

// static void i2c_master_init(void) {  // configuration structure contained is tmp_conf
    
// }

// static void i2c_slave_init(void) {  // configuration structure contained is tmp_conf

// }

void i2c_init(const struct i2c_bus_t *conf) {
    tmp_conf = *conf;
    // if (tmp_conf.esp_idf_conf.mode == I2C_MODE_MASTER) {
    //     i2c_master_init();
    // }
    // else if (tmp_conf.esp_idf_conf.mode == I2C_MODE_SLAVE) {
    //     i2c_slave_init();
    // }
    // else {  // Neither master nor slave are set. Or both master and slave are set to true.
    //     return;
    // }
    i2c_param_config(tmp_conf.port, &(tmp_conf.conf));
    i2c_driver_install(tmp_conf.port, tmp_conf.conf.mode, tmp_conf.rx, tmp_conf.tx, 0);
}

esp_err_t i2c_read_bytes(const struct i2c_dev_handle_t *dev, u8 *data, u8 size) {
    assert(size);
    assert(ptr_check(data));
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | READ_BIT, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, data, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(dev->port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

u8 i2c_read_byte(const struct i2c_dev_handle_t *dev) {  // dev pointer integrity delegated to i2c_read_bytes
    u8 *buf = (u8*) malloc(sizeof(u8) * 1);
    i2c_read_bytes(dev, 1, buf);
    return *buf;
}

esp_err_t i2c_write_bytes(const struct i2c_dev_handle_t *dev, const u8 *data, u8 size) {
    assert(ptr_check(dev));
    assert(size);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(dev->port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t i2c_write_byte(const struct i2c_dev_handle_t *dev, u8 data) {  // pointer integrity check delegated to i2c_write_bytes
    return i2c_write_bytes(dev, 1, &data);    
}