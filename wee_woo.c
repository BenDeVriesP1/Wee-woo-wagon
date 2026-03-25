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
#include "debug.h"


const uint LED_PIN = PICO_DEFAULT_LED_PIN;

//extern const unsigned char _binary_Normal16KhzStereo_wav_start;
extern const struct audio_data Fast16KhzMono;
extern const struct audio_data BetterLong16KhzMono;
extern const struct audio_data EEEOOOO16monokhz;
extern const struct audio_data Normal16KhzMono;
uint8_t counts;



const struct audio_data *tracklist[] = {&Normal16KhzMono,&EEEOOOO16monokhz,&BetterLong16KhzMono,&Fast16KhzMono};
const struct audio_data *selected = &Normal16KhzMono;

uint sample_head = 0;

bool muted = true;

static void __time_critical_func(audio_cb)();

void audio_cb(int16_t *new_buffer,uint size)
{
    static int32_t working_audio_buffer[AUDIO_BUFFER_SIZE];
    uint mono_size = size/2;
    int16_t temp;
    
    const struct audio_data *Aselected = selected;
    uint total_samples = Aselected->points;
    gpio_put(LED_PIN, 1);

    if(!muted)
    {
        for(uint i = 0; i < mono_size;i++)
        {
            working_audio_buffer[i] = (int32_t)Aselected->data[sample_head++];
            if(sample_head >= total_samples)
            {
                sample_head = 0;
            }
        }
    }
    else
    {
        for(uint i = 0; i < mono_size;i++)
        {
            working_audio_buffer[i] = 0;
        }
    }

    for(uint i = 0; i < mono_size;i++)
    {
        if(working_audio_buffer[i]>INT16_MAX)
        {
            temp=INT16_MAX;
        }
        else if(working_audio_buffer[i]<INT16_MIN)
        {
            temp=INT16_MIN;
        }
        else
        {
            temp=(int16_t)working_audio_buffer[i];
        }

        new_buffer[i*2] = temp;
        new_buffer[(i*2)+1] = temp;
        
    }
    gpio_put(LED_PIN, 0);
}

void mute(int argc,char **argv)
{
    muted = true;
}

DEFINE_DEBUG_SUB_NO_ARGS(mutesub,mute,"mute the wee-woos");

void unmute(int argc,char **argv)
{
    muted = false;
}


DEFINE_DEBUG_SUB_NO_ARGS(unmutesub,unmute,"unmute the wee-woos");


void cycletrack(int argc,char **argv)
{
    static uint on_track = 0;
    uint tracks_n = sizeof(tracklist)/sizeof(tracklist[0]);
    on_track++;
    on_track %= tracks_n;
    selected = tracklist[on_track];

    sample_head=0;


}

DEFINE_DEBUG_SUB_NO_ARGS(cycletracksub,cycletrack,"cycle through tracks");



void printfunc(const char *const print)
{
    printf("%s",print);
}



const uint I2S_DATA = 19;
const uint I2S_CLOCK = 20;

int main() {
    // Set a 132.000 MHz system clock to more evenly divide the audio frequencies
    set_sys_clock_khz(132000, true);
    stdio_init_all();

    sleep_ms(1000); //Give time for the usb to bootup

    printf("System Clock: %lu\n", clock_get_hz(clk_sys));
    printf("WEEE WOOO WAGON\n");

    subFnc(&mutesub);
    subFnc(&unmutesub);
    subFnc(&cycletracksub);

    startdebug(printfunc);



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

    bool firstchar = true;
    while (true) {
        
        int c = getchar_timeout_us(0);
        
        if (c != PICO_ERROR_TIMEOUT) {
            if(firstchar)
            {
                firstchar = false;
            }
            else
            {
                debugIn((const char)c);
            }
        }
    }
}