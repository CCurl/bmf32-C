// The simplest block editor possible

#include "dwc-vm.h"

#define COLS 64
#define ROWS 16
#define POS ((cursor_y*COLS) + cursor_x)

cell blockNum, needRedraw, clearStatus;
char buf[COLS * ROWS];

void readBlock() {
    ata_read_block(blockNum*2, buf);
    ata_read_block((blockNum*2)+1, buf+512);
}

void writeBlock() {
    ata_write_block(blockNum*2, buf);
    ata_write_block((blockNum*2)+1, buf+512);
}

void gotoXY(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    vga_set_cursor(x, y);
}

void showStatus(char *status) {
    int x = cursor_x, y = cursor_y;
    gotoXY(0, ROWS);
    if (status == NULL) {
        clearStatus = 0;
        for (int i = 0; i < COLS; ++i) { emit(32); }
    } else {
        zType(status);
    }
    gotoXY(x, y);
}

void moveAbs(int x, int y) {
    cursor_x = min(max(x, 0), COLS - 1);
    cursor_y = min(max(y, 0), ROWS - 1);
    gotoXY(cursor_x, cursor_y);
}

void move(int dx, int dy) {
    moveAbs(cursor_x + dx, cursor_y + dy);
}

void insert() {
    int pos = POS;
    int off = (ROWS*COLS)-1;
    while (off > pos) {
        buf[off] = buf[off-1];
        --off;
    }
    buf[pos] = ' ';
}

void replace() {
    showStatus("-replace-");
    while (1) {
        int k = key();
        if (k == 27) break; // ESC key to exit replace mode
        if (k == 10) { moveAbs(0, cursor_y + 1); }
        if (k == 8) { move(-1, 0); }
        if (btwi(k,32,126)) {
            buf[POS] = k;
            emit(k);
            if (cursor_x >= COLS) { moveAbs(0, cursor_y + 1); }
        }
    }
    showStatus(NULL);
}

void delete() {
    int pos = POS;
    int end = ROWS*COLS-1;
    while (pos < end) {
        buf[pos] = buf[pos+1];
        ++pos;
    }
    buf[end] = 32; // Clear the last character after deletion
}

int process() {
    int k = key();
    if (clearStatus) { showStatus(NULL); }
    switch (k) {
        case 'h': move(-1, 0);    // left
        BCASE 'j': move(0, 1);    // down
        BCASE 'k': move(0, -1);   // up
        BCASE 'l': move(1, 0);    // right
        BCASE 'i': insert(); needRedraw = 1;
        BCASE 'R': replace();
        BCASE 'x': delete(); needRedraw = 1;
        BCASE  8: if (cursor_x > 0) { move(-1, 0); delete(); }
        BCASE 10: moveAbs(0, cursor_y + 1);
        BCASE 19: writeBlock(); showStatus("-saved-"); clearStatus = 1;
        BCASE 17: return 1;
        default: break;
    }
    return 0;
}

void reDraw() {
    int x = cursor_x, y = cursor_y, pos = 0;
    gotoXY(0, 0);
    needRedraw = 0;
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLS; ++x) {
            emit(buf[pos++]);
        }
        emit(10);
    }
    gotoXY(x, y);
}

void edit() {
    blockNum = pop();
    readBlock();
    vga_clear();
    needRedraw = 1;
    clearStatus = 0;
    while (1) {
        if (needRedraw) { reDraw(); }
        if (process()) break;
    }
    gotoXY(0, ROWS);
}
