#if !defined(CG2_TEXT_H)
#define CG2_TEXT_H

struct text_tex {
    size_t height;
    size_t width;
    uint8_t *data;
};

int render_text(struct text_tex *tex);

#endif

