/**
 * @file bmp280_ssd1306_clock.c
 * @author Francesco Mecatti
 * @date 19 Apr 2021
 * @brief Little weather station with BMP280 press and temp sensor and SSD1306 LCD display + clock
 * @see https://github.com/lexus2k/ssd1306
 * @warning In order to use this piece of software you have to download lexus2k's library and move it in 'components' dir inside your project folder
 */

#include <libi2c.h>
#include <string.h>
#include <time.h>

#include <ssd1306.h>

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <esp_log.h>

#define BMP280_CHIP_ID 0x58

#define START_TIME 1618840186
#define offset 7200  // 2 hours for GMT+2

struct i2c_bus_t master_config = init_i2c_bus_default_master();
struct i2c_dev_handle_t bmp280 = {.addr = 0x76, .port = PORT_0};
uint8_t id;
int32_t glob_t_fine;

time_t now;
char strftime_buf[64];
struct tm timeinfo;

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

union compensation_params cp;

// Read and check device's chip id
esp_err_t check_id() {
    i2c_select_register(&bmp280, 0xd0, READ_BIT);
    id = i2c_read_byte(&bmp280);
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

void read_compensations() {
    i2c_select_register(&bmp280, 0x88, READ_BIT);  // Select the first register, then burst read
    uint16_t data;
    for (int i = 0; i < 12; i++) {
        i2c_read_bytes(&bmp280, &data, 2);
        cp.array[i] = data;
        ESP_LOGD("COMPENSATIONS", "%d", cp.array[i]);
    }
}

void log_compensations() {
    ESP_LOGI("COMPENSATIONS", "dig_T1 = %d", cp.dig_T1);
    ESP_LOGI("COMPENSATIONS", "dig_T2 = %d", cp.dig_T2);
    ESP_LOGI("COMPENSATIONS", "dig_T3 = %d", cp.dig_T3);
    ESP_LOGI("COMPENSATIONS", "dig_P1 = %d", cp.dig_P1);
    ESP_LOGI("COMPENSATIONS", "dig_P2 = %d", cp.dig_P2);
    ESP_LOGI("COMPENSATIONS", "dig_P3 = %d", cp.dig_P3);
    ESP_LOGI("COMPENSATIONS", "dig_P4 = %d", cp.dig_P4);
    ESP_LOGI("COMPENSATIONS", "dig_P5 = %d", cp.dig_P5);
    ESP_LOGI("COMPENSATIONS", "dig_P6 = %d", cp.dig_P6);
    ESP_LOGI("COMPENSATIONS", "dig_P7 = %d", cp.dig_P7);
    ESP_LOGI("COMPENSATIONS", "dig_P8 = %d", cp.dig_P8);
    ESP_LOGI("COMPENSATIONS", "dig_P9 = %d", cp.dig_P9);
}

float compensate_temp(int32_t raw_t) {
    int32_t var1, var2;
    raw_t >>= 4;

    var1 = ((((raw_t >> 3) - ((int32_t) cp.dig_T1 << 1)))
            * ((int32_t) cp.dig_T2)) >> 11;

    var2 = (((((raw_t >> 4) - ((int32_t) cp.dig_T1))
            * ((raw_t >> 4) - ((int32_t) cp.dig_T1))) >> 12)
            * ((int32_t) cp.dig_T3)) >> 14;
    
    glob_t_fine = var1 + var2;
    return ((((var1 + var2) * 5 + 128) >> 8) / 100.0);
}

float compensate_press(int32_t raw_p) {
    int64_t var1, var2, p;
    raw_p >>= 4;

    var1 = ((int64_t) glob_t_fine) - 128000;
    var2 = var1 * var1 * (int64_t) cp.dig_P6;
    var2 = var2 + ((var1 * (int64_t) cp.dig_P5) << 17);
    var2 = var2 + (((int64_t) cp.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t) cp.dig_P3) >> 8)
            + ((var1 * (int64_t) cp.dig_P2) << 12);
    var1 = (((((int64_t) 1) << 47) + var1)) * ((int64_t) cp.dig_P1)
            >> 33;

    if (var1 == 0) {
        return -1; // avoid exception caused by division by zero
    }
    p = 1048576 - raw_p;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t) cp.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t) cp.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t) cp.dig_P7) << 4);
    p = p >> 8; // /256
    return ((float) p / 100);
}

void read_values_task(void *pv) {
    int32_t raw_temp, raw_press;
    float temp, press;
    uint8_t temp_buf[3] = {0};
    uint8_t press_buf[3] = {0};
    char temp_str[32];
    char press_str[32];
    char time_str[32];

    while (true) {
        i2c_select_register(&bmp280, 0xf7, READ_BIT);  // Read from 0xf7 to 0xfc
        i2c_read_bytes(&bmp280, press_buf, 3);
        i2c_read_bytes(&bmp280, temp_buf, 3);
        raw_temp = (temp_buf[0] << 16) | (temp_buf[1] << 8) | temp_buf[2];
        raw_press = (press_buf[0] << 16) | (press_buf[1] << 8) | press_buf[2];
        if (raw_temp == 0x800000 || raw_press == 0x800000) {  // Value in case temp or press measurement was disabled
            ESP_LOGD("VALUES", "Measurement disabled: raw_temp: %d\t|\traw_press: %d", raw_temp, raw_press);
            continue;
        }
        
        temp = compensate_temp(raw_temp);
        press = compensate_press(raw_temp);
        ESP_LOGI("VALUES", "Temperature: %f\n", temp);
        ESP_LOGI("VALUES", "Pressure: %f\n", press);

        time(&now);
        now += START_TIME;
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        sprintf(time_str, "%.9s\n", strftime_buf+strlen(strftime_buf)-14);
        ESP_LOGI("TIME", "%s", time_str);

        // Time
        // sprintf(temp_str, "  %.2f  ", temp);
        // ssd1306_printFixed(0,  8, temp_str, STYLE_NORMAL);

        ssd1306_setFixedFont(ssd1306xled_font8x16);

        // Temp
        sprintf(temp_str, " %02.2f", temp);
        ssd1306_printFixed(0,  33, temp_str, STYLE_BOLD);
        ssd1306_printFixed(45,  48, "°C", STYLE_NORMAL);

        // Press
        sprintf(press_str, " %03.2f", press);
        ssd1306_printFixed(ssd1306_displayWidth()/2+1,  33, press_str, STYLE_BOLD);
        ssd1306_printFixed(ssd1306_displayWidth()/2+30,  48, "mBar", STYLE_NORMAL);

        ssd1306_setFixedFont(ssd1306xled_font6x8);
        ssd1306_printFixed(ssd1306_displayWidth()/2-5*6, 8, time_str, STYLE_NORMAL); 

        vTaskDelay(500/portTICK_RATE_MS);
    }
}

void bmp280_init(void) {
    i2c_select_register(&bmp280, 0xe0, WRITE_BIT);  // Reset register
    i2c_write_byte(&bmp280, 0xb6);  // Reset using soft-reset

    vTaskDelay(300/portTICK_RATE_MS);  // Wait for wake-up
    do {
        i2c_select_register(&bmp280, 0xf3, READ_BIT);  // Calibration register
        vTaskDelay(100/portTICK_RATE_MS);
    } while (i2c_read_byte(&bmp280) & 0x01);
    
    read_compensations();

    // iot_bme280_set_sampling(dev, BME280_MODE_NORMAL, BME280_SAMPLING_X16,
    //         BME280_SAMPLING_X16, BME280_SAMPLING_X16, BME280_FILTER_OFF,
    //         BME280_STANDBY_MS_0_5)

    i2c_select_register(&bmp280, 0xf4, WRITE_BIT);
    i2c_write_byte(&bmp280, 0xff);
}

void display_init() {
    ssd1306_128x64_i2c_init();
    ssd1306_clearScreen();
    ssd1306_setFixedFont(ssd1306xled_font8x16);
    ssd1306_drawLine(0, ssd1306_displayHeight() - 43, ssd1306_displayWidth(), ssd1306_displayHeight() - 43);
    ssd1306_drawLine(ssd1306_displayWidth()/2, ssd1306_displayHeight()-1, ssd1306_displayWidth()/2, ssd1306_displayHeight() - 43);
    // ssd1306_printFixed(0,  8, "Normal text", STYLE_NORMAL);
}

void update_time_task(void *pv) {
    time(&now);
    // setenv("TZ", "CST-8", 1);
    // tzset();
    now += START_TIME;
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    while (true) {
        ESP_LOGI("TIME", "%s\n", strftime_buf);
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}

void time_init() {
    setenv("TZ", "GMT+1", 1);
    tzset();
}

void app_main() {
    master_config.port = PORT_0;
    i2c_init(&master_config);
    ESP_ERROR_CHECK(check_id());
    bmp280_init();
    display_init();
    // time_init();
    log_compensations();

    xTaskCreate(read_values_task, "read_values", 2048, NULL, 1, NULL);
    // xTaskCreate(update_time_task, "update_time", 1024, NULL, 1, NULL);
}