#include <stdio.h>
#include <string.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"
#include "hal/ledc_hal.h"
#include "soc/ledc_struct.h"

#include "dev_spwm.h"

static const char *TAG = "ledc_spwm";
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT // Set duty resolution to 10 bits
#define LEDC_MAX_DUTY			  (1023)
#define LEDC_PWM_NUM			  (2)
#define LEDC_PWM0_GPIO			(4)
#define LEDC_PWM1_GPIO			(5)

ledc_channel_config_t led_channel[LEDC_PWM_NUM];
static gptimer_handle_t gptimer = NULL;

#define CAR 15000
#define FRQ 50
#define LEN (CAR/FRQ)
static int ch;
static uint16_t duty[CAR/FRQ];
uint32_t isr_cnt = 0;

static void led_channel_init(int i, ledc_channel_t channel, uint32_t timer_sel, gpio_num_t gpio){
	led_channel[i].channel = channel;
	led_channel[i].duty = 0;
	led_channel[i].gpio_num = gpio;
	led_channel[i].speed_mode = LEDC_MODE;
	led_channel[i].timer_sel  = timer_sel;
  led_channel[i].hpoint     = 0;
}

static void ledc_pwm_init(int pa, int pb){
	ledc_timer_config_t led_timer = {
	.duty_resolution = LEDC_DUTY_RES,
	.freq_hz = CAR,
	.speed_mode = LEDC_MODE,
	.timer_num = LEDC_TIMER,
    .clk_cfg = LEDC_AUTO_CLK,
	};
	ledc_timer_config(&led_timer);
 
	led_channel_init(0, LEDC_CHANNEL_0, LEDC_TIMER_0, pa);
	led_channel_init(1, LEDC_CHANNEL_1, LEDC_TIMER_0, pb);

	for (ch = 0; ch < 2; ch++) {
        ledc_channel_config(&led_channel[ch]);
    }
}

//Generate sinusoidal data have value in the front segment and zero in the back segment
void duty_init(){
    for (int i = 0; i < sizeof(duty)/sizeof(duty[0])/2; ++i) {
        float x = M_PI * 2 * i * FRQ / CAR;
        duty[i] = sin(x) * 1024;
    }
    for (int i = sizeof(duty)/sizeof(duty[0])/2; i < sizeof(duty)/sizeof(duty[0]); ++i) {
        duty[i] = 0;
    }
}

static inline void dev_update_duty(int chn, int duty) {
    LEDC.channel_group[LEDC_MODE].channel[chn].duty.val = duty<<4;
    LEDC.channel_group[LEDC_MODE].channel[chn].conf0.sig_out_en = true;
    LEDC.channel_group[LEDC_MODE].channel[chn].conf1.duty_start = true;
    LEDC.channel_group[LEDC_MODE].channel[chn].conf0.low_speed_update = 1;
}

static bool IRAM_ATTR timer_on_alarm_cb_v1(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    uint32_t i = isr_cnt % LEN;
    uint32_t j = (i + LEN/2) % LEN;

    dev_update_duty(0, duty[i]);    
    dev_update_duty(1, duty[j]);

    isr_cnt ++;
    return 0;
}

static void alarm_init(int x) {
    if(x == 0){
        if (gptimer) gptimer_stop(gptimer);
        ESP_LOGI(TAG, "GP STOP");

        return;
    }
    if (x && gptimer) {
        gptimer_start(gptimer);
        ESP_LOGI(TAG, "GP START");
        return;   
    }
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_on_alarm_cb_v1,
    };
    
    gptimer_alarm_config_t alarm_config1 = {
        .alarm_count = 1000000/CAR, // period = 1s
        .flags.auto_reload_on_alarm = 1,
    };

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_LOGI(TAG, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config1));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}


void spwm_init(int pa, int pb){
    duty_init();
    ledc_pwm_init(pa, pb);
}

void spwm_enable(int x){
    if (x > 0) {
        alarm_init(1);
    }
    else if (x == 0) {
        alarm_init(0);
        dev_update_duty(0, 0);
        dev_update_duty(1, 0);
    } else {
        alarm_init(0);
        dev_update_duty(0, 1023);
        dev_update_duty(1, 1023);        
    }    
}
