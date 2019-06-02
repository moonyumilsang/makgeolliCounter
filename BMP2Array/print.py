from PIL import Image
import numpy as np
import sys
im = Image.open(sys.argv[1])
p = np.array(im)

for i in range(0, len(p)):
    for j in range(0, len(p[i])/8):
        one = 0
        for k in range(0, 8):
            one += (p[i][j*8 + k] << (7 - k))

        one ^= 0xFF
        text = ("0x%02X" % one)
        sys.stdout.write(text)
        if not (i == len(p) - 1 and j == len(p[i])/8 - 1):
            sys.stdout.write(",")
    sys.stdout.write("\n")

print len(p)*len(p[i])/8
