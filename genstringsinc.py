import sys
import zlib
import lzma
import gzip
import bz2
import re
import collections

TIdentifier = collections.namedtuple('TIdentifier', 'txt')
TNumber = collections.namedtuple('TNumber', 'txt')
TSyntax = collections.namedtuple('TSyntax', 'txt')
TNewline = collections.namedtuple('TNewline', 'txt')
TPreproc = collections.namedtuple('TPreproc', 'txt')

class Shrinker(object):

    REIDENTIFIER = re.compile('[0-9a-zA-Z_]+')
    RENUMBER = re.compile('-?[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)?')
    RESYNTAX = re.compile('[-\\[\\]{}()*/+=,#;%<>&|.]')
    REWHITESPACE = re.compile('[ \t\r]*')
    REPREPROC = re.compile('#[^\n\r]+')

    def __init__(self):
        self.tokens = []

    def shrink(self, txt):
        while len(txt):
            l = len(txt)
            txt = self._consume(txt)
            if len(txt) == l:
                print('ENDLESS LOOP AT', l)
                print(txt[:100])
                sys.exit(1)
        shrunk = []
        prevtok = None
        for tok in self.tokens:
            if isinstance(prevtok, TIdentifier) and isinstance(tok, TIdentifier):
                shrunk.append(' ')
            if isinstance(tok, TNewline):
                if (isinstance(prevtok, TSyntax) and prevtok.txt in '{};') or isinstance(prevtok, TPreproc):
                    shrunk.append(tok.txt)
            else:
                shrunk.append(tok.txt)
            prevtok = tok
        return ''.join(shrunk)

    def _consume(self, txt):
        if txt.startswith('//'):
            txt = txt[2:]
            try:
                commentend = txt.index('\n')
            except ValueError:
                return ''
            return txt[commentend:]
        if txt.startswith('/*'):
            txt = txt[2:]
            try:
                commentend = txt.index('*/')
            except ValueError:
                return ''
            return txt[commentend + 2:]
        if txt.startswith('\n'):
            self.tokens.append(TNewline('\n'))
            return txt[1:]
        match = Shrinker.REPREPROC.match(txt)
        if match:
            self.tokens.append(TPreproc(txt[:match.end()]))
            return txt[match.end():]
        match = Shrinker.REIDENTIFIER.match(txt)
        if match:
            self.tokens.append(TIdentifier(txt[:match.end()]))
            return txt[match.end():]
        match = Shrinker.RENUMBER.match(txt)
        if match:
            self.tokens.append(TNumber(txt[:match.end()]))
            return txt[match.end():]
        match = Shrinker.RESYNTAX.match(txt)
        if match:
            self.tokens.append(TSyntax(txt[:match.end()]))
            return txt[match.end():]
        match = Shrinker.REWHITESPACE.match(txt)
        if match:
            return txt[match.end():]
        raise ValueError('Parse error around ``' + txt[:100] + "''")


def testcompression(best, btotal, name, func):
    deflated = func(btotal)
    print(name, ':', len(deflated))
    if best is None or len(deflated) < len(best[1]):
        return (name, deflated)
    else:
        return best

total = 0
bstrings = []
for fname in ('vertex.glsl', 'fragment_pre.glsl', 'fragment_post.glsl'):
    with open(fname, 'rb') as f:
        txt = f.read().decode('ascii', errors='ignore')
        txt = Shrinker().shrink(txt)
        print(txt)
        btxt = txt.encode('ascii', errors='ignore')
        bstrings.append(btxt)
        bstrings.append(b'\x00')

btotal = b''.join(bstrings)
best = None
best = testcompression(best, btotal, 'zlib', lambda btotal: zlib.compress(btotal, 9))
best = testcompression(best, btotal, 'gzip', lambda btotal: gzip.compress(btotal, 9))
best = testcompression(best, btotal, 'bz2', lambda btotal: bz2.compress(btotal, 9))
for i in range(1, 10):
    for j in (0, lzma.PRESET_EXTREME):
        func = lambda btotal: lzma.compress(btotal,
                                            format=lzma.FORMAT_RAW,
                                            filters=[
                                                {'id': lzma.FILTER_LZMA2,
                                                 'preset': i | j}
                                            ])
        best = testcompression(best, btotal,
                               'lzma ' + str(i) + (' regular', ' extreme')[j != 0],
                               func)

name = best[0]
deflated = best[1]
print('best:', name)
with open('shaders.compressed.inc', 'wt') as f:
    f.write('// compressed with ')
    f.write(name)
    f.write('\n')
    f.write('static const uint8_t shaders_vert_fragpre_fragpost[] = { \n')
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
    f.write('static size_t shaders_vert_fragpre_fragpost_sz = ')
    f.write(str(len(deflated)))
    f.write(';\n')

