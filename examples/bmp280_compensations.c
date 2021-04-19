/**
 * @file bmp280_compensations.c
 * @author Francesco Mecatti
 * @date 22 Mar 2021
 * @brief Loopback communcation between ESP32's I2C controllers
 */

#include <libi2c.h>
#include <string.h>
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include <esp_log.h>

#define BMP280_CHIP_ID 0x58

struct i2c_bus_t master_config = init_i2c_bus_default_master();
struct i2c_dev_handle_t bmp280 = {.addr = 0x76, .port = PORT_1};
uint8_t id;

// int16_t compensation_params[24];  // See datasheet for more details
// int16_t processed_compensation_params[12];

static union compensation_params {  // Trashy workaround to access struct memebers as if it was an array 
    #pragma pack(1)  // Prevent struct padding
    struct {  // See datasheet for more details
        uint16_t dig_T1;
        int16_t dig_T2;
        int16_t dig_T3;
        uint16_t dig_P1;
        int16_t dig_P2;
        int16_t dig_P3;
        int16_t dig_P4;
        int16_t dig_P5;
        int16_t dig_P6;
        int16_t dig_P7;
        int16_t dig_P8;
        int16_t dig_P9;
    };
    uint16_t array[12];
};

// Read and check device's chip id
esp_err_t check_id() {
    i2c_select_register(&bmp280, 0xd0, READ_BIT);
    i2c_read_bytes(&bmp280, &id, 1);
    ESP_LOGI("ID_CHECK", "Received bytes: 0x%02x\n", id);
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

// void process_compensation_array(int16_t *raw_data, int raw_data_size, int16_t *data) {
//     for (int i = 0; i < raw_data_size; i++) {
//         data[i] = raw_data[i+1] << 8 | raw_data[i];
//     }
// }

void read_compensations(union compensation_params *cp) {
    i2c_select_register(&bmp280, 0x88, READ_BIT);  // Select the first register, then burst read
    uint16_t data;
    for (int i = 0; i < 12; i++) {
        i2c_read_bytes(&bmp280, &data, 2);
        cp->array[i] = data;
        ESP_LOGD("COMPENSATIONS", "%d", cp->array[i]);
    }
}

void log_compensations(union compensation_params *cp) {
    ESP_LOGI("COMPENSATIONS", "dig_T1 = %d", cp->dig_T1);
    ESP_LOGI("COMPENSATIONS", "dig_T2 = %d", cp->dig_T2);
    ESP_LOGI("COMPENSATIONS", "dig_T3 = %d", cp->dig_T3);
    ESP_LOGI("COMPENSATIONS", "dig_P1 = %d", cp->dig_P1);
    ESP_LOGI("COMPENSATIONS", "dig_P2 = %d", cp->dig_P2);
    ESP_LOGI("COMPENSATIONS", "dig_P3 = %d", cp->dig_P3);
    ESP_LOGI("COMPENSATIONS", "dig_P4 = %d", cp->dig_P4);
    ESP_LOGI("COMPENSATIONS", "dig_P5 = %d", cp->dig_P5);
    ESP_LOGI("COMPENSATIONS", "dig_P6 = %d", cp->dig_P6);
    ESP_LOGI("COMPENSATIONS", "dig_P7 = %d", cp->dig_P7);
    ESP_LOGI("COMPENSATIONS", "dig_P8 = %d", cp->dig_P8);
    ESP_LOGI("COMPENSATIONS", "dig_P9 = %d", cp->dig_P9);
}

void app_main() {
    union compensation_params cp;
    i2c_init(&master_config);
    ESP_ERROR_CHECK(check_id());
    read_compensations(&cp);
    log_compensations(&cp);
}