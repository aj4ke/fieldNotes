#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "driver/uart.h"

#define MIC_SCK_GPIO    GPIO_NUM_21
#define MIC_WS_GPIO     GPIO_NUM_19
#define MIC_SD_GPIO     GPIO_NUM_18
#define LED_GPIO        GPIO_NUM_15
#define BUTTON_GPIO     GPIO_NUM_23

#define SAMPLE_RATE_HZ  16000
#define CHUNK_SAMPLES   (SAMPLE_RATE_HZ / 50)  // 20 ms per read, button poll interval

#define PCM_SHIFT       12 // Gain, 12 adds 24 dB

#define USB_UART        UART_NUM_0  // console UART, wired to the USB serial chip

// Binary frames for tools/capture.py: "FNAU", type, payload length (2 bytes, little-endian), payload.
// The marker lets capture.py skip the boot log that also comes out of this port after a reset.
#define FRAME_START     'S'  // payload: uint32 sample rate
#define FRAME_AUDIO     'A'  // payload: int16 samples
#define FRAME_END       'E'  // no payload

static int32_t chunk[CHUNK_SAMPLES];
static int16_t pcm[CHUNK_SAMPLES];

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
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx));
    return rx;
}

static void io_init(void)
{
    gpio_config_t led_cfg = {
        .pin_bit_mask = 1ULL << LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&led_cfg));
    gpio_set_level(LED_GPIO, 0);

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
            gpio_set_level(LED_GPIO, recording);
            if (recording) {
                uint32_t rate = SAMPLE_RATE_HZ;
                send_frame(FRAME_START, &rate, sizeof(rate));
            } else {
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

        size_t n = bytes_read / sizeof(chunk[0]);
        for (size_t i = 0; i < n; i++) {
            int32_t s = chunk[i] >> PCM_SHIFT;
            pcm[i] = s > INT16_MAX ? INT16_MAX : (s < INT16_MIN ? INT16_MIN : s);
        }
        send_frame(FRAME_AUDIO, pcm, n * sizeof(pcm[0]));
    }
}
