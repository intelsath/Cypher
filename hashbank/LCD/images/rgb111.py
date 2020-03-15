from PIL import Image # pip install pillow
import math
import sys
import pyperclip
import sqlite3


class PICHEX:

    filename = ""
    pix = 0
    width = 0
    height = 0
    def __init__(self):
        self.filename = sys.argv[1]
        im = Image.open(self.filename) #Can be many different formats.
        self.pix = im.load()
        # Dimensions
        self.width, self.height = im.size

    # Get color in RGB888 format
    def get_RGB888(self,pixelx,pixely):
        RGB = self.pix[pixelx,pixely] # Since we are using monochromatic pictures, we will just use one of the values, e.g the R value
        return RGB

    # get color in RGB666 format (THREE bytes)
    def RGB111(self,RGB):
        redbyte = RGB[0]
        greenbyte = RGB[1]
        bluebyte = RGB[2]

        r = 0
        g = 0
        b = 0
        if redbyte:
            r = 0x04
        if greenbyte:
            g = 0x02
        if bluebyte:
            b = 0x01

        return (r | g | b);

    # Iterate through the whole image
    def scan_image(self):
        iter = 0;
        iter_x = 0
        iter_y = 0
        iter_max = self.width*self.height

        # Write to file
        print "saving file..."
        f2 = open('hexdata.txt','w+')

        all_111_bytes = []
        while(iter < iter_max):
            # Calculate position
            # Y
            rows = math.floor(iter / self.width)
            iter_y = rows
            # X
            cols = iter - (iter_y*self.width)
            iter_x = cols

            # Get RGB
            RGB = self.get_RGB888(iter_x,iter_y)
            #print "RGB: "
            #for byte in RGB:
            #    print hex(byte),
            #print "->",

            # Conversion
            RGB111 = self.RGB111(RGB)

            # Write to file (split the two bytes)
            # MSB goes first then LSB
            all_111_bytes.append(RGB111)
            #print hex(RGB666)
            # Update iterator
            iter = iter+1

            #print lsbbyte
            #print msbbyte



        # Append bytes
        i = 0
        x = 0
        while True:
            mask = 0x07
            byte = (all_111_bytes[i]&mask)<<3 | (all_111_bytes[i+1]&mask) 

            strhex = str(byte) + ","
            #f2.write(strhex[2:])
            f2.write(strhex)
            i += 2
            x += 1
            if i>=len(all_111_bytes)-1:
                break
        # print x
        f2.close()




def main():
    pichandler = PICHEX()

    pichandler.scan_image()


if __name__ == "__main__":
    main()
