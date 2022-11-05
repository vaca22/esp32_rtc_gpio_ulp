#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <esp_log.h>
#include "esp_sleep.h"
#include "nvs.h"
#include "driver/rtc_io.h"
#include "esp32/ulp.h"
#include "ulp_main.h"
#include "ulp_wakeup.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

#define TAG     __FILE__

ulp_wakeup_io_e ulp_wakeup_io_get() {
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    ESP_LOGI(TAG, "cause %d", cause);
    if (cause == ESP_SLEEP_WAKEUP_ULP) {

        int wake_up_io = ulp_io_wake % 65536;
        ESP_LOGW(TAG, "----->wake_io = %d", wake_up_io);
        ulp_wakeup_io_e ulp_wake_up_io = WAKEUP_NORMAL;
        switch (wake_up_io) {
            case 0:
                ulp_wake_up_io = WAKEUP_CHARGE_DONE_PIN;
                break;
            case 1:
                ulp_wake_up_io = WAKEUP_IN_CHARGE_PIN;
                break;
            case 2:
                ulp_wake_up_io = WAKEUP_KEY_PWR_PIN;
                break;
            default:
                ulp_wake_up_io = WAKEUP_NORMAL;
                break;
        }
        ulp_timer_stop();
        return ulp_wake_up_io;
    }

    return WAKEUP_NORMAL;
}


esp_err_t ulp_wakeup_init(void) {
    esp_err_t err = ulp_load_binary(0, ulp_main_bin_start,
                                    (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
    ESP_ERROR_CHECK(err);

    gpio_num_t gpio_num = GPIO_NUM_13;
    int rtcio_num = rtc_io_number_get(gpio_num);

    gpio_num_t gpio_num2 = GPIO_NUM_14;
    int rtcio_num2 = rtc_io_number_get(gpio_num2);

    gpio_num_t gpio_num3 = GPIO_NUM_38;
    int rtcio_num3 = rtc_io_number_get(gpio_num3);


    ulp_next_edge_1 = 0;
    ulp_last_edge_1 = 1 - ulp_next_edge_1;
    ulp_io_number_1 = rtcio_num; /* map from GPIO# to RTC_IO# */


    ulp_next_edge_2 = 0;
    ulp_last_edge_2 = 1 - ulp_next_edge_2;
    ulp_io_number_2 = rtcio_num2; /* map from GPIO# to RTC_IO# */

    ulp_next_edge_3 = 1;
    ulp_last_edge_3 = 1 - ulp_next_edge_3;
    ulp_io_number_3 = rtcio_num3; /* map from GPIO# to RTC_IO# */

    ulp_io_index = 0;


    rtc_gpio_init(gpio_num);
    rtc_gpio_set_direction(gpio_num, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(gpio_num);
    rtc_gpio_pullup_en(gpio_num);
    rtc_gpio_hold_en(gpio_num);

    rtc_gpio_init(gpio_num2);
    rtc_gpio_set_direction(gpio_num2, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(gpio_num2);
    rtc_gpio_pullup_en(gpio_num2);
    rtc_gpio_hold_en(gpio_num2);

    rtc_gpio_init(gpio_num3);
    rtc_gpio_set_direction(gpio_num3, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(gpio_num3);
    rtc_gpio_pullup_en(gpio_num3);
    rtc_gpio_hold_en(gpio_num3);

    esp_deep_sleep_disable_rom_logging();
    ulp_set_wakeup_period(0, 200000);

    err = ulp_run(&ulp_entry - RTC_SLOW_MEM);

    return err;
}

