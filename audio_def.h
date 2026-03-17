#include <stdint.h>
#include <stddef.h>

struct audio_data {
    uint32_t rate;
    uint32_t channels;
    uint32_t bits_per_sample;
    uint32_t points;
    size_t data_size;
    const int16_t *data;
};