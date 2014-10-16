#if !defined(CG2_SHADERS_H)
#define CG2_SHADERS_H

// all return 1 on success, 0 on failure
int get_vertex_shader(const char **vs, size_t *vssz);
int get_fragment_shader_pre(const char **fspre, size_t *fspresz);
int get_fragment_shader_post(const char **fspost, size_t *fspostsz);
int get_fragment_shader_text(const char **fstext, size_t *fstextsz);

#endif

