#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "audio_def.h"




const uint LED_PIN = PICO_DEFAULT_LED_PIN;

//extern const unsigned char _binary_Normal16KhzStereo_wav_start;
extern const struct audio_data Normal16KhzStereo;
//extern const size_t _binary_Normal16KhzStereo_wav_size;

int main() {
    // Set a 132.000 MHz system clock to more evenly divide the audio frequencies
    set_sys_clock_khz(132000, true);
    stdio_init_all();

    sleep_ms(1000); //Give time for the usb to bootup

    printf("System Clock: %lu\n", clock_get_hz(clk_sys));

    printf("Data starts at 0x%08x and is %d bytes long\n",Normal16KhzStereo.data,Normal16KhzStereo.data_size);

    // Init GPIO LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t loops = 0;
    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
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