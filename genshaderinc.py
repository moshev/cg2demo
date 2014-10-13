import zlib
import lzma
import gzip
import bz2
import re

class Shrinker(object):

	COMMENTLINE = 1
	COMMENTMULTI = 2
	NOCOMMENT = 0

	def __init__(self):
		self.comment = Shrinker.NOCOMMENT
		self.prevchar = ''
		self._consume = self.consume_state_empty

	def consume(self, char):
		ret = self._consume(char)
		self.prevchar = char
		return ret

	def consume_state_empty(self, char):
		if char in ' \r\n\t':
			return ''
		else:
			self._consume = self.consume_state_nonempty
			return self.consume_state_nonempty(char)

	def consume_state_nonempty(self, char):
		if char in ' \t\r':
			return ''
		if char == '\n':
			self._consume = self.consume_state_empty
		if self.prevchar == '/':
			if char == '/':
				self._consume = self.consume_state_comment_line
				return ''
			if char == '*':
				self._consume = self.consume_state_comment_block
				return ''
		return char

	def consume_state_comment_line(self, char):
		if char == '\n':
			self._consume = self.consume_state_empty
		return ''

	def consume_state_comment_block(self, char):
		if self.prevchar == '*' and char == '/':
			self._consume = self.consume_state_empty
		return ''

total = 0
bstrings = []
for fname in ('vertex.glsl', 'fragment_pre.glsl', 'fragment_post.glsl'):
	with open(fname, 'rb') as f:
		txt = f.read().decode('ascii', errors='ignore')
	txt = ''.join(map(Shrinker().consume, txt))
	btxt = txt.encode('ascii', errors='ignore')
	bstrings.append(btxt)
	bstrings.append(b'\x00')

btotal = b''.join(bstrings)
#deflated = zlib.compress(btxt, 9)
deflated = lzma.compress(btxt, format=lzma.FORMAT_RAW, filters=[
	{'id':lzma.FILTER_LZMA2, 'preset':9 | lzma.PRESET_EXTREME}])
#deflated = bz2.compress(btotal, 9)
print(fname, 'compressed: ', len(deflated))

with open('shaders.lzma.inc', 'wt') as f:
	f.write('static const uint8_t shaders_vert_fragpre_fragpost_lzma[] = { \n')
	l = 0
	for x in (format(b, '02x') for b in deflated):
		if l >= 76:
			f.write('\n')
			l = 0
		f.write('0x')
		f.write(x)
		f.write(', ')
		l += 6
	f.write('\n};\n')
