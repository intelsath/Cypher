from array import array
import math
import zlib
import os


def add_CRC(write_string):
    CRC32 = zlib.crc32(write_string)
    CRC32 = CRC32 & 0xFFFFFFFF # Convert to unsigned (twos complements to unsigned)

    # Split 4 bytes
    CRC_NBYTES = 4;
    mask = 0xFF000000;   # 4 bytes mask
    n = 0
    buffer = [0,0,0,0]
    for i in range(CRC_NBYTES,0,-1):
        byte_chunkn = CRC32 & mask>>((i-1)*8) # Shift the bits
        byte1 = byte_chunkn>>8*n		# Get the LSBs
        # print byte1, chr(byte1)
        buffer[i-1] = chr(byte1)
        n+=1

    # Append CRC32
    new_string = write_string
    for byte in buffer:
        new_string += byte

    return new_string

def file_test(filename):
    MAX_TXBUFFER = 32
    PC_filename = "D:\\megahash\\" + filename
    filesize = os.path.getsize(PC_filename)
    pages_num = int(math.ceil(filesize/float(MAX_TXBUFFER))) # How many iterations

    print pages_num

    iter_beg = 0
    iter_end = MAX_TXBUFFER
    for x in range(0,pages_num):
        # Open file
        with open(PC_filename, 'rb') as f:
            if pages_num <= 1:
                bytes_tx = f.read()
            else:
                bytes_tx = f.read()[iter_beg:iter_end]
                if x < pages_num-1: # If not last page
                    # Update iterators
                    iter_beg = iter_end
                    iter_end += MAX_TXBUFFER
                else: # las page
                    iter_beg = iter_end
                    iter_end = filesize
        print "|" + bytes_tx

#print add_CRC("h")

def CRCtoSTR(CRC32):
    # Split 4 bytes
    mask = 0xFF000000;   # 4 bytes mask
    n = 0
    buffer = [0,0,0,0] # CRC buffer
    for i in range(4,0,-1): # Split the 4 bytes
        byte_chunkn = CRC32 & mask>>((i-1)*8) # Shift the bits
        byte1 = byte_chunkn>>8*n		# Get the LSBs
        # print byte1, chr(byte1)
        buffer[i-1] = byte1
        n+=1


    CRC_string = 0
    for byte in buffer:
        CRC_string <<= 8
        CRC_string |= byte

    CRC_string = str(CRC_string)

    return CRC_string


CRC32 = zlib.crc32("hello")
CRC32 = CRC32 & 0xFFFFFFFF # Convert to unsigned (twos complements to unsigned)

print CRC32

print CRCtoSTR(CRC32)
