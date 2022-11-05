#pragma once

#include "esp_err.h"

typedef enum {
    WAKEUP_NORMAL = -1,                //不是设置的唤醒源
    WAKEUP_CHARGE_DONE_PIN,            // 充满电 12
    WAKEUP_IN_CHARGE_PIN,              // 充电中 13
    WAKEUP_KEY_PWR_PIN,                // 电源键
} ulp_wakeup_io_e;


/**
 * 初始化 唤醒源
 * @return
 */
esp_err_t ulp_wakeup_init(void);

/**
 * 获取唤醒源
 * @return
 */
ulp_wakeup_io_e ulp_wakeup_io_get(void);



