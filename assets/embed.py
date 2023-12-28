import sys
import struct
import pyperclip

path = sys.argv[1]
data = bytearray(open(path, "rb").read())
string = ','.join([hex(elem) for elem in list(data)])

pyperclip.copy(string)
print('Copied data to clipboard.')