/**
 * @file led_controller.c
 * @brief WS2812 LED Strip Controller Implementation
 *
 * Implementation of WS2812 LED controller using ESP32 RMT peripheral.
 * Manages LED state in RAM buffer and provides hardware abstraction.
 */

#include "led_controller.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "led_controller";

// WS2812 timing constants (in RMT ticks at 80MHz)
#define WS2812_T0H_TICKS 32     // 0.4us high for bit 0
#define WS2812_T0L_TICKS 64     // 0.8us low for bit 0
#define WS2812_T1H_TICKS 64     // 0.8us high for bit 1
#define WS2812_T1L_TICKS 32     // 0.4us low for bit 1
#define WS2812_RESET_TICKS 4000 // 50us reset pulse

// Internal state
static led_color_t *led_buffer = NULL;
static led_config_t current_config = {0};
static bool is_initialized = false;
static rmt_item32_t *rmt_buffer = NULL;

// Predefined color constants
const led_color_t LED_COLOR_RED = {255, 0, 0};
const led_color_t LED_COLOR_GREEN = {0, 255, 0};
const led_color_t LED_COLOR_BLUE = {0, 0, 255};
const led_color_t LED_COLOR_WHITE = {255, 255, 255};
const led_color_t LED_COLOR_YELLOW = {255, 255, 0};
const led_color_t LED_COLOR_CYAN = {0, 255, 255};
const led_color_t LED_COLOR_MAGENTA = {255, 0, 255};
const led_color_t LED_COLOR_OFF = {0, 0, 0};

/**
 * @brief Convert single byte to RMT items for WS2812
 *
 * Converts 8-bit value to 8 RMT items representing WS2812 bit timing.
 *
 * @param byte Byte value to convert
 * @param rmt_items Output buffer for 8 RMT items
 */
static void byte_to_rmt_items(uint8_t byte, rmt_item32_t *rmt_items)
{
    for (int i = 7; i >= 0; i--)
    {
        if (byte & (1 << i))
        {
            // Bit 1: High for 0.8us, Low for 0.4us
            rmt_items[7 - i].level0 = 1;
            rmt_items[7 - i].duration0 = WS2812_T1H_TICKS;
            rmt_items[7 - i].level1 = 0;
            rmt_items[7 - i].duration1 = WS2812_T1L_TICKS;
        }
        else
        {
            // Bit 0: High for 0.4us, Low for 0.8us
            rmt_items[7 - i].level0 = 1;
            rmt_items[7 - i].duration0 = WS2812_T0H_TICKS;
            rmt_items[7 - i].level1 = 0;
            rmt_items[7 - i].duration1 = WS2812_T0L_TICKS;
        }
    }
}

/**
 * @brief Configure RMT channel for WS2812 timing
 */
static esp_err_t configure_rmt_channel(void)
{
    rmt_config_t config = {
        .rmt_mode = RMT_MODE_TX,
        .channel = current_config.rmt_channel,
        .gpio_num = current_config.gpio_pin,
        .clk_div = 1, // 80MHz clock
        .mem_block_num = 1,
        .tx_config = {
            .carrier_en = false,
            .loop_en = false,
            .idle_level = RMT_IDLE_LEVEL_LOW,
            .idle_output_en = true,
        }};

    esp_err_t ret = rmt_config(&config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure RMT channel: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = rmt_driver_install(current_config.rmt_channel, 0, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install RMT driver: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t led_controller_init(const led_config_t *config)
{
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Configuration cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (is_initialized)
    {
        ESP_LOGW(TAG, "LED controller already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (config->led_count == 0 || config->led_count > 1000)
    {
        ESP_LOGE(TAG, "Invalid LED count: %d", config->led_count);
        return ESP_ERR_INVALID_ARG;
    }

    // Store configuration
    current_config = *config;

    // Allocate LED buffer
    led_buffer = (led_color_t *)malloc(config->led_count * sizeof(led_color_t));
    if (led_buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate LED buffer");
        return ESP_ERR_NO_MEM;
    }

    // Allocate RMT buffer (24 bits per LED + reset pulse)
    size_t rmt_buffer_size = (config->led_count * 24 + 1) * sizeof(rmt_item32_t);
    rmt_buffer = (rmt_item32_t *)malloc(rmt_buffer_size);
    if (rmt_buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate RMT buffer");
        free(led_buffer);
        led_buffer = NULL;
        return ESP_ERR_NO_MEM;
    }

    // Configure RMT
    esp_err_t ret = configure_rmt_channel();
    if (ret != ESP_OK)
    {
        free(led_buffer);
        free(rmt_buffer);
        led_buffer = NULL;
        rmt_buffer = NULL;
        return ret;
    }

    // Initialize all LEDs to off
    led_clear_all();

    is_initialized = true;
    ESP_LOGI(TAG, "LED controller initialized: %d LEDs on GPIO%d, RMT channel %d",
             config->led_count, config->gpio_pin, config->rmt_channel);

    return ESP_OK;
}

esp_err_t led_controller_deinit(void)
{
    if (!is_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    // Turn off all LEDs
    led_clear_all();
    led_show();

    // Cleanup RMT
    rmt_driver_uninstall(current_config.rmt_channel);

    // Free memory
    free(led_buffer);
    free(rmt_buffer);
    led_buffer = NULL;
    rmt_buffer = NULL;

    is_initialized = false;
    ESP_LOGI(TAG, "LED controller deinitialized");

    return ESP_OK;
}

esp_err_t led_set_pixel(uint16_t index, led_color_t color)
{
    if (!is_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (index >= current_config.led_count)
    {
        ESP_LOGE(TAG, "LED index %d out of range (0-%d)", index, current_config.led_count - 1);
        return ESP_ERR_INVALID_ARG;
    }

    led_buffer[index] = color;
    return ESP_OK;
}

esp_err_t led_clear_pixel(uint16_t index)
{
    return led_set_pixel(index, LED_COLOR_OFF);
}

led_color_t led_get_pixel(uint16_t index)
{
    if (!is_initialized || index >= current_config.led_count)
    {
        return LED_COLOR_OFF;
    }

    return led_buffer[index];
}

esp_err_t led_clear_all(void)
{
    if (!is_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    for (uint16_t i = 0; i < current_config.led_count; i++)
    {
        led_buffer[i] = LED_COLOR_OFF;
    }

    return ESP_OK;
}

esp_err_t led_show(void)
{
    if (!is_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    // Convert LED buffer to RMT items
    for (uint16_t led = 0; led < current_config.led_count; led++)
    {
        // WS2812 expects GRB order
        uint8_t green = led_buffer[led].green;
        uint8_t red = led_buffer[led].red;
        uint8_t blue = led_buffer[led].blue;

        // Convert each color byte to RMT items
        byte_to_rmt_items(green, &rmt_buffer[led * 24 + 0]);
        byte_to_rmt_items(red, &rmt_buffer[led * 24 + 8]);
        byte_to_rmt_items(blue, &rmt_buffer[led * 24 + 16]);
    }

    // Add reset pulse at the end
    rmt_item32_t *reset_item = &rmt_buffer[current_config.led_count * 24];
    reset_item->level0 = 0;
    reset_item->duration0 = WS2812_RESET_TICKS;
    reset_item->level1 = 0;
    reset_item->duration1 = 0;

    // Transmit
    esp_err_t ret = rmt_write_items(current_config.rmt_channel, rmt_buffer,
                                    current_config.led_count * 24 + 1, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to transmit LED data: %s", esp_err_to_name(ret));
    }

    return ret;
}

led_color_t led_color_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    led_color_t color = {r, g, b};
    return color;
}

led_color_t led_color_brightness(led_color_t color, uint8_t brightness)
{
    led_color_t result;
    result.red = (color.red * brightness) / 255;
    result.green = (color.green * brightness) / 255;
    result.blue = (color.blue * brightness) / 255;
    return result;
}

uint16_t led_get_count(void)
{
    return is_initialized ? current_config.led_count : 0;
}

bool led_is_initialized(void)
{
    return is_initialized;
}
