# How to use it:
# Put a set of images (all images that share the same settings) in a single folder
# Call this script with the path as an argument
# Then all the images inside that folder will be processed automatically
# e.g All transparent pictures in one folder, all png files that have NO transparency in another folder, all fonts images
# in another folder, and so on... Another example :  fonts located in : D:\megahash\hashbank\LCD\images\assets\fonts\montserrat can be processed with the following str_script
# python setallimages.py "D:\megahash\hashbank\LCD\images\assets\fonts\montserrat"

import sys
import os
# Dir
path = sys.argv[1]

# Other settings
if( len(sys.argv) > 2 ):
    overridetransparency = int(sys.argv[2])
if( len(sys.argv) > 3 ):
    alphahexcode = sys.argv[3]
if( len(sys.argv) > 4 ):
    cchexcode = sys.argv[3]

# Get all files in the specified folder
from os import listdir
from os.path import isfile, join
onlyfiles = [f for f in listdir(path) if isfile(join(path, f))]

# List all files
for file in onlyfiles:
    filename = path + "/" + file
    str_cmd = "python bmptohex.py \"" + filename + "\""
    # Add args
    if( len(sys.argv) > 2 ):
        str_cmd += " " + str(overridetransparency)
    if( len(sys.argv) > 3 ):
        str_cmd += " " + str(alphahexcode)
    if( len(sys.argv) > 4 ):
        str_cmd += " " + str(cchexcode)

    print str_cmd
    os.system(str_cmd)
