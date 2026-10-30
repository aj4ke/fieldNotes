#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "driver/uart.h"
#include "led_strip.h"

#define MIC_SCK_GPIO    GPIO_NUM_5
#define MIC_WS_GPIO     GPIO_NUM_4
#define MIC_SD_GPIO     GPIO_NUM_6
#define RGB_GPIO        GPIO_NUM_48
#define BUTTON_GPIO     GPIO_NUM_7

#define SAMPLE_RATE_HZ  16000
#define CHUNK_SAMPLES   (SAMPLE_RATE_HZ / 50)  // 20 ms per read, button poll interval

#define RGB_LEVEL       16
#define PCM_SHIFT       16 // Gain, 12 adds 24 dB

#define USB_UART        UART_NUM_0  // USB serial chip

// Binary frames for tools/capture.py: "FNAU" prefix, payload length (2 bytes, little-endian), payload.
#define FRAME_START     'S'  // payload: uint32 sample rate
#define FRAME_AUDIO     'A'  // payload: int16 samples
#define FRAME_END       'E'  // no payload

static int32_t chunk[CHUNK_SAMPLES * 2];
static int16_t pcm[CHUNK_SAMPLES];
static led_strip_handle_t rgb;

static i2s_chan_handle_t mic_init(void)
{
    i2s_chan_handle_t rx;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = MIC_SCK_GPIO,
            .ws = MIC_WS_GPIO,
            .dout = I2S_GPIO_UNUSED,
            .din = MIC_SD_GPIO,
        },
    };
    
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH; // expect both LR channels on the new board so audio isn't slowed

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx));
    return rx;
}

static void io_init(void)
{
    led_strip_config_t rgb_cfg = {
        .strip_gpio_num = RGB_GPIO,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
    };
    led_strip_rmt_config_t rmt_cfg = {
        .resolution_hz = 10 * 1000 * 1000,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&rgb_cfg, &rmt_cfg, &rgb));
    led_strip_clear(rgb);

    gpio_config_t button_cfg = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&button_cfg));

    // Writing through the UART driver instead of printf skips stdout's \n -> \r\n conversion,
    // which would corrupt the binary audio
    ESP_ERROR_CHECK(uart_driver_install(USB_UART, 256, 0, 0, NULL, 0));
}

static void send_frame(char type, const void *payload, uint16_t len)
{
    const uint8_t header[] = {'F', 'N', 'A', 'U', type, len & 0xff, len >> 8};
    uart_write_bytes(USB_UART, header, sizeof(header));
    if (len) {
        uart_write_bytes(USB_UART, payload, len);
    }
}

void app_main(void)
{
    io_init();
    i2s_chan_handle_t rx = mic_init();

    bool recording = false;
    bool was_pressed = false;

    while (true) {
        bool pressed = gpio_get_level(BUTTON_GPIO) == 0;
        if (pressed && !was_pressed) {
            recording = !recording;
            if (recording) {
                led_strip_set_pixel(rgb, 0, RGB_LEVEL, 0, 0);
                led_strip_refresh(rgb);
                uint32_t rate = SAMPLE_RATE_HZ;
                send_frame(FRAME_START, &rate, sizeof(rate));
            } else {
                led_strip_clear(rgb);
                send_frame(FRAME_END, NULL, 0);
            }
        }
        was_pressed = pressed;

        // Read forever for DMA buffer to be clear
        size_t bytes_read;
        ESP_ERROR_CHECK(i2s_channel_read(rx, chunk, sizeof(chunk), &bytes_read, portMAX_DELAY));
        if (!recording) {
            continue;
        }

        // chunk is L,R,L,R
        size_t n = bytes_read / (2 * sizeof(chunk[0]));
        for (size_t i = 0; i < n; i++) {
            int32_t s = chunk[2 * i] >> PCM_SHIFT;
            pcm[i] = s > INT16_MAX ? INT16_MAX : (s < INT16_MIN ? INT16_MIN : s);
        }
        send_frame(FRAME_AUDIO, pcm, n * sizeof(pcm[0]));
    }
}
