/**
 * @file loopback.h
 * @author Francesco Mecatti
 * @date 22 Mar 2021
 * @brief Loopback communcation between ESP32's I2C controllers
 */

#include <libi2c.h>

void app_main() {
    struct i2c_dev_handle_t master_device = {.addr = 0x01, .port = PORT_1};
    struct i2c_bus_t master_config = init_i2c_bus_default_master(master_device);
    i2c_init(&master_config);

    struct i2c_dev_handle_t slave_device = {.addr = 0x02, .port = PORT_0};
    struct i2c_bus_t slave_config = init_i2c_bus_default_slave(slave_device);
    i2c_init(&slave_config);

    uint8_t send_buf[512] = "aaaaaaaaaa";
    uint8_t receive_buf[512];
    while (true) {
        i2c_slave_write_buffer(slave_device.port, send_buf, 10, 1000 / portTICK_RATE_MS);
        slave_device.port = PORT_1;
        i2c_read_bytes(&slave_device, receive_buf, 10);
        receive_buf[10] = 0x00;
        printf("Slave received: %s", receive_buf);
    }
}