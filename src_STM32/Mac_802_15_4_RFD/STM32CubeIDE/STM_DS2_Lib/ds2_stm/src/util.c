#include "util.h"

uint8_t cmp(const uint8_t* a, const uint8_t* b, size_t len) {
    for (size_t i = 0; i < len; i++)
        if (a[i] != b[i])
            return 0;

    return 1;
}
