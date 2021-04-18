/**
 * @file bmp280_compensations.c
 * @author Francesco Mecatti
 * @date 22 Mar 2021
 * @brief Loopback communcation between ESP32's I2C controllers
 */

#include <libi2c.h>
#include <string.h>
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <esp_log.h>

#define BMP280_CHIP_ID 0x58

struct i2c_bus_t master_config = init_i2c_bus_default_master();
struct i2c_dev_handle_t bmp280 = {.addr = 0x76, .port = PORT_1};
uint8_t id;

int16_t compensation_params[24];  // See datasheet for more details
int16_t processed_compensation_params[12];

// Read and check device's chip id
esp_err_t check_id() {
    i2c_select_register(&bmp280, 0xd0);
    i2c_read_bytes(&bmp280, &id, 1);
    ESP_LOGD("ID_CHECK", "Received bytes: 0x%02x\n", id);
    if (id != BMP280_CHIP_ID)
        return ESP_ERR_INVALID_RESPONSE;
    return ESP_OK;
}

void print_array(int16_t *v, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", v[i]);
    }
    printf("\n");
}

void process_compensation_array(int16_t *raw_data, int raw_data_size, int16_t *data) {
    for (int i = 0; i < raw_data_size; i++) {
        data[i] = raw_data[i+1] << 8 | raw_data[i];
    }
}

void read_compensations() {
    i2c_select_register(&bmp280, 0x88);  // Select the first register, then burst read
    i2c_read_bytes(&bmp280, compensation_params, 24);
    printf("Raw compensation vector: ");
    print_array(compensation_params, 24);
    printf("Processed compensation vector: ");
    process_compensation_array(compensation_params, 24, processed_compensation_params);
    print_array(processed_compensation_params, 12);
}

void app_main() {
    i2c_init(&master_config);
    ESP_ERROR_CHECK(check_id());
    read_compensations();
}