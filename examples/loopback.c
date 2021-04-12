/**
 * @file loopback.c
 * @author Francesco Mecatti
 * @date 22 Mar 2021
 * @brief Loopback communcation between ESP32's I2C controllers
 */

#include <libi2c.h>
#include <string.h>

uint8_t send_buf[128] = "abcdef";
uint8_t receive_buf[128];
int recv_len;  // Number of received bytes

struct i2c_dev_handle_t slave_device = {.addr = 0x02, .port = PORT_1};;

void loop_task(void *p) {
    while (true) {
            recv_len = i2c_slave_write_buffer(PORT_0, send_buf, strlen((const char *)send_buf), 1000 / portTICK_RATE_MS);
            printf("Sent bytes: %d\n", recv_len);
            if (recv_len == 0) {  // No more space in tx buffer
                vTaskDelete(NULL);
            }
            vTaskDelay(100/portTICK_RATE_MS);
            i2c_read_bytes(&slave_device, receive_buf, recv_len);
            receive_buf[recv_len] = 0x00;
            printf("Slave received: %s\n", receive_buf);
    }
}

void app_main() {
    // PORT_1
    struct i2c_bus_t master_config = init_i2c_bus_default_master();
    i2c_init(&master_config);

    // PORT_0 0x02
    struct i2c_bus_t slave_config = init_i2c_bus_default_slave(0x02);
    i2c_init(&slave_config);

    xTaskCreate(&loop_task, "loop", 2048, NULL, 1, NULL);
}