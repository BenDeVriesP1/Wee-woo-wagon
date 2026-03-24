#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "audio_def.h"

#include "audio_i2s.pio.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "i2s_audio.h"


const uint LED_PIN = PICO_DEFAULT_LED_PIN;

//extern const unsigned char _binary_Normal16KhzStereo_wav_start;
extern const struct audio_data Normal16KhzStereo;
//extern const size_t _binary_Normal16KhzStereo_wav_size;
uint8_t counts;


uint sample_head = 0;

void audio_cb(int16_t *new_buffer,uint size)
{
    uint total_samples = sizeof(Normal16KhzStereo.data)/sizeof(Normal16KhzStereo.data)[0];
    counts++;
    if(counts % 2)
    {
        gpio_put(LED_PIN, 1);
    }
    else
    {
        gpio_put(LED_PIN, 0);
    }
    for(uint i = 0; i < size;i++)
    {
        new_buffer[i] = Normal16KhzStereo.data[sample_head++];
        if(sample_head >= total_samples)
        {
            sample_head = 0;
        }
    }
}


const uint I2S_DATA = 19;
const uint I2S_CLOCK = 20;

int main() {
    // Set a 132.000 MHz system clock to more evenly divide the audio frequencies
    set_sys_clock_khz(132000, true);
    stdio_init_all();

    sleep_ms(1000); //Give time for the usb to bootup

    printf("System Clock: %lu\n", clock_get_hz(clk_sys));

    printf("Data starts at 0x%08x and is %d bytes long\n",Normal16KhzStereo.data,Normal16KhzStereo.data_size);

    audio_i2s_config_t channel_1 = {
        .clock_pin_base = I2S_CLOCK,
        .data_pin = I2S_DATA,
    };

    

    // Init GPIO LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);

    audio_i2s_setup(&channel_1,audio_cb);

    uint32_t loops = 0;
    while (true) {
        
        sleep_ms(100);
        //gpio_put(LED_PIN, 0);
        printf("woo\n");
        sleep_ms(100);
        printf("wee\n");
        // if(loops & 0x1)
        // {
        //     printf("wee\n");
        // }
        // else
        // {
        //     printf("woo\n");
        // }
        // loops++;
    }
}