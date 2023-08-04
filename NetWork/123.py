import sys
import struct

R1 = b'='     # 61     3D
R2 = b'\xfb'  # 251
R3 = b'\xe7'  # 231
R4 = b'm'     # 109

c1 = int.from_bytes(R1, byteorder='big')
print(c1)
c2 = int.from_bytes(R2, byteorder='big')
print(c2)
c3 = int.from_bytes(R3, byteorder='big')
print(c3)
c4 = int.from_bytes(R4, byteorder='big')
print(c4)