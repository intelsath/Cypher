// sudo apt-get install libtspi-dev
// g++ main.cpp -o TPM -ltspi

extern "C"
{
	#include <errno.h>
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <linux/i2c-dev.h>
	#include <sys/ioctl.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <stdbool.h>

	#include <tss/tss_error.h>
	#include <tss/platform.h>
	#include <tss/tss_defines.h>
	#include <tss/tss_typedef.h>
	#include <tss/tss_structs.h>
	#include <tss/tspi.h>
	#include <trousers/trousers.h>
}

#include <iostream>


#define DEBUG 1
/* TPM Debug */
#define DBG(message,tResult) if(DEBUG) {                        \
        printf("(Line %d, %s) %s returned 0x%08x. %s.\n",       \
               __LINE__,                                        \
               __func__,                                        \
               message,                                         \
               tResult,                                         \
               Trspi_Error_String(tResult));                    \
    }


TSS_RESULT extend_pcr (const char * buf, const int len)
{

    TSS_HCONTEXT hContext=0;
    TSS_HTPM hTPM = 0;
    TSS_RESULT result;
    TSS_HKEY hSRK = 0;
    TSS_HPOLICY hSRKPolicy=0;
    TSS_HPOLICY hOwnerPolicy=0;
    TSS_UUID SRK_UUID = TSS_UUID_SRK;
    BYTE passcode[20];

    memset(passcode,0,20);
    memcpy (passcode, buf, len);

    UINT32 ulNewPcrValueLength;
    BYTE* NewPcrValue;

    result = Tspi_Context_Create (&hContext);

    DBG(" Create a Context\n",result);
    result = Tspi_Context_Connect (hContext, NULL);
    DBG(" Connect to TPM\n", result);

    // Get the TPM handle
    result = Tspi_Context_GetTpmObject (hContext, &hTPM);
    DBG(" Get TPM Handle\n",result);

    result = Tspi_GetPolicyObject (hTPM, TSS_POLICY_USAGE, &hOwnerPolicy);
    DBG( " Owner Policy\n", result);

    result = Tspi_TPM_PcrExtend (hTPM,
                                 9,
                                 sizeof(passcode),
                                 passcode,
                                 NULL,
                                 &ulNewPcrValueLength,
                                 &NewPcrValue);

    DBG(" extend\n",result);

    return result;
}

int main ()
{
	char buf[4] = {1,9,9,3};
	bool result = false;


	if (TSS_SUCCESS == extend_pcr (buf, sizeof(buf)))
	{
		result = true;
	}
    

	return (result) ? EXIT_SUCCESS : EXIT_FAILURE;

}