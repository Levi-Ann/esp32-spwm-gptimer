#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dev_spwm.h"

static const char *TAG = "spwm_main";
extern uint32_t isr_cnt;

void app_main(void)
{
    ESP_LOGI(TAG,"start pwm init");
    spwm_init(4, 5);
    spwm_enable(1);
    int i = 0;
  
    while(1) {
        vTaskDelay(200);
        printf("[%ld] isr cnt = %ld\r\n", esp_log_timestamp(), isr_cnt);
        spwm_enable(++i&1);
    }
    
}
