#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "cg2demo.h"
#include "text.h"

static const uint16_t text_combining_bit = 0x8000;

static const uint16_t text[] = {
	//           teapot
	0x0BD8, 0xFFFF, 0xEFFE, 0xC4A0, 0,
    //  F       L       Y       I         N               G
	0x39A4, 0x4927, 0x5B52, 0x7497, 0x49AC, 0xC924, 0x3967, 0xC120, 0,
    //  T   filler     E       A       P         O            T
    0x7492, 0x8000, 0x39A7, 0x3B7D, 0x3B74, 0x3927, 0xC920, 0x7492, 0,
	// mushroom, mushroom
	0x3E09, 0xCC00, 0x07C1, 0x8980, 0, 
    //  [         M               U               S               H               R               O               O               M             ]
	0x6926, 0x4D64, 0xAC92, 0x4927, 0xC920, 0x39C7, 0xE194, 0x49E4, 0xC924, 0x793C, 0x8904, 0x3927, 0xC920, 0x3927, 0xC920, 0x4D64, 0xAC92, 0x324B, 0,
};

static const size_t ntext = sizeof(text) / sizeof(*text);

static int kerndist(uint16_t prev, uint16_t next) {
    if (next & text_combining_bit) {
        return 3;
    }
    if (!prev || !next) {
        return 0;
    }
    // maximum distance between edges is 6
    // because each cell is 3px wide
    int mindist = 6;
    // each cell is 5 rows
    // the rows are iterated backwards, i.e. upwards
    // because it's easiest to do the checks
    // to the last 3 bits
    for (int i = 0; i < 5; i++) {
        int dist = 0;
        // check bits of prev from right to left
        if (!(prev & 1)) {
            dist++;
            if (!(prev & 2)) {
                dist++;
                if (!(prev & 4)) {
                    dist++;
                }
            }
        }
        // check bits of next from left to right
        if (!(next & 4)) {
            dist++;
            if (!(next & 2)) {
                dist++;
                if (!(next & 1)) {
                    dist++;
                }
            }
        }
        prev >>= 3;
        next >>= 3;
        if (dist < mindist) {
            mindist = dist;
        }
    }
    return 4 - mindist;
}

static void blitchar(uint8_t *img, int stride, uint16_t ch) {
    for (int i = 0; i < 5; i++) {
        img[0] = (ch & 0x4000) ? 0xFF : 0x00;
        img[1] = (ch & 0x2000) ? 0xFF : 0x00;
        img[2] = (ch & 0x1000) ? 0xFF : 0x00;
        img += stride;
        ch <<= 3;
    }
}

static int pow2(int x) {
	if (!(x & (x - 1))) {
		return x;
	}
    while (x & (x - 1)) {
        x = x & (x - 1);
    }
    return x << 1;
}

#if 0
static bool check_neighbour(uint8_t *mydist, uint8_t neighbourdist) {
    if (neighbourdist == 255) {
        return false;
    }
    if (*mydist > neighbourdist + 1) {
        *mydist = neighbourdist + 1;
        return true;
    } else {
        return false;
    }
}
#endif

static const int line_height = 6;

int render_text(struct text_tex *tex) {
    LOGF("Text bytes: %d", (int)sizeof(text));
    int w = 0;
    int h = 0;
    size_t nlines = 0;
    for (size_t i = 0; i < ntext; i++) {
        int linew = 0;
        uint16_t prev = 0;
        for (; i < ntext && text[i]; i++) {
            int dist = kerndist(prev, text[i]);
            //LOGF("line %d, char %d-%d dist: %d", h, (int)(i - 1), (int)i, dist);
            linew += dist;
            prev = text[i];
        }
        linew += 3;
        if (linew > w) {
            w = linew;
        }
        h += line_height;
        nlines++;
    }
    w = pow2(w);
    h = pow2(h);
    if (w < h) {
        w = h;
    }
    if (h < w) {
        h = w;
    }
    LOGF("Text texture width : %d; height: %d", w, h);
    uint8_t *img = (uint8_t *)malloc(w * h);
    if (!img) {
        LOG("Error malloc");
        return 0;
    }
    memset(img, 0, w * h);
    for (size_t line = 0, i = 0; i < ntext && line < nlines; line++, i++) {
        uint16_t prev = 0;
        int linew = 0;
        for (size_t j = i; j < ntext && text[j]; j++) {
            linew += kerndist(prev, text[j]);
            prev = text[j];
        }
        // 0x4924 - leftmost vertical bar
        linew += kerndist(prev, 0x4924);
        int x = (w - linew + 1) / 2;
        int y = (h - nlines * line_height) / 2 + line * line_height;
        prev = 0;
        for (; i < ntext && text[i]; i++) {
            int dist = kerndist(prev, text[i]);
            x += dist;
            blitchar(img + (h - y) * w + x, -w, text[i]);
            prev = text[i];
        }
    }
#if 0
    // unfortunately this is mostly bullshit
    // now compute signed distance fields
    bool changes = false;
    do {
        changes = false;
        for (size_t y = 0; y < h; y++) {
            for(size_t x = 0; x < w; x++) {
                uint8_t *pmydist = img + y * w + x;
                if (x > 0) {
                    changes = changes || check_neighbour(pmydist, img[y * w + (x - 1)]);
                }
                if (x < w) {
                    changes = changes || check_neighbour(pmydist, img[y * w + (x + 1)]);
                }
                if (y > 0) {
                    changes = changes || check_neighbour(pmydist, img[(y - 1) * w + x]);
                }
                if (y < h) {
                    changes = changes || check_neighbour(pmydist, img[(y + 1) * w + x]);
                }
            }
        }
    } while (changes);
#endif
	// bloody awful hack for teapot lid
	img[3103] = 0xFF;
#if !defined(NDEBUG)
	FILE *imgraw = fopen("img.raw", "wb");
	fwrite(img, 1, w * h, imgraw);
	fclose(imgraw);
#endif
    tex->width = w;
    tex->height = h;
    tex->data = img;
    return 1;
}

