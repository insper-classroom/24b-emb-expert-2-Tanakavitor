#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include "ssd1306.h"
#include "gfx.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"
#include "hardware/dma.h"

#define CORE_0 (1 << 0)
#define CORE_1 (1 << 1)
const uint BUTTON1 = 28;
const uint BUTTON2 = 26;
const uint BUTTON3 = 27;
const uint LED1 = 20;
const uint LED2 = 21;
const uint LED3 = 22;
QueueHandle_t adcQueue;
void oled_dma_write(uint8_t *buffer, uint8_t page, uint8_t column, uint8_t length) {
    ssd1306_set_page_address(page);
    ssd1306_set_column_address(column);

    int dma_channel = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_dreq(&config, DREQ_SPI0_TX);

    dma_channel_configure(
        dma_channel,
        &config,
        &spi_get_hw(SPI_PORT)->dr,
        buffer,
        length,
        true
    );

    while (dma_channel_is_busy(dma_channel)) {
        taskYIELD();
    }

    dma_channel_unclaim(dma_channel);
}

void oled_display_task(void *params) {
    ssd1306_t display;
    uint16_t adc_value;
    float tensionn;
    char buffer[20];

    ssd1306_init();
    gfx_init(&display, 128, 32);
    const uint leds[] = {LED1, LED2, LED3};
    const uint buttons[] = {BUTTON1, BUTTON2, BUTTON3};

    for (int i = 0; i < 3; i++) {
        gpio_init(leds[i]);
        gpio_set_dir(leds[i], GPIO_OUT);
        gpio_init(buttons[i]);
        gpio_set_dir(buttons[i], GPIO_IN);
        gpio_pull_up(buttons[i]);
    }

    while (true) {
        if (xQueueReceive(adcQueue, &adc_value, portMAX_DELAY) == pdTRUE) {
            gfx_clear_buffer(&display);
            float voltage = adc_value * 3.5f;
            tensionn = voltage / 4095.0f;
            snprintf(buffer, sizeof(buffer), "Tensao: %.2f V", tensionn);
            gfx_draw_string(&display, 0, 0, 1, buffer);
            gfx_show(&display);
        }
    }
}

void adc_read_task(void *params) {
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);
    uint16_t adc_result;

    while (1) {
        adc_result = adc_read();
        
        if (xQueueSend(adcQueue, &adc_result, portMAX_DELAY) != pdPASS) {
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main() {
    stdio_init_all();
    adcQueue = xQueueCreate(32, sizeof(uint16_t));
    TaskHandle_t oledTaskHandle;
    TaskHandle_t adcTaskHandle;
    xTaskCreate(adc_read_task, "ADC Read Task", 256, NULL, 1, &adcTaskHandle);
    xTaskCreate(oled_display_task, "OLED Display Task", 256, NULL, 1, &oledTaskHandle);
    vTaskCoreAffinitySet(adcTaskHandle, CORE_1);
    vTaskCoreAffinitySet(oledTaskHandle, CORE_0);
    vTaskStartScheduler();
    while (1) {
    }
}