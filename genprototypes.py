
h = open('protodecl.inc', 'wt')
c = open('protodef.inc', 'wt')
g = open('protoget.inc', 'wt')

protos = '''
AttachShader
BindBuffer
BindVertexArray
BufferData
CompileShader
CreateProgram
CreateShader
EnableVertexAttribArray
GenBuffers
GenVertexArrays
GetAttribLocation
GetProgramInfoLog
GetProgramiv
GetShaderInfoLog
GetShaderiv
GetUniformLocation
LinkProgram
ShaderSource
Uniform1i
Uniform4fv
UniformMatrix4fv
UseProgram
VertexAttribPointer
'''.split()

for p in protos:
	print('extern PFNGL{0}PROC gl{1};'.format(p.upper(), p), file=h)
	print('PFNGL{0}PROC gl{1};'.format(p.upper(), p), file=c)
	print('gl{1} = (PFNGL{0}PROC) SDL_GL_GetProcAddress("gl{1}");'.format(p.upper(), p), file=g)
