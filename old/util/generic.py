# -*- coding: utf-8 -*-
from __future__ import absolute_import, division, generators, print_function, with_statement


import os
import sys
import numpy
import OpenGL
import OpenGL.GL
import ctypes
from itertools import repeat
from OpenGL import GL


def iapply(func, *iterables):
    '''iapply(func, p, q, ...):
    calls func(p[0], q[0], ...)
    until the shortest iterable is exhausted.
    Like reduce without collecting the results.'''
    if len(iterables) == 0:
        return

    if not callable(func):
        raise ValueError('{0} is not a callable'.format(func))

    for args in zip(*iterables):
        func(*args)


def repeat_each(items, repeats):
    '''Iterate items, repeatedly yielding each item the corresponding times.'''
    for i, r in zip(items, repeats):
        for j in repeat(i, r):
            yield j


def arrayify(x):
    '''Constructs a float array from x'''
    return numpy.array(x, dtype=float)


def find_datadir():
    '''
    Returns an absolute path to the 'data' directory.
    Source and data should be laid out like so:
        +/
        |
        +--+ data
        |  | ... files ...
        |
        +--+ src
           | sys.argv[0]
    '''
    return os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), '..', 'data'))


