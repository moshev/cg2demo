h = open('protodecl.inc', 'wt')
c = open('protodef.inc', 'wt')
g = open('protoget.inc', 'wt')

with open('glprotos.txt') as f:
    protos = f.read().split()

for f in (h, c, g):
    print('#if !defined(__MACOSX__)', file=f)

for p in protos:
    print('extern PFNGL{0}PROC gl{1};'.format(p.upper(), p), file=h)
    print('PFNGL{0}PROC gl{1};'.format(p.upper(), p), file=c)
    print('gl{1} = (PFNGL{0}PROC) SDL_GL_GetProcAddress("gl{1}");'.format(p.upper(), p), file=g)

for f in (h, c, g):
    print('#endif', file=f)

