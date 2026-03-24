#include <stdint.h>

#define AUDIO_BUFFER_SIZE 4096

#ifndef PICO_AUDIO_I2S_DMA_IRQ
#ifdef PICO_AUDIO_DMA_IRQ
#define PICO_AUDIO_I2S_DMA_IRQ PICO_AUDIO_DMA_IRQ
#else
#define PICO_AUDIO_I2S_DMA_IRQ 0
#endif
#endif

#ifndef PICO_AUDIO_I2S_PIO
#ifdef PICO_AUDIO_PIO
#define PICO_AUDIO_I2S_PIO PICO_AUDIO_PIO
#else
#define PICO_AUDIO_I2S_PIO 0
#endif
#endif

#if !(PICO_AUDIO_I2S_DMA_IRQ == 0 || PICO_AUDIO_I2S_DMA_IRQ == 1)
#error PICO_AUDIO_I2S_DMA_IRQ must be 0 or 1
#endif

#if !(PICO_AUDIO_I2S_PIO == 0 || PICO_AUDIO_I2S_PIO == 1)
#error PICO_AUDIO_I2S_PIO ust be 0 or 1
#endif

#ifndef PICO_AUDIO_I2S_MAX_CHANNELS
#ifdef PICO_AUDIO_MAX_CHANNELS
#define PICO_AUDIO_I2S_MAX_CHANNELS PICO_AUDIO_MAX_CHANNELS
#else
#define PICO_AUDIO_I2S_MAX_CHANNELS 2u
#endif
#endif

#ifndef PICO_AUDIO_I2S_BUFFERS_PER_CHANNEL
#ifdef PICO_AUDIO_BUFFERS_PER_CHANNEL
#define PICO_AUDIO_I2S_BUFFERS_PER_CHANNEL PICO_AUDIO_BUFFERS_PER_CHANNEL
#else
#define PICO_AUDIO_I2S_BUFFERS_PER_CHANNEL 3u
#endif
#endif

#ifndef PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH
#ifdef PICO_AUDIO_BUFFER_SAMPLE_LENGTH
#define PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH PICO_AUDIO_BUFFER_SAMPLE_LENGTH
#else
#define PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH 576u
#endif
#endif

#ifndef PICO_AUDIO_I2S_SILENCE_BUFFER_SAMPLE_LENGTH
#ifdef PICO_AUDIO_I2S_SILENCE_BUFFER_SAMPLE_LENGTH
#define PICO_AUDIO_I2S_SILENCE_BUFFER_SAMPLE_LENGTH PICO_AUDIO_SILENCE_BUFFER_SAMPLE_LENGTH
#else
#define PICO_AUDIO_I2S_SILENCE_BUFFER_SAMPLE_LENGTH 256u
#endif
#endif

// Allow use of pico_audio driver without actually doing anything much
#ifndef PICO_AUDIO_I2S_NOOP
#ifdef PICO_AUDIO_NOOP
#define PICO_AUDIO_I2S_NOOP PICO_AUDIO_NOOP
#else
#define PICO_AUDIO_I2S_NOOP 0
#endif
#endif

#ifndef PICO_AUDIO_I2S_MONO_INPUT
#define PICO_AUDIO_I2S_MONO_INPUT 0
#endif
#ifndef PICO_AUDIO_I2S_MONO_OUTPUT
#define PICO_AUDIO_I2S_MONO_OUTPUT 0
#endif

#ifndef PICO_AUDIO_I2S_DATA_PIN
//#warning PICO_AUDIO_I2S_DATA_PIN should be defined when using AUDIO_I2S
#define PICO_AUDIO_I2S_DATA_PIN 28
#endif

#ifndef PICO_AUDIO_I2S_CLOCK_PIN_BASE
//#warning PICO_AUDIO_I2S_CLOCK_PIN_BASE should be defined when using AUDIO_I2S
#define PICO_AUDIO_I2S_CLOCK_PIN_BASE 26
#endif

// The default order is CLOCK_PIN_BASE=LRCLK, CLOCK_PIN_BASE+1=BCLK
// The swapped order is CLOCK_PIN_BASE=BCLK,  CLOCK_PIN_BASE+1=LRCLK
#ifndef PICO_AUDIO_I2S_CLOCK_PINS_SWAPPED
#define PICO_AUDIO_I2S_CLOCK_PINS_SWAPPED 0
#endif

#ifndef PICO_AUDIO_I2S_CLOCK_DMA_CTL
#define PICO_AUDIO_I2S_CLOCK_DMA_CTL 0
#endif

#ifndef PICO_AUDIO_I2S_CLOCK_DMA_DATA
#define PICO_AUDIO_I2S_CLOCK_DMA_DATA 1
#endif

// todo this needs to come from a build config
/** \brief Base configuration structure used when setting up
 * \ingroup pico_audio_i2s
 */
typedef struct audio_i2s_config {
    uint8_t data_pin;
    uint8_t clock_pin_base;
} audio_i2s_config_t;

void audio_i2s_setup(const audio_i2s_config_t *config,void (*audio_cb)(int16_t*,uint));

