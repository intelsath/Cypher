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
        filename_orig = sys.argv[1]
        # Was the file given as a path or just the filename?
        im = 0
        filename_path = ""
        if "/" in filename_orig:
            filename_path = filename_orig.split("/")
            self.filename = filename_path[len(filename_path)-1]
            im = Image.open(filename_orig) #Can be many different formats.
        else:
            self.filename = filename_orig
            im = Image.open(self.filename) #Can be many different formats.

        self.pix = im.load()
        # Dimensions
        self.width, self.height = im.size

        # Transparency settings
        # NOTE: only supported in png images (remove antialiasing)
        self.alphahexcode = 0x00FCFCFC # Default transparency -- white in RGB666 (uint32t)
        self.cchexcode = 0x00EC1C24 # Default color change hex code  -- Red in RGB666 (uint32t)
        self.overridetransparency = 0  # Does this image support transparency?
        if( len(sys.argv) > 2 ):
            self.overridetransparency = int(sys.argv[2])
        if( len(sys.argv) > 3 ):
            self.alphahexcode = sys.argv[3]
        if( len(sys.argv) > 4 ):
            self.cchexcode = sys.argv[3]

    # Get color in RGB888 format
    def get_RGB888(self,pixelx,pixely):
        RGB = self.pix[pixelx,pixely] # Since we are using monochromatic pictures, we will just use one of the values, e.g the R value
        return RGB

    # get color in RGB666 format (THREE bytes)
    def RGB666(self,RGB):
        redbyte = RGB[0]
        greenbyte = RGB[1]
        bluebyte = RGB[2]

        msk = 0xFC
        # RGB666
        r = ((redbyte & msk)) << 16
        g = ((greenbyte & msk)) << 8
        b = (bluebyte & msk)
        return (r | g | b);

    def save_info(self):
        try:
            conn = sqlite3.connect('../../cypher.db')

            # Does this image support transparency?
            extension = self.filename[-4:]
            pictype = ""
            if len(self.filename) > 5:
                pictype = self.filename[:5]
            transparency = 0
            alpha_hexcode = 0x00000000
            colorchange = 0
            cc_hexcode = 0x00000000
            if self.overridetransparency != -1 and extension == ".png": # File with transparency
                transparency = 1
                alpha_hexcode = self.alphahexcode
            if self.overridetransparency == -1 and extension == ".png":
                print "Overriding transparency..."
            if pictype == "font_":
                colorchange = 1
                cc_hexcode = self.cchexcode

            c = conn.cursor()
            # Insert a row of data
            sql = "INSERT INTO images(Filename,Height,Width,Transparency,AlphaHexCode,ColorChange,CCHexCode) VALUES ('" + self.filename[:-4] + "','" + str(self.height) + "','" + str(self.width) + "','" + str(transparency) + "','" + str(alpha_hexcode) + "','" + str(colorchange) + "','" + str(cc_hexcode) + "')"
            c.execute(sql)
            # Save (commit) the changes
            conn.commit()
            conn.close()
        except:
            print "Warning: Data could not be saved to SQL db."

    # Iterate through the whole image
    def scan_image(self):
        iter = 0;
        iter_x = 0
        iter_y = 0
        iter_max = self.width*self.height

        # Write to file
        print "saving file..."
        f = open("../rgb666/" + self.filename[:-4],'wb+')
        f2 = open('hexdata.txt','w+')
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
            RGB666 = self.RGB666(RGB)

            # Write to file (split the two bytes)
            # MSB goes first then LSB
            msk = 0x0000FF
            byte1 = RGB666 & msk
            byte2 = (RGB666 & (msk<<8))>>8
            byte3 = (RGB666 & (msk<<16))>>16
            f.write(chr(byte3))
            f.write(chr(byte2))
            f.write(chr(byte1))
            #print hex(RGB666)
            # Update iterator
            iter = iter+1

            #print lsbbyte
            #print msbbyte

            strhex = str(hex(RGB666))
            f2.write(strhex[2:])


        f2.close()
        f.close()




def main():
    pichandler = PICHEX()

    pichandler.scan_image()
    # Save info file
    pichandler.save_info()

if __name__ == "__main__":
    main()
