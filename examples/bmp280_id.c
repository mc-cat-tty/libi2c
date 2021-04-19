/**
 * @file bmp280_id.c
 * @author Francesco Mecatti
 * @date 22 Mar 2021
 * @brief Loopback communcation between ESP32's I2C controllers
 */

#include <libi2c.h>
#include <string.h>

void app_main() {
    struct i2c_bus_t master_config = init_i2c_bus_default_master();
    master_config.conf.master.clk_speed = 100000;
    i2c_init(&master_config);
    struct i2c_dev_handle_t bmp280 = {.addr = 0x76, .port = PORT_1};
    uint8_t id;

    while (true) {
        i2c_select_register(&bmp280, 0xd0, READ_BIT);
        i2c_read_bytes(&bmp280, &id, 1);  
        printf("Received bytes: 0x%02x\n", id);
        vTaskDelay(100/portTICK_RATE_MS);
    }
}