#include "dwc-vm.h"
#include "boot.h"

// ==================================================
void repl() {
    char *tib = (char *)(last-1024);
    if (state != COMPILE) { state = INTERPRET; }
    zType((state == COMPILE) ? " ... "  : " ok\n");
    push((cell)tib); push(256); outer("accept");
    if (pop()) {
        emit(' ');
        outer(tib);
    }
}

void dwcRun() {
    dwcInit();
    outer(DWC_SRC);
    do { repl(); } while (1);
}

// ==================================================
int strlen(const char *s) {
    int len = 0;
    while (s && s[len] != '\0') { len++; }
    return len;
}

char *strcpy(char *dest, const char *src) {
    char *out = dest;
    while ((*dest++ = *src++) != '\0') { }
    return out;
}

int strEqI(const char *a, const char *b) {
    if (!a || !b) { return a == b; }
    while (*a && *b) {
        unsigned char ca = (unsigned char)*a;
        unsigned char cb = (unsigned char)*b;
        if (ca >= 'A' && ca <= 'Z') { ca = (unsigned char)(ca + 32); }
        if (cb >= 'A' && cb <= 'Z') { cb = (unsigned char)(cb + 32); }
        if (ca != cb) { return 0; }
        ++a; ++b;
    }
    return *a == *b;
}

// IDK why the build fails without memcpy because we don't call it.
void *memcpy(void *dest, const void *src, cell num) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    for (cell i = 0; i < num; ++i) { d[i] = s[i]; }
    return dest;
}

void *memmove(void *dest, const void *src, cell num) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    if (d == s || num == 0) { return dest; }
    if (d < s) {
        for (cell i = 0; i < num; ++i) { d[i] = s[i]; }
    } else {
        for (cell i = num; i > 0; --i) { d[i - 1] = s[i - 1]; }
    }
    return dest;
}

int key(void) {
    int c = -1;
    while (c < 0) { c = keyboard_get_char(); }
    return c;
}
