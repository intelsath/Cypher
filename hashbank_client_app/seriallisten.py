import shlex
import subprocess
import serial
import struct
import math
import zlib
import time
import json
import sys
import os

class SerialListen:
    serial = serial.Serial("COM21", 115200, timeout=1)

    initial_handshake = False # Did we get a correct handshsake
    # Commands
    PC_COMMANDS = {
        'PC_HNDSHK': 'PC_HNDSHK',
        'PC_RSNDTA': 'PC_RSNDTA',
        'PC_INFOOK': 'PC_INFOOK',
        'PC_SNDFLE': 'PC_SNDFLE',
        'PC_CRTADR': 'PC_CRTADR',
        'PC_RQSADR': 'PC_RQSADR',
        'PC_CRTWLT': 'PC_CRTWLT',
        'PC_SGNTRN': 'PC_SGNTRN',
        'PC_NFCRAD': 'PC_NFCRAD',
        'PC_NFCWRT': 'PC_NFCWRT',
        'PC_RSTRWL': 'PC_RSTRWL',
        'PC_UPDATE': 'PC_UPDATE'
    }
    HB_COMMANDS = {
        'HB_HNDSHK': 'HB_HNDSHK',
        'HB_INFOOK': 'HB_INFOOK',
        'HB_RSNDTA': 'HB_RSNDTA'
    }
    # Buffer size
    DEFAULT_BUFFERTXRX = 9
    rx_buffer = DEFAULT_BUFFERTXRX
    tx_buffer = DEFAULT_BUFFERTXRX
    # CRC 32 bits
    CRC_NBYTES = 4
    # Max TX buffer_size
    MAX_TXBUFFER = 1024 # Max buffer of 32 bytes

    # Methods
    def compute_CRC32(self,stringchk):
        CRC32 = zlib.crc32(stringchk)
        CRC32 = CRC32 & 0xFFFFFFFF # Convert to unsigned (twos complements to unsigned)

        return CRC32

    def CRCtoSTR(self,CRC32):
        # Split 4 bytes
        mask = 0xFF000000;   # 4 bytes mask
        n = 0
        buffer = [0,0,0,0] # CRC buffer
        for i in range(self.CRC_NBYTES,0,-1): # Split the 4 bytes
            byte_chunkn = CRC32 & mask>>((i-1)*8) # Shift the bits
            byte1 = byte_chunkn>>8*n		# Get the LSBs
            # print byte1, chr(byte1)
            buffer[i-1] = byte1
            n+=1

        CRC_string = ""
        for byte in buffer:
            CRC_string += str(chr(byte))

        # print CRC_string
        return CRC_string

    # Addd CRC32 at the end of the string
    def add_CRC(self,write_string):
        CRC32 = self.compute_CRC32(write_string)
        CRC_string = self.CRCtoSTR(CRC32) # split the bytes and return a string of the CRC to append it at the end

        # Append CRC32
        new_string = write_string + CRC_string

        return new_string # String with CRC32 added

    def verify_CRC(self,received_data):
        data_rx = received_data[0:len(received_data)-self.CRC_NBYTES]
        original_buf = len(data_rx)

        CRC_stringRX = 0
        for i in range(original_buf,original_buf+self.CRC_NBYTES):
            CRC_stringRX <<= 8
            CRC_stringRX |= ord(received_data[i])

        CRC_computed = self.compute_CRC32(data_rx) # New CRC (to compare)

        # print CRC_computed
        # print CRC_stringRX
        if CRC_computed == CRC_stringRX: # Do both CRCs match?
            valid_data = True
        else:
            valid_data = False

        return (valid_data,data_rx) # Is data valid? And data received (without crc)

    def data_validate(self):
    	wrstr = self.PC_COMMANDS["PC_HNDSHK"]

    	# uart transmit data
    	self.uart_transfer_data(wrstr)
    	#if tx_status:
    		# cout << "INFOOK was sent succesfully!" << endl;
        #else:
    		#cout << "Error when sending Handshake to serial port." << endl;

    def request_repeatdata(self):
    	wrstr = self.PC_COMMANDS["PC_RSNDTA"]
    	self.uart_transfer_data(wrstr)

    # Expect some data (use this after we KNOW for sure PC will be sending some data to HB)
    def uart_rxcommunicate(self):
        rx = 0
        while rx <= 0:
            expecting_rx = 1
            (valid_data,data_rx) = self.uart_receive_data()

            rx = valid_data
            # If I received something
            if rx == 1:
                self.data_validate();   # Data was OK
            elif rx == 0:
                self.request_repeatdata(); # Data was corrupted. Request data to be re-sent
                # Flush contents!
                self.serial.flushInput()
                self.serial.flushOutput()

        return data_rx

    def uart_receive_data(self):
        buffer_size = self.rx_buffer + self.CRC_NBYTES # buffer size in bytes
        while True:
            data = self.serial.read(buffer_size)
            #if not null then...
            valid_data = 0
            data_rx = ""
            if(data):
                #print "Received data: " + data
                (valid_data,data_rx) = self.verify_CRC(data)
                # print "valid_data: " + str(valid_data)
                # print "Received data: " + data_rx
                all_bytes = [ord(c) for c in data]
                # print "Received: '0x" + "".join("{:02x}".format(c) for c in all_bytes) + "'..."
                break
            else:
                continue
        return (valid_data,data_rx)

    def uart_transfer_data(self,write_string):
        snduart_string = self.add_CRC(write_string)

        # Send string as unsigned chars (aka bytes in the form 0-255)
        fmt = "B" * len(snduart_string)
        all_bytes = [ord(c) for c in snduart_string]
        byte_array = struct.pack(fmt,*all_bytes)

        # print "sending '0x" + "".join("{:02x}".format(c) for c in all_bytes) + "'..."

        self.serial.write(byte_array)
    # HB protocl UART
    def start(self):
        #print "Expecting Initial Handshake..."
        while not self.initial_handshake:
            (valid_data,data_rx) = self.uart_receive_data()
            if valid_data:
                self.uart_transfer_data(self.PC_COMMANDS["PC_HNDSHK"])
                self.initial_handshake = True
        #print "First Handshake Received!"

    def uart_txcommunicate(self,tx_string):
        handshake_received = False
        sent_once = False
        while not handshake_received:
            if not sent_once:
                self.uart_transfer_data(tx_string)
                sent_once = True
            (valid_data,data_rx) = self.uart_receive_data()
            if valid_data:
                # Did we get correct data?
                if data_rx == self.HB_COMMANDS["HB_INFOOK"]:
                    handshake_received = True
                elif data_rx == self.HB_COMMANDS["HB_RSNDTA"]:
                    # self.serial.flushInput()
                    # self.serial.flushInput()
                    # self.serial.flushOutput()
                    # time.sleep(1)
                    print "Resending last buffer..."
                    self.uart_transfer_data(tx_string) # Resend data as requested!
            else:
                # Data is not valid... Do nothing (wait for handhsake (wait til timeout))
                print "Waiting for timeout..."
                # self.serial.flushInput()
                # self.serial.flushOutput()


    def changerx_buffer(self):
    	# cout << "Expecting new buffer..." << endl;
    	rx_bufsize = self.uart_rxcommunicate()
        self.rx_buffer = int(rx_bufsize)
        # print "Next buffer to expect: " + str(self.rx_buffer)


    def change_txbuffer(self,str2send):
        # Make sure there are 9 (default buffer) characters
        nxt_buffer = str(len(str2send))
        # nxt_buffer = str(unichr(buf)) # convert to byte equivalent
        left_chrs = self.DEFAULT_BUFFERTXRX-len(nxt_buffer) # characters to fill
        concat_chrs = ""
        for x in range(left_chrs):
            concat_chrs += "0"
        nxt_buffer = concat_chrs + nxt_buffer

        return nxt_buffer

    def default_txbuffer(self):
        self.tx_buffer = self.DEFAULT_BUFFERTXRX

    def default_rxbuffer(self):
        self.rx_buffer = self.DEFAULT_BUFFERTXRX

    def read_in_chunks(file_object, chunk_size=1024):
        """Lazy function (generator) to read a file piece by piece.
        Default chunk size: 1k."""
        while True:
            data = file_object.read(chunk_size)
            if not data:
                break
            yield data

    def send_command(self,k):
        if k=="1": # File (send file!!)
            #print "Sending file..."
            self.uart_txcommunicate(self.PC_COMMANDS["PC_SNDFLE"])

            # filename = "hashbank.zip"
            # PC_filename = "D:\\megahash\\" + filename
            # HB_filename = "/home/debian/hashbank/UART/" + filename
            # Get filename and save directory from arguments
            PC_filename = sys.argv[2]
            HB_filename = sys.argv[3]

            #print PC_filename + " to " + HB_filename
            # Make sure there are 9 (default buffer) characters
            nxt_buffer = self.change_txbuffer(HB_filename)

            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(HB_filename)   # Send filename

            self.default_txbuffer(); # Set default buffer

            # Calculate file size
            filesize = os.path.getsize(PC_filename)
            #print "filesize: " + str(filesize)

            pages_num = int(math.ceil(filesize/float(self.MAX_TXBUFFER))) # How many iterations
            #print "pages_num: " + str(pages_num)
            # If buffer is large enough send whole file, otherwise split it into pages
            # Send buffer everytime before sending data
            iter_beg = 0
            iter_end = self.MAX_TXBUFFER
            current_percent = 0
            update_min = 0.01 # Update every 1% change
            last_percent = 0 # Last percentage

            f = open(PC_filename,'rb')
            all_bytes = f.read() # Read all file at once
            for x in range(0,pages_num):
                # print x
                current_percent = x / float(pages_num)*100.0
                current_percent = math.ceil(current_percent*100)/100
                delta_percent = current_percent - last_percent
                if delta_percent > update_min:
                    #print "File transfer progress: " + str(current_percent) + "%"
                    last_percent = current_percent

                # Send the file chunk by chunk
                if pages_num <= 1:
                    bytes_tx = all_bytes
                else:
                    bytes_tx = all_bytes[iter_beg:iter_end]
                    if x < pages_num-1: # If not last page
                        # Update iterators
                        iter_beg = iter_end
                        iter_end += self.MAX_TXBUFFER
                    else: # las page
                        iter_beg = iter_end
                        iter_end = filesize
                        print json.dumps("Finished")

                nxt_buffer = self.change_txbuffer(bytes_tx)
                # print "Sending buffer..."
                self.uart_txcommunicate(nxt_buffer) # Send next buffer
                # print "len_bytes_tx: " + str(len(bytes_tx))
                # print "nxt_buffer: " + nxt_buffer
                # print "Sending data..."
                self.uart_txcommunicate(bytes_tx)   # Send filename
            # Send buffer "0" in order to tell program we are done!
            nxt_buffer = self.change_txbuffer("")
            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            # Close file
            f.close()

        if k=="2": # Create address
            # print "Creating address..."
            self.uart_txcommunicate(self.PC_COMMANDS["PC_CRTADR"])
            # Coin name
            coinname = sys.argv[2]
            # Wallet id
            wallet_id = sys.argv[3]

            #print "Creating an address for " + coinname + " and wallet_id " + wallet_id
            # Make sure there are 9 (default buffer) characters
            nxt_buffer = self.change_txbuffer(coinname)

            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(coinname)   # Send coin name

            # Make sure there are 9 (default buffer) characters
            nxt_buffer = self.change_txbuffer(wallet_id)

            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(wallet_id)   # Send wallet_id

            data_list = [] # Initialize list
            while self.rx_buffer != 0:
                self.default_rxbuffer() # Set default buffer8
                self.changerx_buffer()

                if self.rx_buffer == 0:
                    print json.dumps(data_list[1])
                    break

                data_rx = self.uart_rxcommunicate()

                #print "Data received: " + data_rx
                data_list.append(data_rx)

            #print "Public key: " + data_list[0]
            #print "Public Address: " + data_list[1]
            #print "done!"

        if k=="3": # Request all addresses
            self.uart_txcommunicate(self.PC_COMMANDS["PC_RQSADR"])

            rqst_all = False # Are we requesting ALL addresses ever created?
            # Coin name
            if len(sys.argv) < 3:
                coinname = "All"
                rqst_all = True
            else:
                coinname = sys.argv[2]
                wallet_id = sys.argv[3]
            #print "Requesting all addresses for " + coinname

            # Make sure there are 9 (default buffer) characters
            nxt_buffer = self.change_txbuffer(coinname)

            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(coinname)   # Send coin name

            if rqst_all == False:
                # Make sure there are 9 (default buffer) characters
                nxt_buffer = self.change_txbuffer(wallet_id)

                self.uart_txcommunicate(nxt_buffer) # Send next buffer
                self.uart_txcommunicate(wallet_id)   # Send wallet_id

            public_address_list = [] # Initialize list
            public_key_list = [] # Initialize list

            # In case "all"
            coin_ids = [] # Coin ids (from db)
            wallet_ids = [] # Wallet ids (from db)
            idnames = []
            # Public keys!
            while self.rx_buffer != 0:
                self.default_rxbuffer() # Set default buffer8
                self.changerx_buffer()

                if self.rx_buffer == 0:
                    break

                data_rx = self.uart_rxcommunicate()
                public_key_list.append(data_rx)
                #print "Received public key: " + data_rx
            #print "All public keys received!"

            self.default_rxbuffer() # Set default buffer

            # Public Addresses!
            while self.rx_buffer != 0:
                self.default_rxbuffer() # Set default buffer8
                self.changerx_buffer()

                if self.rx_buffer == 0:
                    break

                data_rx = self.uart_rxcommunicate()
                public_address_list.append(data_rx)
                #print "Received public address: " + data_rx
            #print "All public addresses received!"

            if coinname == "All":
                self.default_rxbuffer() # Set default buffer
                # Coin ids!
                while self.rx_buffer != 0:
                    self.default_rxbuffer() # Set default buffer8
                    self.changerx_buffer()

                    if self.rx_buffer == 0:
                        break

                    data_rx = self.uart_rxcommunicate()
                    coin_ids.append(data_rx)
                    #print "Received coin id: " + data_rx
                #print "All coin IDs received!"
                json.dumps(coin_ids)

                self.default_rxbuffer() # Set default buffer
                # Wallet ids!
                while self.rx_buffer != 0:
                    self.default_rxbuffer() # Set default buffer8
                    self.changerx_buffer()

                    if self.rx_buffer == 0:
                        break

                    data_rx = self.uart_rxcommunicate()
                    wallet_ids.append(data_rx)
                    #print "Received wallet id: " + data_rx
                #print "All Wallet IDs received!"
                # print json.dumps(wallet_ids)

                #print "-----------------------------"
                #print "All wallets' Info: "
                self.default_rxbuffer() # Set default buffer
                # Wallet ids!
                while self.rx_buffer != 0:
                    self.default_rxbuffer() # Set default buffer8
                    self.changerx_buffer()

                    if self.rx_buffer == 0:
                        break

                    data_rx = self.uart_rxcommunicate()
                    idnames.append(data_rx)
                    # wallet_ids.append(data_rx)
                    #print "Received wallet id/name/type: " + data_rx
                #print "All Wallet ID, names and types received!"

                #json.dumps(wallet_ids)
                #print "All wallet ids received!"

            print json.dumps([idnames, wallet_ids, coin_ids, public_address_list])
            # print json.dumps(public_key_list)
            # print "All addresses received!"

        if k=="4": # Create new wallet
            self.uart_txcommunicate(self.PC_COMMANDS["PC_CRTWLT"])

            wallet_name = sys.argv[2]
            wallet_type = sys.argv[3]
            wallet_passphrase = sys.argv[4]

            # Make sure there are 9 (default buffer) characters
            nxt_buffer = self.change_txbuffer(wallet_name)

            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(wallet_name)   # Send wallet name

            # Make sure there are 9 (default buffer) characters
            nxt_buffer = self.change_txbuffer(wallet_type)

            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(wallet_type)   # Send wallet name

            # Make sure there are 9 (default buffer) characters
            nxt_buffer = self.change_txbuffer(wallet_passphrase)

            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(wallet_passphrase)   # Send wallet name

            while self.rx_buffer != 0:
                self.default_rxbuffer() # Set default buffer
                self.changerx_buffer()

                if self.rx_buffer == 0:
                    print json.dumps(data_rx)
                    break

                data_rx = self.uart_rxcommunicate()
                #print "Received path id: " + data_rx

            #print "done!"

        # Sign transaction
        if k=="5":
            self.uart_txcommunicate(self.PC_COMMANDS["PC_SGNTRN"])

            # Args
            wallet_id = sys.argv[2]
            coin_name = sys.argv[3]
            adress_to = sys.argv[4]
            amount = sys.argv[5]
            fee = sys.argv[6]

            # print "Sending "  + amount + " " + coin_name + " to " + adress_to + " from wallet_id " +  wallet_id

            # WALLET ID
            # Make sure there are 9 (default buffer) characters
            nxt_buffer = self.change_txbuffer(wallet_id)
            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(wallet_id)   # Send wallet_id

            # COIN NAME
            # Make sure there are 9 (default buffer) characters
            nxt_buffer = self.change_txbuffer(coin_name)
            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(coin_name)   # Send coinname

            # AMOUNT
            self.default_txbuffer(); # Set default buffer
            nxt_buffer = self.change_txbuffer(amount)
            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(amount)   # Send coinname

            # ADDRESS TO
            self.default_txbuffer(); # Set default buffer
            nxt_buffer = self.change_txbuffer(adress_to)
            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(adress_to)   # Send coinname

            tmpjsonfile = "tmp_trnout" # temp file to record json output
            str_script = "./cryptolibs/" + coin_name + "_transactionPC.js"
            cmd_str = "node " + str_script + " " + adress_to + " " + amount + " " + fee + " " + tmpjsonfile
            # print "cmd: " + cmd_str
            try:
                # Get inputs and outputs from js script
                p1 = subprocess.Popen(shlex.split(cmd_str)) # Output in json format
                p1.wait()
            except Exception as e:
                print "Error: " + e.message

            # Calculate file size
            filesize = os.path.getsize(tmpjsonfile)
            # print "filesize: " + str(filesize)
            pages_num = int(math.ceil(filesize/float(self.MAX_TXBUFFER))) # How many iterations
            # print "pages_num: " + str(pages_num)

            # If buffer is large enough send whole file, otherwise split it into pages
            # Send buffer everytime before sending data
            iter_beg = 0
            iter_end = self.MAX_TXBUFFER
            current_percent = 0
            update_min = 0.01 # Update every 1% change
            last_percent = 0 # Last percentage

            # SEND JSON INPUTS/OUTPUT OR ALL RELEVANT DATA FOR SIGNING
            f = open(tmpjsonfile,'rb')
            all_bytes = f.read() # Read all file at once
            for x in range(0,pages_num):
                # print x
                current_percent = x / float(pages_num)*100.0
                current_percent = math.ceil(current_percent*100)/100
                delta_percent = current_percent - last_percent
                if delta_percent > update_min:
                    # print "File transfer progress: " + str(current_percent) + "%"
                    last_percent = current_percent

                # Send the file chunk by chunk
                if pages_num <= 1:
                    bytes_tx = all_bytes
                else:
                    bytes_tx = all_bytes[iter_beg:iter_end]
                    if x < pages_num-1: # If not last page
                        # Update iterators
                        iter_beg = iter_end
                        iter_end += self.MAX_TXBUFFER
                    else: # las page
                        iter_beg = iter_end
                        iter_end = filesize

                nxt_buffer = self.change_txbuffer(bytes_tx)
                # print "Sending buffer..."
                self.uart_txcommunicate(nxt_buffer) # Send next buffer
                # print "len_bytes_tx: " + str(len(bytes_tx))
                # print "nxt_buffer: " + nxt_buffer
                # print "Sending data..."
                self.uart_txcommunicate(bytes_tx)   # Send
            # Send buffer "0" in order to tell program we are done!
            nxt_buffer = self.change_txbuffer("")
            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            # Close file
            f.close()
            os.remove(tmpjsonfile) # Remove json tmp file

            # RECEIVE SIGNED TRANSACTION
            txsigned_hex = ""
            while self.rx_buffer != 0:
            	# Set buffer to default again
            	self.default_rxbuffer(); # Set default buffer
            	# Get expected NEW buffer size for the next iteration
            	self.changerx_buffer()

            	if self.rx_buffer == 0: # Next buffer is zero, so we finished reading file
            		break

                # Receive data and concatenate signature
                data_rx = self.uart_rxcommunicate()
                txsigned_hex += data_rx

            # print "Done! Transaction signature received!"
            print json.dumps(txsigned_hex)

        # Read NFC card
        if k == "6":
            self.uart_txcommunicate(self.PC_COMMANDS["PC_NFCRAD"])

            #print "Reading NFC contents..."
            data_list=""
            while self.rx_buffer != 0:
                self.default_rxbuffer() # Set default buffer8
                self.changerx_buffer()

                if self.rx_buffer == 0:
                    print json.dumps(data_list)
                    break

                data_rx = self.uart_rxcommunicate()
                data_list = data_rx
                # print "Data received: " + data_rx
            # print json.dumps(data_list)
            #print "NFC contents: " + data_list

        # Write wallet to NFC card
        if k == "7":
            self.uart_txcommunicate(self.PC_COMMANDS["PC_NFCWRT"])

            # Wallet id
            wallet_id = sys.argv[2]

            #print "Writing wallet " + wallet_id + " to NFC card ..."

            # WALLET ID
            # Make sure there are 9 (default buffer) characters
            nxt_buffer = self.change_txbuffer(wallet_id)
            self.uart_txcommunicate(nxt_buffer) # Send next buffer
            self.uart_txcommunicate(wallet_id)   # Send wallet_id

            #print "Confirming NFC contents..."
            data_list=""
            while self.rx_buffer != 0:
                self.default_rxbuffer() # Set default buffer8
                self.changerx_buffer()

                if self.rx_buffer == 0:
                    print json.dumps(data_list)
                    break

                data_rx = self.uart_rxcommunicate()
                data_list = data_rx
                # print "Data received: " + data_rx
            # print json.dumps(data_list)
            #print "NFC contents: " + data_list
        
        # Update cold storage
        if k == "8":
            self.uart_txcommunicate(self.PC_COMMANDS["PC_UPDATE"])
            data_list=""
            while self.rx_buffer != 0:
                self.default_rxbuffer() # Set default buffer8
                self.changerx_buffer()

                if self.rx_buffer == 0:
                    print json.dumps("Success!")
                    break

                data_rx = self.uart_rxcommunicate()
                data_list = data_rx

        # Write wallet to NFC card

def main():
    UART1 = SerialListen()
    UART1.start() # Start serial port

    time.sleep(1) # Delay
    # Argumnts
    selected_command = sys.argv[1]
    #print "Something else..."
    # print "MENU: "
    # print "1) Send file\n"
    UART1.send_command(selected_command)
    #serial.write("F")
    #time.sleep(1)

if __name__ == "__main__":
    try:
        main() # Main program
    except Exception as e:
        print e.message
