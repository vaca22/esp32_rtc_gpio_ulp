#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_sleep.h>
#include <soc/rtc_cntl_reg.h>
#include <driver/rtc_io.h>
#include <ulp_common.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "ulp_wakeup.h"

static const char *TAG = "example";

#define MOUNT_POINT "/sdcard"

#define GPIO_OUTPUT_IO_0    36
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) )
void app_main(void)
{
    rtc_gpio_hold_dis(13);
    rtc_gpio_hold_dis(14);
    rtc_gpio_hold_dis(38);
    rtc_gpio_deinit(14);
    rtc_gpio_deinit(13);
    rtc_gpio_deinit(38);
    gpio_reset_pin(13);
    gpio_reset_pin(14);
    gpio_reset_pin(38);
    CLEAR_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);
    esp_rom_delay_us(10);


    esp_err_t ret;
    int g=gpio_get_level(13);
    ESP_LOGE("gaga","%d",g);

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {

        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");


    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();


    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();


    slot_config.width = 1;

    // On chips where the GPIOs used for SD card can be configured, set them in
    // the slot_config structure:
//#ifdef CONFIG_SOC_SDMMC_USE_GPIO_MATRIX
//    slot_config.clk = CONFIG_EXAMPLE_PIN_CLK;
//    slot_config.cmd = CONFIG_EXAMPLE_PIN_CMD;
//    slot_config.d0 = CONFIG_EXAMPLE_PIN_D0;
//#ifdef CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
//    slot_config.d1 = CONFIG_EXAMPLE_PIN_D1;
//    slot_config.d2 = CONFIG_EXAMPLE_PIN_D2;
//    slot_config.d3 = CONFIG_EXAMPLE_PIN_D3;
//#endif  // CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
//#endif  // CONFIG_SOC_SDMMC_USE_GPIO_MATRIX


    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files:

    // First create a file.
    const char *file_hello = MOUNT_POINT"/hello.txt";

    ESP_LOGI(TAG, "Opening file %s", file_hello);
    FILE *f = fopen(file_hello, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    const char *file_foo = MOUNT_POINT"/foo.txt";

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_foo, &st) == 0) {
        // Delete it if it exists
        unlink(file_foo);
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file %s", file_foo);
    f = fopen(file_foo, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    // Read a line from file
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    // All done, unmount partition and disable SDMMC peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    for(int k=0;k<10;k++){
        vTaskDelay(100);
        ESP_LOGE("ff","asdf");
    }
    ulp_wakeup_init();

    esp_sleep_enable_ulp_wakeup();
    esp_deep_sleep_start();
}
