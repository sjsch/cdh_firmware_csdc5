#!/usr/bin/env python3

import serial
from time import sleep, time
from random import randint
from os import urandom
import cffi
import sys

start_time = time()

class Tee(object):
    def __init__(self, name, mode):
        self.file = open(name, mode)
        self.stdout = sys.stdout
        sys.stdout = self

    def __del__(self):
        sys.stdout = self.stdout
        self.file.close()

    def write(self, data):
        self.file.write(data)
        self.stdout.write(data)

    def __enter__(self):
        pass

    def __exit__(self, _type, _value, _traceback):
        pass

ffi = cffi.FFI()

tee = Tee(sys.argv[1], 'w')

def t(m):
    print(time() - start_time, m, file=tee)

ffi.cdef("""
int hydro_hash_hash(uint8_t *out, size_t out_len, const void *in_, size_t in_len,
                    const char    *ctx,
                    const uint8_t *key);
""")
lib = ffi.dlopen("libhydrogen.so")

global_context = ffi.new('uint8_t[]', init=b'CONTEXT!')

def hash(in_bytes):
    in_buf = ffi.from_buffer('uint8_t[]', in_bytes)
    out_buf = ffi.new('uint8_t[32]')
    lib.hydro_hash_hash(out_buf, 32, in_buf, len(in_bytes),
                        global_context, ffi.NULL)
    return bytes(out_buf)

ser = serial.Serial('/dev/ttyACM0', 115200)
sleep(0.5)
ser.write(b'comp')

sleep(0.5)
t(ser.readline())
sleep(0.5)

rounds = 10000

while 1:
    bs = urandom(8)
    expect = hash(bs)
    for i in range(rounds):
        expect = hash(expect)
    t(f'sending {bs.hex()}')
    ser.write(bs)
    t(f'expect {expect.hex()}')
    t(ser.readline())
    t(ser.readline())
    t(ser.readline())
    t(f'A: {ser.read(32).hex()}')
    t(f'B: {ser.read(32).hex()}')
    t(f'C: {ser.read(32).hex()}')
    sleep(0.2)

ser.close()
tee.close()
