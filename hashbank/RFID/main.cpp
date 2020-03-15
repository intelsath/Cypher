#include <iostream>
extern "C"{
	#include <stdio.h>
	#include <inttypes.h>
	#include "../libs/RC522.h"
}

using namespace std;

int main()
{
	// Object
	MFRC522 RFIDNFC;

	uint8_t byte;

	int detected = 0;
	
	uint8_t str[MAX_LEN];
	uint8_t curr_id[5];
	uint16_t card_tipe;

	uint8_t block_data[16] = { 0x48,0x65,0x6c,0x6c,0x6f,0x57,0x6f,0x72,0x6c,0x64 };
	uint8_t status = 0;

	// Read version
	byte = RFIDNFC.mfrc522_read(VersionReg);
	if (byte == 0x92)
	{
		cout << "MIFARE RC522v2, Detected\r\n";
		detected = 1;
	}
	else if (byte == 0x91 || byte == 0x90)
	{
		cout << "MIFARE RC522v1, Detected\r\n";
		detected = 1; 
	}
	else
	{
		cout << "No reader found!\r\n";
		detected = 0;
	}
	
	if (detected)
	{
		cout << "detected status: " << detected << endl;
		// Note: The NFC tag has 16 sectors of 4 blocks (16 bytes per block)
		if (RFIDNFC.rc522_read_card_id(curr_id, &card_tipe))
		{
			uint8_t block_select = 4;
			// Select tag
			printf("Card ID: ");
			for (int n = 0; n < sizeof(curr_id); n++) {
				printf("%d ", curr_id[n]);
			}
			printf(" card type: %d ", card_tipe);
			uint8_t size_tag = RFIDNFC.mfrc522_select_tag(curr_id);
			printf("Tag selected size: %d ", size_tag);
			//write block 
			uint8_t login_status = RFIDNFC.mfrc522_auth(PICC_AUTHENT1A, block_select, RFIDNFC.getkeys('A'), curr_id);
			status = RFIDNFC.mfrc522_write_block(block_select, block_data);
			printf("Login Status: %d, Writter status: %d! ", login_status, status);
			// Read block
			//uint8_t status_login = mfrc522_auth(PICC_AUTHENT1A, 4, RFIDNFC.keyA_default, curr_id);
			//printf("Status login: %d "), status_login);
			printf("Read from block %d: ", block_select);
			RFIDNFC.mfrc522_read_block(block_select, str);
			for (int i = 0; i < sizeof(str); i++) {
				printf("%d ", str[i]);
			}
			cout << " (";
			for (int i = 0; i < sizeof(str); i++) {
				printf("%c ", str[i]);
			}
			cout << ")";
			printf("\r\n");
		}
	}


	RFIDNFC.free_spi();

	return 0;
}

/*** end of "spi_test.c" file ***/