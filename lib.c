#include "dwc-vm.h"

// ==================================================
int strlen(const char *s) {
    int len = 0;
    while (s && s[len] != '\0') { len++; }
    return len;
}

char *strcpy(char *dest, const char *src) {
    char *out = dest;
    while ((*(dest++) = *(src++)) != '\0') { }
    return out;
}

int strEqI(const char *a, const char *b) {
    if (!a || !b) { return a == b; }
    while (*a && *b) {
        unsigned char ca = (unsigned char)*a;
        unsigned char cb = (unsigned char)*b;
        if (btwi(ca,'A','Z')) { ca += 32; }
        if (btwi(cb,'A','Z')) { cb += 32; }
        if (ca != cb) { return 0; }
        ++a; ++b;
    }
    return *a == *b;
}

void *memcpy(void *dest, const void *src, cell num) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    if (d == s || num == 0) { return dest; }
    if (d < s) {
        for (cell i = 0; i < num; ++i) { *(d++) = *(s++); }
    } else {
        for (cell i = num-1; i >= 0; --i) { d[i] = s[i]; }
    }
    return dest;
}
