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

static void __isr __time_critical_func(audio_i2s_dma_irq_handler)();

int16_t real_audio_buffer[AUDIO_BUFFER_SIZE * 2] = {0};

typedef struct audio_i2s_perf
{
    PIO pio;
    uint8_t i2s_sm;
    uint dma_swap_ctrl;
    uint dma_audio_out;
    //int16_t* control_blocks[2];
    int16_t* audio_buffer;
    void (*audio_cb)(int16_t*,uint);
}audio_i2s_perf_t;

audio_i2s_perf_t i2s_perf={0};

#define audio_pio __CONCAT(pio, PICO_AUDIO_I2S_PIO)
#define GPIO_FUNC_PIOx __CONCAT(GPIO_FUNC_PIO, PICO_AUDIO_I2S_PIO)
#define DREQ_PIOx_TX0 __CONCAT(__CONCAT(DREQ_PIO, PICO_AUDIO_I2S_PIO), _TX0)


static void audio_i2s_dma_irq_handler(void)
{
    dma_hw->ints0 = 1u << i2s_perf.dma_swap_ctrl;  // clear the IRQ
    if(i2s_perf.audio_cb != NULL)
    {
        if(*(int16_t**)dma_hw->ch[i2s_perf.dma_swap_ctrl].read_addr == real_audio_buffer)
        {
            i2s_perf.audio_cb(real_audio_buffer,AUDIO_BUFFER_SIZE);
            //i2s_perf.audio_cb(&real_audio_buffer[AUDIO_BUFFER_SIZE],AUDIO_BUFFER_SIZE);
        }
        else
        {
            i2s_perf.audio_cb(&real_audio_buffer[AUDIO_BUFFER_SIZE],AUDIO_BUFFER_SIZE);
            //i2s_perf.audio_cb(real_audio_buffer,AUDIO_BUFFER_SIZE);
        }

        
    }
}

//weird dma BS
int16_t* control_blocks[2] __attribute__((aligned(2*sizeof(int16_t *))));

void audio_i2s_setup(const audio_i2s_config_t *config,void (*audio_cb)(int16_t*,uint)) {
    

    //memset(&real_audio_buffer[AUDIO_BUFFER_SIZE],0xff,AUDIO_BUFFER_SIZE*2);    
    uint func = GPIO_FUNC_PIOx;
    gpio_set_function(config->data_pin, func);
    gpio_set_function(config->clock_pin_base, func);
    gpio_set_function(config->clock_pin_base + 1, func);

#if PICO_PIO_USE_GPIO_BASE
    if(config->data_pin >= 32 || config->clock_pin_base + 1 >= 32) {
        assert(config->data_pin >= 16 && config->clock_pin_base >= 16);
        pio_set_gpio_base(audio_pio, 16);
    }
#endif
    i2s_perf.i2s_sm = 0;
    i2s_perf.audio_buffer = real_audio_buffer;
    control_blocks[0] = real_audio_buffer;
    control_blocks[1] = &real_audio_buffer[AUDIO_BUFFER_SIZE];
    i2s_perf.audio_cb = audio_cb;

    pio_sm_claim(audio_pio, i2s_perf.i2s_sm );


    const struct pio_program *program = &audio_i2s_program;
    uint offset = pio_add_program(audio_pio, program);

    audio_i2s_program_init(audio_pio, i2s_perf.i2s_sm , offset, config->data_pin, config->clock_pin_base);
    uint32_t system_clock_frequency = clock_get_hz(clk_sys);
    uint32_t divider = system_clock_frequency * 4 / 16000; // avoid arithmetic overflow
    pio_sm_set_clkdiv_int_frac(audio_pio,i2s_perf.i2s_sm,divider >> 8u, divider & 0xffu);



    i2s_perf.dma_swap_ctrl = dma_claim_unused_channel(true);
    i2s_perf.dma_audio_out = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(i2s_perf.dma_swap_ctrl);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_ring(&c, false, 3);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    dma_channel_configure(i2s_perf.dma_swap_ctrl, &c, &dma_hw->ch[i2s_perf.dma_audio_out].al3_read_addr_trig, control_blocks, 1, false);


     c = dma_channel_get_default_config(i2s_perf.dma_audio_out);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_chain_to(&c, i2s_perf.dma_swap_ctrl);
    channel_config_set_dreq(&c, pio_get_dreq(audio_pio, i2s_perf.i2s_sm, true));


    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    dma_channel_configure(i2s_perf.dma_audio_out,
                          &c,
                          &audio_pio->txf[i2s_perf.i2s_sm],  // dest
                          0, // src
                          AUDIO_BUFFER_SIZE / 2, // half because we send one 2 16 bit words per transfer
                          false // trigger
    );


    dma_channel_set_irq0_enabled(i2s_perf.dma_swap_ctrl, true);
    irq_set_exclusive_handler(DMA_IRQ_0, audio_i2s_dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_start(i2s_perf.dma_swap_ctrl);
    dma_channel_start(i2s_perf.dma_audio_out);
    pio_sm_set_enabled(audio_pio,i2s_perf.i2s_sm,true);

    // irq_add_shared_handler(DMA_IRQ_0, audio_i2s_dma_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    // dma_irqn_set_channel_enabled(PICO_AUDIO_I2S_DMA_IRQ, dma_channel, 1);
}