/* SPDX-License-Identifier: BSD-3-Clause */
#include <string.h>
#include "bondwell.h"
#include "simglb.h"

void bw_key(unsigned matrix, bool shift, bool control)
{
    static const unsigned order[9] = {6, 3, 1, 0, 2, 4, 5, 7, 8};
    if (matrix >= 90) return;
    unsigned code = matrix > 63 ? 0x100 | (matrix - 64) : matrix;
    code |= (shift ? 0x40 : 0) | (control ? 0x80 : 0);
    for (unsigned i = 0; i < 9; ++i) bw.key_bits[i] = (code >> order[i]) & 1;
    bw.key_shift = 0;
    bw.key_ready = true;
    bw.key_release = T + 200000; /* host-generated key pulse, 50 ms */
    if (bw.pia[3] & 2) bw.pia[3] |= 0x80;
}
void bw_key_up(void)
{
    if (bw.key_ready && !(bw.pia[3] & 2)) bw.pia[3] |= 0x80;
    bw.key_ready = false; bw.key_release = 0;
}
void bw_queue_key(unsigned matrix, bool shift, bool control)
{
    unsigned next = (bw.key_tail + 1) % 64;
    if (matrix >= 90 || next == bw.key_head) return;
    bw.key_queue[bw.key_tail] = matrix | (shift ? 256 : 0) | (control ? 512 : 0);
    bw.key_tail = next;
}
void bw_keyboard_tick(void)
{
    if (bw.key_ready && bw.key_release && T >= bw.key_release) {
        bw_key_up();
        bw.key_next = T + 200000;
    }
    if (!bw.key_ready && T >= bw.key_next && bw.key_head != bw.key_tail) {
        unsigned key = bw.key_queue[bw.key_head];
        bw.key_head = (bw.key_head + 1) % 64;
        bw_key(key & 255, (key & 256) != 0, (key & 512) != 0);
    }
}
bool bw_key_ascii(unsigned ch)
{
    static const char *rows[9] = {
        "7890123456", "uiopqwerty", "\0\0\r\0 \0\0\0\0\0", "",
        "\0\0\0\b@\0-]\0\0", "\0\0\0\n\177\t^[\0\0",
        "m,./zxcvbn", "jkl;asdfgh", "\0-\0\0\033\0:\0\0\0"
    };
    bool control = ch > 0 && ch < 27 && ch != 8 && ch != 9 && ch != 10 && ch != 13;
    bool shift = ch >= 'A' && ch <= 'Z';
    if (control) ch += 'a' - 1;
    if (shift) ch += 'a' - 'A';
    const char *shifted = "!\"#$%&'()_<>?+*={}~\\";
    const char *normal  = "123456789-,./;:-[]^@";
    const char *p = ch && ch < 128 ? strchr(shifted, (int)ch) : NULL;
    if (p) { ch = (unsigned char)normal[p - shifted]; shift = true; }
    if (!ch) return false;
    for (unsigned row = 0; row < 9; ++row) {
        if (row == 3) continue;
        for (unsigned col = 0; col < 10; ++col) {
            if ((unsigned char)rows[row][col] == ch) {
                bw_queue_key(row * 10 + col, shift, control); return true;
            }
        }
    }
    return false;
}
