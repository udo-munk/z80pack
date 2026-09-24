/* SPDX-License-Identifier: BSD-3-Clause */
#include <string.h>
#include "bondwell.h"
#include "simglb.h"
void bw_render(BYTE pixels[640 * 256], unsigned *height)
{
    unsigned columns = bw.crtc[1], rows = bw.crtc[6], lines = (bw.crtc[9] & 31) + 1;
    unsigned start = ((bw.crtc[12] << 8) | bw.crtc[13]) & 0x7ff;
    unsigned cursor = ((bw.crtc[14] << 8) | bw.crtc[15]) & 0x7ff;
    unsigned mode = (bw.crtc[10] >> 5) & 3;
    bool cursor_on = mode != 1;
    if (mode >= 2) cursor_on = ((T / (mode == 2 ? 1000000 : 2000000)) & 1) == 0;
    if (columns > 80) columns = 80;
    if (rows * lines > 256) rows = 256 / lines;
    *height = rows * lines;
    if (!*height) *height = 225;
    memset(pixels, 0, 640 * 256);
    for (unsigned row = 0; row < rows; ++row) {
        for (unsigned col = 0; col < columns; ++col) {
            unsigned address = (start + row * columns + col) & 0x7ff;
            BYTE character = bw.common[0x7800 + address];
            for (unsigned line = 0; line < lines; ++line) {
                BYTE bits = bw.chargen[character * 16 + (line & 15)];
                if (cursor_on && address == cursor && line >= (bw.crtc[10] & 31) &&
                    line <= (bw.crtc[11] & 31)) bits = 0xff;
                for (unsigned b = 0; b < 8; ++b)
                    pixels[(row * lines + line) * 640 + col * 8 + b] = (bits >> (7 - b)) & 1;
            }
        }
    }
}
int bw_screenshot(const char *path)
{
    BYTE pixels[640 * 256]; unsigned height;
    bw_render(pixels, &height);
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); return -1; }
    fprintf(f, "P6\n640 %u\n255\n", height);
    for (unsigned i = 0; i < 640 * height; ++i) {
        BYTE rgb[3] = {pixels[i] ? 255 : 12, pixels[i] ? 184 : 9, pixels[i] ? 64 : 3};
        if (fwrite(rgb, 1, 3, f) != 3) { fclose(f); return -1; }
    }
    return fclose(f) ? -1 : 0;
}
void bw_text(FILE *out)
{
    unsigned cols = bw.crtc[1], rows = bw.crtc[6];
    unsigned start = (bw.crtc[12] << 8) | bw.crtc[13];
    if (cols > 80) cols = 80;
    if (rows > 32) rows = 32;
    for (unsigned r = 0; r < rows; ++r) {
        for (unsigned c = 0; c < cols; ++c) {
            BYTE v = bw.common[0x7800 + ((start + r * cols + c) & 0x7ff)];
            fputc(v >= 32 && v < 127 ? v : ' ', out);
        }
        fputc('\n', out);
    }
}
