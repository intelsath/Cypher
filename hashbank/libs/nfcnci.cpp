/******************************************************************************
 *
 *  Copyright (C) 2015 NXP Semiconductors
 *
 *  Licensed under the Apache License, Version 2.0 (the "License")
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#include "nfcnci.h"

#define WRITE_FAILED -1
#define CARD_NOT_FOUND -2
#define MULTIPLE_CARDS_FOUND -3

using namespace std;


unsigned char *HCE_data = NULL;
unsigned int HCE_dataLenght = 0x00;
unsigned char *pT4T_NdefRecord = NULL;
unsigned short T4T_NdefRecord_size = 0;

/********************************** HCE **********************************/
static void T4T_NDEF_EMU_FillRsp (unsigned char *pRsp, unsigned short offset, unsigned char length)
{
    if (offset == 0)
    {
        pRsp[0] = (T4T_NdefRecord_size & 0xFF00) >> 8;
        pRsp[1] = (T4T_NdefRecord_size & 0x00FF);
        memcpy(&pRsp[2], &pT4T_NdefRecord[0], length-2);
    }
    else if (offset == 1)
    {
        pRsp[0] = (T4T_NdefRecord_size & 0x00FF);
        memcpy(&pRsp[1], &pT4T_NdefRecord[0], length-1);
    }
    else
    {
        memcpy(pRsp, &pT4T_NdefRecord[offset-2], length);
    }
    /* Did we reached the end of NDEF record ?*/
    if ((offset + length) >= (T4T_NdefRecord_size + 2))
    {
        /* Notify application of the NDEF send */
        if(pT4T_NDEF_EMU_PushCb != NULL) pT4T_NDEF_EMU_PushCb(pT4T_NdefRecord, T4T_NdefRecord_size);
    }
}
void T4T_NDEF_EMU_SetRecord(unsigned char *pRecord, unsigned short Record_size, T4T_NDEF_EMU_Callback_t *cb)
{
    pT4T_NdefRecord = pRecord;
    T4T_NdefRecord_size = Record_size;
    pT4T_NDEF_EMU_PushCb =  cb;
}
void T4T_NDEF_EMU_Reset(void)
{
    eT4T_NDEF_EMU_State = Ready;
}
void T4T_NDEF_EMU_Next(unsigned char *pCmd, unsigned short Cmd_size, unsigned char *pRsp, unsigned short *pRsp_size)
{
    unsigned char eStatus = 0x00;
    if (!memcmp(pCmd, T4T_NDEF_EMU_APP_Select, sizeof(T4T_NDEF_EMU_APP_Select)))
    {
        *pRsp_size = 0;
        eStatus = 0x01;
        eT4T_NDEF_EMU_State = NDEF_Application_Selected;
    }
    else if (!memcmp(pCmd, T4T_NDEF_EMU_CC_Select, sizeof(T4T_NDEF_EMU_CC_Select)))
    {
        if(eT4T_NDEF_EMU_State == NDEF_Application_Selected)
        {
            *pRsp_size = 0;
            eStatus = 0x01;
            eT4T_NDEF_EMU_State = CC_Selected;
        }
    }
    else if (!memcmp(pCmd, T4T_NDEF_EMU_NDEF_Select, sizeof(T4T_NDEF_EMU_NDEF_Select)))
    {
        *pRsp_size = 0;
        eStatus = 0x01;
        eT4T_NDEF_EMU_State = NDEF_Selected;
    }
    else if (!memcmp(pCmd, T4T_NDEF_EMU_Read, sizeof(T4T_NDEF_EMU_Read)))
    {
        if(eT4T_NDEF_EMU_State == CC_Selected)
        {
            memcpy(pRsp, T4T_NDEF_EMU_CC, sizeof(T4T_NDEF_EMU_CC));
            *pRsp_size = sizeof(T4T_NDEF_EMU_CC);
            eStatus = 0x01;
        }
        else if (eT4T_NDEF_EMU_State == NDEF_Selected)
        {
            unsigned short offset = (pCmd[2] << 8) + pCmd[3];
            unsigned char length = pCmd[4];
            if(length <= (T4T_NdefRecord_size + offset + 2))
            {
                T4T_NDEF_EMU_FillRsp(pRsp, offset, length);
                *pRsp_size = length;
                eStatus = 0x01;
            }
        }
    }
    if (eStatus == 0x01)
    {
        memcpy(&pRsp[*pRsp_size], T4T_NDEF_EMU_OK, sizeof(T4T_NDEF_EMU_OK));
        *pRsp_size += sizeof(T4T_NDEF_EMU_OK);
    } 
    else
    {
        memcpy(pRsp, T4T_NDEF_EMU_NOK, sizeof(T4T_NDEF_EMU_NOK));
        *pRsp_size = sizeof(T4T_NDEF_EMU_NOK);
        T4T_NDEF_EMU_Reset();
    }
}
/********************************** CallBack **********************************/
void onDataReceived(unsigned char *data, unsigned int data_length)
{
    framework_LockMutex(g_HCELock);
    
    HCE_dataLenght = data_length;
    HCE_data = (unsigned char*)malloc(HCE_dataLenght * sizeof(unsigned char));
    memcpy(HCE_data, data, data_length);
    
    if(eHCEState_NONE == g_HCEState)
    {
        g_HCEState = eHCEState_DATA_RECEIVED;
    }
    else if (eHCEState_WAIT_DATA == g_HCEState)
    {
        g_HCEState = eHCEState_DATA_RECEIVED;
        framework_NotifyMutex(g_HCELock, 0);
    }
    
    framework_UnlockMutex(g_HCELock);
}
void onHostCardEmulationActivated()
{
    framework_LockMutex(g_devLock);
    
    T4T_NDEF_EMU_Reset();
    
    if(eDevState_WAIT_ARRIVAL == g_DevState)
    {
        printf("\tNFC Reader Found\n\n");
        g_DevState = eDevState_PRESENT;
        g_Dev_Type = eDevType_READER;
        framework_NotifyMutex(g_devLock, 0);
    }
    else if(eDevState_WAIT_DEPARTURE == g_DevState)
    {
        g_DevState = eDevState_PRESENT;
        g_Dev_Type = eDevType_READER;
        framework_NotifyMutex(g_devLock, 0);
    }
    else if(eDevState_EXIT == g_DevState)
    {
        g_DevState = eDevState_DEPARTED;
        g_Dev_Type = eDevType_NONE;
        framework_NotifyMutex(g_devLock, 0);
    }
    else
    {
        g_DevState = eDevState_PRESENT;
        g_Dev_Type = eDevType_READER;
    }
    framework_UnlockMutex(g_devLock);
}
void onHostCardEmulationDeactivated()
{
    framework_LockMutex(g_devLock);
    
    if(eDevState_WAIT_DEPARTURE == g_DevState)
    {
        printf("\tNFC Reader Lost\n\n");
        g_DevState = eDevState_DEPARTED;
        g_Dev_Type = eDevType_NONE;
        framework_NotifyMutex(g_devLock, 0);
    }
    else if(eDevState_PRESENT == g_DevState)
    {
        printf("\tNFC Reader Lost\n\n");
        g_DevState = eDevState_DEPARTED;
        g_Dev_Type = eDevType_NONE;
    }
    else if(eDevState_WAIT_ARRIVAL == g_DevState)
    {
    }
    else if(eDevState_EXIT == g_DevState)
    {
    }
    else
    {
        g_DevState = eDevState_DEPARTED;
        g_Dev_Type = eDevType_NONE;
    }
    framework_UnlockMutex(g_devLock);
    
    framework_LockMutex(g_HCELock);
    if(eHCEState_WAIT_DATA == g_HCEState)
    {
        g_HCEState = eHCEState_NONE;
        framework_NotifyMutex(g_HCELock, 0x00);
    }
    else if(eHCEState_EXIT == g_HCEState)
    {
        
    }
    else
    {
        g_HCEState = eHCEState_NONE;
    }
    framework_UnlockMutex(g_HCELock);
}
 
void onTagArrival(nfc_tag_info_t *pTagInfo)
{
    framework_LockMutex(g_devLock);
    
    if(eDevState_WAIT_ARRIVAL == g_DevState)
    {    
        printf("\tNFC Tag Found\n\n");
        memcpy(&g_TagInfo, pTagInfo, sizeof(nfc_tag_info_t));
        g_DevState = eDevState_PRESENT;
        g_Dev_Type = eDevType_TAG;
        framework_NotifyMutex(g_devLock, 0);
    }
    else if(eDevState_WAIT_DEPARTURE == g_DevState)
    {    
        memcpy(&g_TagInfo, pTagInfo, sizeof(nfc_tag_info_t));
        g_DevState = eDevState_PRESENT;
        g_Dev_Type = eDevType_TAG;
        framework_NotifyMutex(g_devLock, 0);
    }
    else if(eDevState_EXIT == g_DevState)
    {
        g_DevState = eDevState_DEPARTED;
        g_Dev_Type = eDevType_NONE;
        framework_NotifyMutex(g_devLock, 0);
    }
    else
    {
        g_DevState = eDevState_PRESENT;
        g_Dev_Type = eDevType_TAG;
    }
    framework_UnlockMutex(g_devLock);
}
void onTagDeparture(void)
{    
    framework_LockMutex(g_devLock);
    
    
    if(eDevState_WAIT_DEPARTURE == g_DevState)
    {
        printf("\tNFC Tag Lost\n\n");
        g_DevState = eDevState_DEPARTED;
        g_Dev_Type = eDevType_NONE;
        framework_NotifyMutex(g_devLock, 0);
    }
    else if(eDevState_WAIT_ARRIVAL == g_DevState)
    {
    }    
    else if(eDevState_EXIT == g_DevState)
    {
    }
    else
    {
        g_DevState = eDevState_DEPARTED;
        g_Dev_Type = eDevType_NONE;
    }
    framework_UnlockMutex(g_devLock);
}
void onDeviceArrival (void)
{
    framework_LockMutex(g_devLock);
    
    switch(g_DevState)
    {
        case eDevState_WAIT_DEPARTURE:
        {
            g_DevState = eDevState_PRESENT;
            g_Dev_Type = eDevType_P2P;
            framework_NotifyMutex(g_devLock, 0);
        } break;
        case eDevState_EXIT:
        {
            g_Dev_Type = eDevType_P2P;
        } break;
        case eDevState_NONE:
        {
            g_DevState = eDevState_PRESENT;
            g_Dev_Type = eDevType_P2P;
        } break;
        case eDevState_WAIT_ARRIVAL:
        {
            g_DevState = eDevState_PRESENT;
            g_Dev_Type = eDevType_P2P;
            framework_NotifyMutex(g_devLock, 0);
        } break;
        case eDevState_PRESENT:
        {
            g_Dev_Type = eDevType_P2P;
        } break;
        case eDevState_DEPARTED:
        {
            g_Dev_Type = eDevType_P2P;
            g_DevState = eDevState_PRESENT;
        } break;
    }
    
    framework_UnlockMutex(g_devLock);
}

void onDeviceDeparture (void)
{
    framework_LockMutex(g_devLock);
    
    switch(g_DevState)
    {
        case eDevState_WAIT_DEPARTURE:
        {
            g_DevState = eDevState_DEPARTED;
            g_Dev_Type = eDevType_NONE;
            framework_NotifyMutex(g_devLock, 0);
        } break;
        case eDevState_EXIT:
        {
            g_Dev_Type = eDevType_NONE;
        } break;
        case eDevState_NONE:
        {
            g_Dev_Type = eDevType_NONE;
        } break;
        case eDevState_WAIT_ARRIVAL:
        {
            g_Dev_Type = eDevType_NONE;
        } break;
        case eDevState_PRESENT:
        {
            g_Dev_Type = eDevType_NONE;
            g_DevState = eDevState_DEPARTED;
        } break;
        case eDevState_DEPARTED:
        {
            g_Dev_Type = eDevType_NONE;
        } break;
    }
    framework_UnlockMutex(g_devLock);
    
    framework_LockMutex(g_SnepClientLock);
    
    switch(g_SnepClientState)
    {
        case eSnepClientState_WAIT_OFF:
        {
            g_SnepClientState = eSnepClientState_OFF;
            framework_NotifyMutex(g_SnepClientLock, 0);
        } break;
        case eSnepClientState_OFF:
        {
        } break;
        case eSnepClientState_WAIT_READY:
        {
            g_SnepClientState = eSnepClientState_OFF;
            framework_NotifyMutex(g_SnepClientLock, 0);
        } break;
        case eSnepClientState_READY:
        {
            g_SnepClientState = eSnepClientState_OFF;
        } break;
        case eSnepClientState_EXIT:
        {
        } break;
    }
    
    framework_UnlockMutex(g_SnepClientLock);
}

void onMessageReceived(unsigned char *message, unsigned int length)
{
    unsigned int i = 0x00;
    printf("\n\t\tNDEF Message Received : \n");
    PrintNDEFContent(NULL, NULL, message, length);
}
void onSnepClientReady()
{
    framework_LockMutex(g_devLock);
    
    switch(g_DevState)
    {
        case eDevState_WAIT_DEPARTURE:
        {
            g_DevState = eDevState_PRESENT;
            g_Dev_Type = eDevType_P2P;
            framework_NotifyMutex(g_devLock, 0);
        } break;
        case eDevState_EXIT:
        {
            g_Dev_Type = eDevType_P2P;
        } break;
        case eDevState_NONE:
        {
            g_DevState = eDevState_PRESENT;
            g_Dev_Type = eDevType_P2P;
        } break;
        case eDevState_WAIT_ARRIVAL:
        {
            g_DevState = eDevState_PRESENT;
            g_Dev_Type = eDevType_P2P;
            framework_NotifyMutex(g_devLock, 0);
        } break;
        case eDevState_PRESENT:
        {
            g_Dev_Type = eDevType_P2P;
        } break;
        case eDevState_DEPARTED:
        {
            g_Dev_Type = eDevType_P2P;
            g_DevState = eDevState_PRESENT;
        } break;
    }
    framework_UnlockMutex(g_devLock);
    
    framework_LockMutex(g_SnepClientLock);
    
    switch(g_SnepClientState)
    {
        case eSnepClientState_WAIT_OFF:
        {
            g_SnepClientState = eSnepClientState_READY;
            framework_NotifyMutex(g_SnepClientLock, 0);
        } break;
        case eSnepClientState_OFF:
        {
            g_SnepClientState = eSnepClientState_READY;
        } break;
        case eSnepClientState_WAIT_READY:
        {
            g_SnepClientState = eSnepClientState_READY;
            framework_NotifyMutex(g_SnepClientLock, 0);
        } break;
        case eSnepClientState_READY:
        {
        } break;
        case eSnepClientState_EXIT:
        {
        } break;
    }
    
    framework_UnlockMutex(g_SnepClientLock);
}

void onSnepClientClosed()
{
    framework_LockMutex(g_devLock);
    
    switch(g_DevState)
    {
        case eDevState_WAIT_DEPARTURE:
        {
            g_DevState = eDevState_DEPARTED;
            g_Dev_Type = eDevType_NONE;
            framework_NotifyMutex(g_devLock, 0);
        } break;
        case eDevState_EXIT:
        {
            g_Dev_Type = eDevType_NONE;
        } break;
        case eDevState_NONE:
        {
            g_Dev_Type = eDevType_NONE;
        } break;
        case eDevState_WAIT_ARRIVAL:
        {
            g_Dev_Type = eDevType_NONE;
        } break;
        case eDevState_PRESENT:
        {
            g_Dev_Type = eDevType_NONE;
            g_DevState = eDevState_DEPARTED;
        } break;
        case eDevState_DEPARTED:
        {
            g_Dev_Type = eDevType_NONE;
        } break;
    }
    framework_UnlockMutex(g_devLock);
    
    framework_LockMutex(g_SnepClientLock);
    
    switch(g_SnepClientState)
    {
        case eSnepClientState_WAIT_OFF:
        {
            g_SnepClientState = eSnepClientState_OFF;
            framework_NotifyMutex(g_SnepClientLock, 0);
        } break;
        case eSnepClientState_OFF:
        {
        } break;
        case eSnepClientState_WAIT_READY:
        {
            g_SnepClientState = eSnepClientState_OFF;
            framework_NotifyMutex(g_SnepClientLock, 0);
        } break;
        case eSnepClientState_READY:
        {
            g_SnepClientState = eSnepClientState_OFF;
        } break;
        case eSnepClientState_EXIT:
        {
        } break;
    }
    
    framework_UnlockMutex(g_SnepClientLock);
}
 
int InitMode(int tag, int p2p, int hce)
{
    int res = 0x00;
    
    g_TagCB.onTagArrival = onTagArrival;
    g_TagCB.onTagDeparture = onTagDeparture;
        
    g_SnepServerCB.onDeviceArrival = onDeviceArrival;
    g_SnepServerCB.onDeviceDeparture = onDeviceDeparture;
    g_SnepServerCB.onMessageReceived = onMessageReceived;
    
    g_SnepClientCB.onDeviceArrival = onSnepClientReady;
    g_SnepClientCB.onDeviceDeparture = onSnepClientClosed;
        
    g_HceCB.onDataReceived = onDataReceived;
    g_HceCB.onHostCardEmulationActivated = (void (*)(unsigned char))onHostCardEmulationActivated;
    g_HceCB.onHostCardEmulationDeactivated = onHostCardEmulationDeactivated;
    
    if(0x00 == res)
    {
        res = nfcManager_doInitialize();
        if(0x00 != res)
        {
            printf("NfcService Init Failed\n");
        }
    }
    
    if(0x00 == res)
    {
        if(0x01 == tag)
        {
            nfcManager_registerTagCallback(&g_TagCB);
        }
        
        if(0x01 == p2p)
        {
            res = nfcSnep_registerClientCallback(&g_SnepClientCB);
            if(0x00 != res)
            {
                printf("SNEP Client Register Callback Failed\n");
            }
        }
    }
    
    if(0x00 == res && 0x01 == hce)
    {
        nfcHce_registerHceCallback(&g_HceCB);
    }
    if(0x00 == res)
    {
        nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 0x00, hce, 0);
        if(0x01 == p2p)
        {
            res = nfcSnep_startServer(&g_SnepServerCB);
            if(0x00 != res)
            {
                printf("Start SNEP Server Failed\n");
            }
        }
    }
    
    return res;
}

int DeinitPollMode()
{
    int res = 0x00;
    
    nfcSnep_stopServer();
    
    nfcManager_disableDiscovery();
    
    nfcSnep_deregisterClientCallback();
    
    nfcManager_deregisterTagCallback();
    
    nfcHce_deregisterHceCallback();
    
    res = nfcManager_doDeinitialize();
    
    if(0x00 != res)
    {
        printf("NFC Service Deinit Failed\n");
    }
    return res;
}

int SnepPush(unsigned char* msgToPush, unsigned int len)
{
    int res = 0x00;
    
    framework_LockMutex(g_devLock);
    framework_LockMutex(g_SnepClientLock);
    
    if(eSnepClientState_READY != g_SnepClientState && eSnepClientState_EXIT!= g_SnepClientState && eDevState_PRESENT == g_DevState)
    {
        framework_UnlockMutex(g_devLock);
        g_SnepClientState = eSnepClientState_WAIT_READY;
        framework_WaitMutex(g_SnepClientLock, 0);
    }
    else
    {
        framework_UnlockMutex(g_devLock);
    }
    
    if(eSnepClientState_READY == g_SnepClientState)
    {
        framework_UnlockMutex(g_SnepClientLock);
        res = nfcSnep_putMessage(msgToPush, len);
        
        if(0x00 != res)
        {
            printf("\t\tPush Failed\n");
        }
        else
        {
            printf("\t\tPush successful\n");
			finished_processing = true; // done
        }
    }
    else
    {
        framework_UnlockMutex(g_SnepClientLock);
    }
    
    return res;
}

int WriteTag(nfc_tag_info_t TagInfo, unsigned char* msgToPush, unsigned int len)
{
    int res = 0x00;
    
    res = nfcTag_writeNdef(TagInfo.handle, msgToPush, len);
    if(0x00 != res)
    {
        res = 0xFF;
    }
    else
    {
        res = 0x00;
    }
    return res;
}

void PrintfNDEFInfo(ndef_info_t pNDEFinfo)
{
    if(0x01 == pNDEFinfo.is_ndef)
    {
        printf("\t\tRecord Found :\n");
        printf("\t\t\t\tNDEF Content Max size :     '%d bytes'\n", pNDEFinfo.max_ndef_length);
        printf("\t\t\t\tNDEF Actual Content size :     '%d bytes'\n", pNDEFinfo.current_ndef_length);
        if(0x01 == pNDEFinfo.is_writable)
        {
            printf("\t\t\t\tReadOnly :                      'FALSE'\n");
        }
        else
        {
            printf("\t\t\t\tReadOnly :                         'TRUE'\n");
        }
    }
    else    
    {
        printf("\t\tNo Record found\n");
    }
}

void open_uri (const char* uri)
{
    char *temp = (char*)malloc(strlen("xdg-open ") + strlen(uri) + 1);
    if (temp != NULL)
    {
        strcpy(temp, "xdg-open ");
        strcat(temp, uri);
        strcat(temp, "&");
        printf("\t\t- Opening URI in web browser ...\n");
        system(temp);
        free(temp);
    }
}

void PrintNDEFContent(nfc_tag_info_t* TagInfo, ndef_info_t* NDEFinfo, unsigned char* ndefRaw, unsigned int ndefRawLen)
{
    unsigned char* NDEFContent = NULL;
    nfc_friendly_type_t lNDEFType = NDEF_FRIENDLY_TYPE_OTHER;
    unsigned int res = 0x00;
    unsigned int i = 0x00;
    char* TextContent = NULL;
    char* URLContent = NULL;
    nfc_handover_select_t HandoverSelectContent;
    nfc_handover_request_t HandoverRequestContent;
    if(NULL != NDEFinfo)
    {
        ndefRawLen = NDEFinfo->current_ndef_length;
        NDEFContent = (unsigned char*)malloc(ndefRawLen * sizeof(unsigned char));
        res = nfcTag_readNdef(TagInfo->handle, NDEFContent, ndefRawLen, &lNDEFType);
    }
    else if (NULL != ndefRaw && 0x00 != ndefRawLen)
    {
        NDEFContent = (unsigned char*)malloc(ndefRawLen * sizeof(unsigned char));
        memcpy(NDEFContent, ndefRaw, ndefRawLen);
        res = ndefRawLen;
        if((NDEFContent[0] & 0x7) == NDEF_TNF_WELLKNOWN && 0x55 == NDEFContent[3])
        {
            lNDEFType = NDEF_FRIENDLY_TYPE_URL;
        }
        if((NDEFContent[0] & 0x7) == NDEF_TNF_WELLKNOWN && 0x54 == NDEFContent[3])
        {
            lNDEFType = NDEF_FRIENDLY_TYPE_TEXT;
        }
    }
    else
    {
        printf("\t\t\t\tError : Invalid Parameters\n");
    }
    
    if(res != ndefRawLen)
    {
        printf("\t\t\t\tRead NDEF Content Failed\n");
    }
    else
    {
        switch(lNDEFType)
        {
            case NDEF_FRIENDLY_TYPE_TEXT:
            {
                TextContent = (char*)malloc(res * sizeof(char));
                res = ndef_readText(NDEFContent, res, TextContent, res);
                if(0x00 <= res)
                {
                    printf("\t\t\t\tType :                 'Text'\n");
                    printf("\t\t\t\tText :                 '%s'\n\n", TextContent);

					// Write to pipe!
					//string all_bytes = "Text"; // Convert to string
					//putnfc_contents(&named_pipe_nfc,all_bytes);
					// string all_bytes = (char*)TextContent; // Convert to string
					// putnfc_contents(all_bytes);
                }
                else
                {
                    printf("\t\t\t\tRead NDEF Text Error\n");
                }
                if(NULL != TextContent)
                {
                    free(TextContent);
                    TextContent = NULL;
                }
            } break;
            case NDEF_FRIENDLY_TYPE_URL:
            {
                /*NOTE : + 27 = Max prefix lenght*/
                URLContent = (char*)malloc(res * sizeof(unsigned char) + 27 );
                memset(URLContent, 0x00, res * sizeof(unsigned char) + 27);
                res = ndef_readUrl(NDEFContent, res, URLContent, res + 27);
                if(0x00 <= res)
                {
                    printf("                Type :                 'URI'\n");
                    printf("                URI :                 '%s'\n\n", URLContent);
					
                    /*NOTE: open url in browser*/
                    /*open_uri(URLContent);*/
                }
                else
                {
                    printf("                Read NDEF URL Error\n");
                }
                if(NULL != URLContent)
                {
                    free(URLContent);
                    URLContent = NULL;
                }
            } break;
            case NDEF_FRIENDLY_TYPE_HS:
            {
                res = ndef_readHandoverSelectInfo(NDEFContent, res, &HandoverSelectContent);
                if(0x00 <= res)
                {
                    printf("\n\t\tHandover Select : \n");
                    
                    printf("\t\tBluetooth : \n\t\t\t\tPower state : ");
                    switch(HandoverSelectContent.bluetooth.power_state)
                    {
                        case HANDOVER_CPS_INACTIVE:
                        {
                            printf(" 'Inactive'\n");
                        } break;
                        case HANDOVER_CPS_ACTIVE:
                        {
                            printf(" 'Active'\n");
                        } break;
                        case HANDOVER_CPS_ACTIVATING:
                        {
                            printf(" 'Activating'\n");
                        } break;
                        case HANDOVER_CPS_UNKNOWN:
                        {
                            printf(" 'Unknown'\n");
                        } break;
                        default:
                        {
                            printf(" 'Unknown'\n");
                        } break;
                    }
                    if(HANDOVER_TYPE_BT == HandoverSelectContent.bluetooth.type)
                    {
                        printf("\t\t\t\tType :         'BT'\n");
                    }
                    else if(HANDOVER_TYPE_BLE == HandoverSelectContent.bluetooth.type)
                    {
                        printf("\t\t\t\tType :         'BLE'\n");
                    }
                    else
                    {
                        printf("\t\t\t\tType :            'Unknown'\n");
                    }
                    printf("\t\t\t\tAddress :      '");
                    for(i = 0x00; i < 6; i++)
                    {
                        printf("%02X ", HandoverSelectContent.bluetooth.address[i]);
                    }
                    printf("'\n\t\t\t\tDevice Name :  '");
                    for(i = 0x00; i < HandoverSelectContent.bluetooth.device_name_length; i++)    
                    {
                        printf("%c ", HandoverSelectContent.bluetooth.device_name[i]);
                    }
                    printf("'\n\t\t\t\tNDEF Record :     \n\t\t\t\t");
                    for(i = 0x01; i < HandoverSelectContent.bluetooth.ndef_length+1; i++)
                    {
                        printf("%02X ", HandoverSelectContent.bluetooth.ndef[i]);
                        if(i%8 == 0)
                        {
                            printf("\n\t\t\t\t");
                        }
                    }
                    printf("\n\t\tWIFI : \n\t\t\t\tPower state : ");
                    switch(HandoverSelectContent.wifi.power_state)
                    {
                        case HANDOVER_CPS_INACTIVE:
                        {
                            printf(" 'Inactive'\n");
                        } break;
                        case HANDOVER_CPS_ACTIVE:
                        {
                            printf(" 'Active'\n");
                        } break;
                        case HANDOVER_CPS_ACTIVATING:
                        {
                            printf(" 'Activating'\n");
                        } break;
                        case HANDOVER_CPS_UNKNOWN:
                        {
                            printf(" 'Unknown'\n");
                        } break;
                        default:
                        {
                            printf(" 'Unknown'\n");
                        } break;
                    }
                    
                    printf("\t\t\t\tSSID :         '");
                    for(i = 0x01; i < HandoverSelectContent.wifi.ssid_length+1; i++)
                    {
                        printf("%02X ", HandoverSelectContent.wifi.ssid[i]);
                        if(i%30 == 0)
                        {
                            printf("\n");
                        }
                    }
                    printf("'\n\t\t\t\tKey :          '");
                    for(i = 0x01; i < HandoverSelectContent.wifi.key_length+1; i++)
                    {
                        printf("%02X ", HandoverSelectContent.wifi.key[i]);
                        if(i%30 == 0)
                        {
                            printf("\n");
                        }
                    }                
                    printf("'\n\t\t\t\tNDEF Record : \n");
                    for(i = 0x01; i < HandoverSelectContent.wifi.ndef_length+1; i++)
                    {
                        printf("%02X ", HandoverSelectContent.wifi.ndef[i]);
                        if(i%30 == 0)
                        {
                            printf("\n");
                        }
                    }
                    printf("\n");
                }
                else
                {
                    printf("\n\t\tRead NDEF Handover Select Failed\n");
                }
                
            } break;
            case NDEF_FRIENDLY_TYPE_HR:
            {
                res = ndef_readHandoverRequestInfo(NDEFContent, res, &HandoverRequestContent);
                if(0x00 <= res)
                {
                    printf("\n\t\tHandover Request : \n");
                    printf("\t\tBluetooth : \n\t\t\t\tPower state : ");
                    switch(HandoverRequestContent.bluetooth.power_state)
                    {
                        case HANDOVER_CPS_INACTIVE:
                        {
                            printf(" 'Inactive'\n");
                        } break;
                        case HANDOVER_CPS_ACTIVE:
                        {
                            printf(" 'Active'\n");
                        } break;
                        case HANDOVER_CPS_ACTIVATING:
                        {
                            printf(" 'Activating'\n");
                        } break;
                        case HANDOVER_CPS_UNKNOWN:
                        {
                            printf(" 'Unknown'\n");
                        } break;
                        default:
                        {
                            printf(" 'Unknown'\n");
                        } break;
                    }
                    if(HANDOVER_TYPE_BT == HandoverRequestContent.bluetooth.type)
                    {
                        printf("\t\t\t\tType :         'BT'\n");
                    }
                    else if(HANDOVER_TYPE_BLE == HandoverRequestContent.bluetooth.type)
                    {
                        printf("\t\t\t\tType :         'BLE'\n");
                    }
                    else
                    {
                        printf("\t\t\t\tType :            'Unknown'\n");
                    }
                    printf("\t\t\t\tAddress :      '");
                    for(i = 0x00; i < 6; i++)
                    {
                        printf("%02X ", HandoverRequestContent.bluetooth.address[i]);
                    }
                    printf("'\n\t\t\t\tDevice Name :  '");
                    for(i = 0x00; i < HandoverRequestContent.bluetooth.device_name_length; i++)    
                    {
                        printf("%c ", HandoverRequestContent.bluetooth.device_name[i]);
                    }
                    printf("'\n\t\t\t\tNDEF Record :     \n\t\t\t\t");
                    for(i = 0x01; i < HandoverRequestContent.bluetooth.ndef_length+1; i++)
                    {
                        printf("%02X ", HandoverRequestContent.bluetooth.ndef[i]);
                        if(i%8 == 0)
                        {
                            printf("\n\t\t\t\t");
                        }
                    }
                    printf("\n\t\t\t\tWIFI :         'Has WIFI Request : %X '", HandoverRequestContent.wifi.has_wifi);
                    printf("\n\t\t\t\tNDEF Record :     \n\t\t\t\t");
                    for(i = 0x01; i < HandoverRequestContent.wifi.ndef_length+1; i++)
                    {
                        printf("%02X ", HandoverRequestContent.wifi.ndef[i]);
                        if(i%8 == 0)
                        {
                            printf("\n\t\t\t\t");
                        }
                    }
                    printf("\n");
                }
                else
                {
                    printf("\n\t\tRead NDEF Handover Request Failed\n");
                }
            } break;
            case NDEF_FRIENDLY_TYPE_OTHER:
            {
                switch(NDEFContent[0] & 0x7)
                {
                    case NDEF_TNF_EMPTY:
                    {
                        printf("\n\t\tTNF Empty\n");
                    } break;
                    case NDEF_TNF_WELLKNOWN:
                    {
                        printf("\n\t\tTNF Well Known\n");
                    } break;
                    case NDEF_TNF_MEDIA:
                    {
                        printf("\n\t\tTNF Media\n\n");
                        printf("\t\t\tType : ");
                        for(i = 0x00; i < NDEFContent[1]; i++)
                        {
                            printf("%c", NDEFContent[3 + i]);
                        }
                        printf("\n\t\t\tData : ");
                        for(i = 0x00; i < NDEFContent[2]; i++)
                         {
                            printf("%c", NDEFContent[3 + NDEFContent[1] + i]);
                            if('\n' == NDEFContent[3 + NDEFContent[1] + i])
                            {
                                printf("\t\t\t");
                            }
                        }
                        printf("\n");
                        
                    } break;
                    case NDEF_TNF_URI:
                    {
                        printf("\n\t\tTNF URI\n");
                    } break;
                    case NDEF_TNF_EXT:
                    {
                        printf("\n\t\tTNF External\n\n");
                        printf("\t\t\tType : ");
                        for(i = 0x00; i < NDEFContent[1]; i++)
                        {
                            printf("%c", NDEFContent[3 + i]);
                        }
                        printf("\n\t\t\tData : ");
                        for(i = 0x00; i < NDEFContent[2]; i++)
                         {
                            printf("%c", NDEFContent[3 + NDEFContent[1] + i]);
                            if('\n' == NDEFContent[3 + NDEFContent[1] + i])
                            {
                                printf("\t\t\t");
                            }
                        }
                        printf("\n");
                    } break;
                    case NDEF_TNF_UNKNOWN:
                    {
                        printf("\n\t\tTNF Unknown\n");
                    } break;
                    case NDEF_TNF_UNCHANGED:
                    {
                        printf("\n\t\tTNF Unchanged\n");
                    } break;
                    default:
                    {
                        printf("\n\t\tTNF Other\n");
                    } break;
                }
            } break;
            default:
            {
            } break;
        }
        printf("\n\t\t%d bytes of NDEF data received :\n\t\t", ndefRawLen);
        for(i = 0x00; i < ndefRawLen; i++)
        {
            printf("%02X ", NDEFContent[i]);
			
            if(i%30 == 0 && 0x00 != i)
            {
                printf("\n\t\t");
            }
        }
        printf("\n\n");

		// Send to pipe
		string bytes = (char*)NDEFContent; // Convert to string
		string msg_rcvd;
		msg_rcvd = bytes.substr (7,ndefRawLen-7); // Discard the first 7 bytes and the last 7
		putnfc_contents(msg_rcvd);
		finished_processing = true; // done
    }

    if(NULL != NDEFContent)
    {
        free(NDEFContent);
        NDEFContent = NULL;
    }
}

/* mode=1 => poll, mode=2 => push, mode=3 => write, mode=4 => HCE */
int WaitDeviceArrival(int mode, unsigned char* msgToSend, unsigned int len)
{
    int res = 0x00;
    unsigned int i = 0x00;
    int block = 0x01;
    unsigned char key[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ndef_info_t NDEFinfo;
    eDevType DevTypeBck = eDevType_NONE;
    unsigned char MifareAuthCmd[] = {0x60U, 0x00 /*block*/, 0x02, 0x02, 0x02, 0x02, 0x00 /*key*/, 0x00 /*key*/, 0x00 /*key*/, 0x00 /*key*/ , 0x00 /*key*/, 0x00 /*key*/};
    unsigned char MifareAuthResp[255];
    unsigned char MifareReadCmd[] = {0x30U,  /*block*/ 0x00};
    unsigned char MifareWriteCmd[] = {0xA2U,  /*block*/ 0x04, 0xFF, 0xFF, 0xFF, 0xFF};
    unsigned char MifareResp[255];
    
    unsigned char HCEReponse[255];
    short unsigned int HCEResponseLen = 0x00;
    int tag_count=0;
    int num_tags = 0;
    
    nfc_tag_info_t TagInfo;
    
    MifareAuthCmd[1] = block;
    memcpy(&MifareAuthCmd[6], key, 6);
    MifareReadCmd[1] = block;
    
    do
    {
        framework_LockMutex(g_devLock);
        if(eDevState_EXIT == g_DevState)
        {
            framework_UnlockMutex(g_devLock);
            break;
        }
        
        else if(eDevState_PRESENT != g_DevState)
        {
               if(tag_count == 0) printf("Waiting for a Tag/Device...\n\n");
            g_DevState = eDevState_WAIT_ARRIVAL;
            framework_WaitMutex(g_devLock, 0);
        }
        
        if(eDevState_EXIT == g_DevState)
        {
            framework_UnlockMutex(g_devLock);
            break;
        }
        
        if(eDevState_PRESENT == g_DevState)
        {
            DevTypeBck = g_Dev_Type;
            if(eDevType_TAG == g_Dev_Type)
            {
                memcpy(&TagInfo, &g_TagInfo, sizeof(nfc_tag_info_t));
                framework_UnlockMutex(g_devLock);
                printf("        Type : ");
                switch (TagInfo.technology)
                {
                    case TARGET_TYPE_UNKNOWN:
                    {
                        printf("        'Type Unknown'\n");
                    } break;
                    case TARGET_TYPE_ISO14443_3A:
                    {
                        printf("        'Type A'\n");
                    } break;
                    case TARGET_TYPE_ISO14443_3B:
                    {
                        printf("        'Type 4B'\n");
                    } break;
                    case TARGET_TYPE_ISO14443_4:
                    {
                        printf("        'Type 4A'\n");
                    } break;
                    case TARGET_TYPE_FELICA:
                    {
                        printf("        'Type F'\n");
                    } break;
                    case TARGET_TYPE_ISO15693:
                    {
                        printf("        'Type V'\n");
                    } break;
                    case TARGET_TYPE_NDEF:
                    {
                        printf("        'Type NDEF'\n");
                    } break;
                    case TARGET_TYPE_NDEF_FORMATABLE:
                    {
                        printf("        'Type Formatable'\n");
                    } break;
                    case TARGET_TYPE_MIFARE_CLASSIC:
                    {
                        printf("        'Type A - Mifare Classic'\n");
                    } break;
                    case TARGET_TYPE_MIFARE_UL:
                    {
                        printf("        'Type A - Mifare Ul'\n");
                    } break;
                    case TARGET_TYPE_KOVIO_BARCODE:
                    {
                        printf("        'Type A - Kovio Barcode'\n");
                    } break;
                    case TARGET_TYPE_ISO14443_3A_3B:
                    {
                        printf("        'Type A/B'\n");
                    } break;
                    default:
                    {
                        printf("        'Type %d (Unknown or not supported)'\n", TagInfo.technology);
                    } break;
                }
                /*32 is max UID len (Kovio tags)*/
                if((0x00 != TagInfo.uid_length) && (32 >= TagInfo.uid_length))
                {
                    if(4 == TagInfo.uid_length || 7 == TagInfo.uid_length || 10 == TagInfo.uid_length)
                    {
                        printf("        NFCID1 :    \t'");
                    }
                    else if(8 == TagInfo.uid_length)
                    {
                        printf("        NFCID2 :    \t'");
                    }
                    else
                    {
                        printf("        UID :       \t'");
                    }
                    
                    for(i = 0x00; i < TagInfo.uid_length; i++)
                    {
                        printf("%02X ", (unsigned char) TagInfo.uid[i]);
                    }
                    printf("'\n");
                }
                res = nfcTag_isNdef(TagInfo.handle, &NDEFinfo);
                if(0x01 == res)
                {
                    PrintfNDEFInfo(NDEFinfo);
                    PrintNDEFContent(&TagInfo, &NDEFinfo, NULL, 0x00);
                }
                else
                {
                    printf("\t\tNDEF Content : NO, mode=%d, tech=%d\n", mode, TagInfo.technology);
                    
                    if(0x03 == mode)
                    {
                         printf("\n\tFormating tag to NDEF prior to write ...\n");
                        if(nfcTag_isFormatable(TagInfo.handle))
                        {
                            if(nfcTag_formatTag(TagInfo.handle) == 0x00)
                            {
                                  printf("\tTag formating succeed\n");
                            }
                            else
                            {
                                  printf("\tTag formating failed\n");
                            }
                        }
                        else
                        {
                              printf("\tTag is not formatable\n");
                        }
                    }
                    else if(TARGET_TYPE_MIFARE_CLASSIC == TagInfo.technology)
                    {
                        memset(MifareAuthResp, 0x00, 255);
                        memset(MifareResp, 0x00, 255);
                        res = nfcTag_transceive(TagInfo.handle, MifareAuthCmd, 12, MifareAuthResp, 255, 500);
                        if(0x00 == res)
                        {
                            printf("\n\t\tRAW Tag transceive failed\n");
                        }
                        else
                        {
                            printf("\n\t\tMifare Authenticate command sent\n\t\tResponse : \n\t\t");
                            for(i = 0x00; i < (unsigned int) res; i++)
                            {
                                printf("%02X ", MifareAuthResp[i]);
                            }
                            printf("\n");
                            
                            res = nfcTag_transceive(TagInfo.handle, MifareReadCmd, 2, MifareResp, 255, 500);
                            if(0x00 == res)
                            {
                                printf("\n\t\tRAW Tag transceive failed\n");
                            }
                            else
                            {
                                printf("\n\t\tMifare Read command sent\n\t\tResponse : \n\t\t");
                                for(i = 0x00; i < (unsigned int)res; i++)
                                {
                                    printf("%02X ", MifareResp[i]);
                                }
                                printf("\n\n");
                            }
                        }
                    }
                    else if(TARGET_TYPE_MIFARE_UL == TagInfo.technology)
                    {
                        printf("\n\tMIFARE UL card\n");
                        printf("\t\tMifare Read command: ");
                        for(i = 0x00; i < (unsigned int) sizeof(MifareReadCmd) ; i++)
                        {
                            printf("%02X ", MifareReadCmd[i]);
                        }
                        printf("\n");
                        res = nfcTag_transceive(TagInfo.handle, MifareReadCmd, sizeof(MifareReadCmd), MifareResp, 16, 500);
                        if(0x00 == res)
                        {
                            printf("\n\t\tRAW Tag transceive failed\n");
                        }
                        else
                        {
                            printf("\n\t\tMifare Read command sent\n\t\tResponse : \n\t\t");
                            for(i = 0x00; i < (unsigned int)res; i++)
                            {
                                printf("%02X ", MifareResp[i]);
                            }
                            printf("\n\n");
                        }
                    }
                    else
                    {
                        printf("\n\tNot a MIFARE card\n");
                    }
                }
                if(0x03 == mode)
                {
                    res = WriteTag(TagInfo, msgToSend, len);
                    if(0x00 == res)
                    {
                        printf("\tWrite Tag OK\n\tRead back data\n");
                        res = nfcTag_isNdef(TagInfo.handle, &NDEFinfo);
                        if(0x01 == res)
                        {
                            PrintfNDEFInfo(NDEFinfo);
                            PrintNDEFContent(&TagInfo, &NDEFinfo, NULL, 0x00);
                        }
                    }
                    else
                    {
                        printf("\tWrite Tag Failed\n");
						//string str_snd = "Write tag failed. ";
						// putnfc_contents(str_snd); // Timeout reached!
						EXCEPTION = WRITE_FAILED;
                    }
                }
                num_tags = nfcManager_getNumTags();
                if(num_tags > 1)
                {
                    tag_count++;
                    if (tag_count < num_tags)
                    {
                        printf("\tMultiple tags found...\n");
						//EXCEPTION = MULTIPLE_CARDS_FOUND;
                        //nfcManager_selectNextTag();
                    }
                    else
                    {
                        tag_count = 0;
                    }
                }
                 framework_LockMutex(g_devLock);
            }
            else if(eDevType_P2P == g_Dev_Type)/*P2P Detected*/
            {
                framework_UnlockMutex(g_devLock);
                 printf("\tDevice Found\n");
                
                if(2 == mode)
                {
                    SnepPush(msgToSend, len);
                }
                
                framework_LockMutex(g_SnepClientLock);
    
                if(eSnepClientState_READY == g_SnepClientState)
                {
                    g_SnepClientState = eSnepClientState_WAIT_OFF;
                    framework_WaitMutex(g_SnepClientLock, 0);
                }
                
                framework_UnlockMutex(g_SnepClientLock);
                framework_LockMutex(g_devLock);
        
            }
            else if(eDevType_READER == g_Dev_Type)
            {                
                framework_LockMutex(g_HCELock);
                do
                {
                    framework_UnlockMutex(g_devLock);
                
                    if(eHCEState_NONE == g_HCEState)
                    {
                        g_HCEState = eHCEState_WAIT_DATA;
                        framework_WaitMutex(g_HCELock, 0x00);
                    }
                    
                    if(eHCEState_DATA_RECEIVED == g_HCEState)
                    {
                        g_HCEState = eHCEState_NONE;
                        
                        if(HCE_data != NULL)
                        {
                            printf("\t\tReceived data from remote device : \n\t\t");
                            
                            for(i = 0x00; i < HCE_dataLenght; i++)
                            {
                                printf("%02X ", HCE_data[i]);
                            }
                            
                            /*Call HCE response builder*/
                            T4T_NDEF_EMU_Next(HCE_data, HCE_dataLenght, HCEReponse, &HCEResponseLen);
                            free(HCE_data);
                            HCE_dataLenght = 0x00;
                            HCE_data = NULL;
                        }
                        framework_UnlockMutex(g_HCELock);
                        res = nfcHce_sendCommand(HCEReponse, HCEResponseLen);
                        framework_LockMutex(g_HCELock);
                        if(0x00 == res)
                        {
                            printf("\n\n\t\tResponse sent : \n\t\t");
                            for(i = 0x00; i < HCEResponseLen; i++)
                            {
                                printf("%02X ", HCEReponse[i]);
                            }
                            printf("\n\n");
                        }
                        else
                        {
                            printf("\n\n\t\tFailed to send response\n\n");
                        }
                    }
                    framework_LockMutex(g_devLock);
                }while(eDevState_PRESENT == g_DevState);
                framework_UnlockMutex(g_HCELock);
            }
            else
            {
                framework_UnlockMutex(g_devLock);
                break;
            }
            
            if(eDevState_PRESENT == g_DevState)
            {
                g_DevState = eDevState_WAIT_DEPARTURE;
                framework_WaitMutex(g_devLock, 0);
                if(eDevType_P2P == DevTypeBck)
                {
                    printf("\tDevice Lost\n\n");
                }
                DevTypeBck = eDevType_NONE;
            }
            else if(eDevType_P2P == DevTypeBck)
            {
                printf("\tDevice Lost\n\n");
            }
        }
        
        framework_UnlockMutex(g_devLock);
    }while(0x01);
    
    return res;
}

void strtolower(char * string) 
{
    unsigned int i = 0x00;
    
    for(i = 0; i < strlen(string); i++) 
    {
        string[i] = tolower(string[i]);
    }
}

char* strRemovceChar(const char* str, char car)
{
    unsigned int i = 0x00;
    unsigned int index = 0x00;
    char * dest = (char*)malloc((strlen(str) + 1) * sizeof(char));
    
    for(i = 0x00; i < strlen(str); i++)
    {
        if(str[i] != car)
        {
            dest[index++] = str[i];
        }
    }
    dest[index] = '\0';
    return dest;
}

int convertParamtoBuffer(char* param, unsigned char** outBuffer, unsigned int* outBufferLen)
{
    int res = 0x00;
    unsigned int i = 0x00;
    int index = 0x00;
    char atoiBuf[3];
    atoiBuf[2] = '\0';
    
    if(NULL == param || NULL == outBuffer || NULL == outBufferLen)    
    {
        printf("Parameter Error\n");
        res = 0xFF;
    }
    
    if(0x00 == res)
    {
        param = strRemovceChar(param, ' ');
    }
    
    if(0x00 == res)
    {
        if(0x00 == strlen(param) % 2)
        {
            *outBufferLen = strlen(param) / 2;
            
            *outBuffer = (unsigned char*) malloc((*outBufferLen) * sizeof(unsigned char));
            if(NULL != *outBuffer)
            {
                for(i = 0x00; i < ((*outBufferLen) * 2); i = i + 2)
                {
                    atoiBuf[0] = param[i];
                    atoiBuf[1] = param[i + 1];
                    (*outBuffer)[index++] = strtol(atoiBuf, NULL, 16);
                }
            }
            else
            {
                printf("Memory Allocation Failed\n");
                res = 0xFF;
            }
        }
        else
        {
            printf("Invalid NDEF Message Param\n");
        }
        free(param);
    }
        
    return res;
}

int BuildNDEFMessage(int arg_len, char** arg, unsigned char** outNDEFBuffer, unsigned int* outNDEFBufferLen)
{
    int res = 0x00;
    nfc_handover_cps_t cps = HANDOVER_CPS_UNKNOWN;
    unsigned char* ndef_msg = NULL;
    unsigned int ndef_msg_len = 0x00;
    char *type = NULL;
    char *uri = NULL;
    char *text = NULL;
    char* lang = NULL;
    char* mime_type = NULL;
    char* mime_data = NULL;
    char* carrier_state = NULL;
    char* carrier_name = NULL;
    char* carrier_data = NULL;
    
    if(0xFF == LookForTag(arg, arg_len, "-t", &type, 0x00) && 0xFF == LookForTag(arg, arg_len, "--type", &type, 0x01))
    {
        res = 0xFF;
        printf("Record type missing (-t)\n");
        ////help(0x02);
    }
    if(0x00 == res)
    {
        strtolower(type);
        if(0x00 == strcmp(type, "uri"))
        {
            if(0x00 == LookForTag(arg, arg_len, "-u", &uri, 0x00) || 0x00 == LookForTag(arg, arg_len, "--uri", &uri, 0x01))
            {
                *outNDEFBufferLen = strlen(uri) + 30; /*TODO : replace 30 by URI NDEF message header*/
                *outNDEFBuffer = (unsigned char*) malloc(*outNDEFBufferLen * sizeof(unsigned char));
                res = ndef_createUri(uri, *outNDEFBuffer, *outNDEFBufferLen);
                if(0x00 >= res)
                {
                    printf("Failed to build URI NDEF Message\n");
                }
                else
                {
                    *outNDEFBufferLen = res;
                    res = 0x00;
                }
            }
            else
            {
                printf("URI Missing (-u)\n");
                ////help(0x02);
                res = 0xFF;
            }
        }
        else if(0x00 == strcmp(type, "text"))
        {
            if(0xFF == LookForTag(arg, arg_len, "-r", &text, 0x00) && 0xFF == LookForTag(arg, arg_len, "--rep", &text, 0x01))
            {
                printf("Representation missing (-r)\n");
                res = 0xFF;
            }
            
            if(0xFF == LookForTag(arg, arg_len, "-l", &lang, 0x00) && 0xFF == LookForTag(arg, arg_len, "--lang", &lang, 0x01))
            {
                printf("Language missing (-l)\n");
                res = 0xFF;
            }
            if(0x00 == res)
            {
                *outNDEFBufferLen = strlen(text) + strlen(lang) + 30; /*TODO : replace 30 by TEXT NDEF message header*/
                *outNDEFBuffer = (unsigned char*) malloc(*outNDEFBufferLen * sizeof(unsigned char));
                res = ndef_createText(lang, text, *outNDEFBuffer, *outNDEFBufferLen);
                if(0x00 >= res)
                {
                    printf("Failed to build TEXT NDEF Message\n");
                }
                else
                {
                    *outNDEFBufferLen = res;
                    res = 0x00;
                }    
            }
            else
            {
                ////help(0x02);
            }
        }
        else if(0x00 == strcmp(type, "mime"))
        {
            if(0xFF == LookForTag(arg, arg_len, "-m", &mime_type, 0x00) && 0xFF == LookForTag(arg, arg_len, "--mime", &mime_type, 0x01))
            {
                printf("Mime-type missing (-m)\n");
                res = 0xFF;
            }
            if(0xFF == LookForTag(arg, arg_len, "-d", &mime_data, 0x00) && 0xFF == LookForTag(arg, arg_len, "--data", &mime_data, 0x01))
            {
                printf("NDEF Data missing (-d)\n");
                res = 0xFF;
            }
            if(0x00 == res)
            {
                *outNDEFBufferLen = strlen(mime_data) +  strlen(mime_type) + 30; /*TODO : replace 30 by MIME NDEF message header*/
                *outNDEFBuffer = (unsigned char*) malloc(*outNDEFBufferLen * sizeof(unsigned char));
                
                res = convertParamtoBuffer(mime_data, &ndef_msg, &ndef_msg_len);
                
                if(0x00 == res)
                {
                    res = ndef_createMime(mime_type, ndef_msg, ndef_msg_len, *outNDEFBuffer, *outNDEFBufferLen);
                    if(0x00 >= res)
                    {
                        printf("Failed to build MIME NDEF Message\n");
                    }
                    else
                    {
                        *outNDEFBufferLen = res;
                        res = 0x00;
                    }
                }
            }
            else
            {
                ////help(0x02);
            }
        }
        else if(0x00 == strcmp(type, "hs"))
        {
            
            if(0xFF == LookForTag(arg, arg_len, "-cs", &carrier_state, 0x00) && 0xFF == LookForTag(arg, arg_len, "--carrierState", &carrier_state, 0x01))
            {
                printf("Carrier Power State missing (-cs)\n");
                res = 0xFF;
            }
            if(0xFF == LookForTag(arg, arg_len, "-cn", &carrier_name, 0x00) && 0xFF == LookForTag(arg, arg_len, "--carrierName", &carrier_name, 0x01))
            {
                printf("Carrier Reference Name missing (-cn)\n");
                res = 0xFF;
            }
            
            if(0xFF == LookForTag(arg, arg_len, "-d", &carrier_data, 0x00) && 0xFF == LookForTag(arg, arg_len, "--data", &carrier_data, 0x01))
            {
                printf("NDEF Data missing (-d)\n");
                res = 0xFF;
            }
            
            if(0x00 == res)
            {
                *outNDEFBufferLen = strlen(carrier_name) + strlen(carrier_data) + 30;  /*TODO : replace 30 by HS NDEF message header*/
                *outNDEFBuffer = (unsigned char*) malloc(*outNDEFBufferLen * sizeof(unsigned char));
                
                strtolower(carrier_state);
                
                if(0x00 == strcmp(carrier_state, "inactive"))
                {
                    cps = HANDOVER_CPS_INACTIVE;
                }
                else if(0x00 == strcmp(carrier_state, "active"))
                {
                    cps = HANDOVER_CPS_ACTIVE;
                }
                else if(0x00 == strcmp(carrier_state, "activating"))
                {
                    cps = HANDOVER_CPS_ACTIVATING;
                }
                else
                {
                    printf("Error : unknown carrier power state %s\n", carrier_state);
                    res = 0xFF;
                }
                
                if(0x00 == res)
                {
                    res = convertParamtoBuffer(carrier_data, &ndef_msg, &ndef_msg_len);
                }
                
                if(0x00 == res)
                {
                    res = ndef_createHandoverSelect(cps, carrier_name, ndef_msg, ndef_msg_len, *outNDEFBuffer, *outNDEFBufferLen);
                    if(0x00 >= res)
                    {
                        printf("Failed to build handover select message\n");
                    }
                    else
                    {
                        *outNDEFBufferLen = res;
                        res = 0x00;
                    }
                }
            }
            else
            {
                ////help(0x02);
            }
        }
        else
        {
            printf("NDEF Type %s not supported\n", type);
            res = 0xFF;
        }
    }
    
    if(NULL != ndef_msg)
    {
        free(ndef_msg);
        ndef_msg = NULL;
        ndef_msg_len = 0x00;
    }
    
    return res;
}

/* if data = NULL this tag is not followed by dataStr : for example -h --help
if format = 0 tag format -t "text" if format=1 tag format : --type=text */
int LookForTag(char** args, int args_len, char* tag, char** data, int format)
{
    int res = 0xFF;
    int i = 0x00;
    int found = 0xFF;
    
    for(i = 0x00; i < args_len; i++)
    {
        found = 0xFF;
        strtolower(args[i]);
        if(0x00 == format)
        {
            found = strcmp(args[i], tag);
        }
        else
        {
            found = strncmp(args[i], tag, strlen(tag));
        }
        
        if(0x00 == found)
        {
            if(NULL != data)
            {
                if(0x00 == format)
                {
                    if(i < (args_len - 1))
                    {
                        *data = args[i + 1];
                        res = 0x00;
                        break;
                    }
                    else
                    {
                        printf("Argument missing after %s\n", tag);
                    }
                }
                else
                {
                    *data = &args[i][strlen(tag) + 1]; /* +1 to remove '='*/
                    res = 0x00;
                    break;
                }
            }
            else
            {
                res = 0x00;
                break;
            }
        }
    }
    
    return res;
}
 
void cmd_poll(int arg_len, char** arg)
{
	timer1.reset(); // Reset to zero
	timer1.start(); // Start

    int res = 0x00;
    
    printf("--------------------------------------------------------\n");
    printf("--                                       NFC demo                                      --\n");
    printf("--------------------------------------------------------\n");
    printf("--                                 Poll mode activated                                 --\n");
    printf("--------------------------------------------------------\n");
    
    InitEnv();
    if(0x00 == LookForTag(arg, arg_len, "-h", NULL, 0x00) || 0x00 == LookForTag(arg, arg_len, "--help", NULL, 0x01))
    {
        ////help(0x01);
    }
    else
    {
        res = InitMode(0x01, 0x01, 0x00);
        
        if(0x00 == res)
        {
            WaitDeviceArrival(0x01, NULL , 0x00);
        }
    
        res = DeinitPollMode();
    }
    
    printf("Leaving ...\n");
}
 
void cmd_push(int arg_len, char** arg)
{
    int res = 0x00;
    unsigned char * NDEFMsg = NULL;
    unsigned int NDEFMsgLen = 0x00;
    
    printf("--------------------------------------------------------\n");
    printf("--                                       NFC demo                                      --\n");
    printf("--------------------------------------------------------\n");
    printf("--                                 Push mode activated                                 --\n");
    printf("--------------------------------------------------------\n");
    
    InitEnv();
    
    if(0x00 == LookForTag(arg, arg_len, "-h", NULL, 0x00) || 0x00 == LookForTag(arg, arg_len, "--help", NULL, 0x01))
    {
        ////help(0x02);
    }
    else
    {
        res = InitMode(0x01, 0x01, 0x00);
        
        if(0x00 == res)
        {
            res = BuildNDEFMessage(arg_len, arg, &NDEFMsg, &NDEFMsgLen);
        }
        
        if(0x00 == res)
        {
            WaitDeviceArrival(0x02, NDEFMsg, NDEFMsgLen);
        }
        
        if(NULL != NDEFMsg)
        {
            free(NDEFMsg);
            NDEFMsg = NULL;
            NDEFMsgLen = 0x00;
        }
        
        res = DeinitPollMode();
    }
    
    printf("Leaving ...\n");
}

void cmd_share(int arg_len, char** arg)
{
    int res = 0x00;
    unsigned char * NDEFMsg = NULL;
    unsigned int NDEFMsgLen = 0x00;
    
    printf("--------------------------------------------------------\n");
    printf("--                                       NFC demo                                      --\n");
    printf("--------------------------------------------------------\n");
    printf("--                                 Share mode activated                                --\n");
    printf("--------------------------------------------------------\n");
    
    InitEnv();
    
    if(0x00 == LookForTag(arg, arg_len, "-h", NULL, 0x00) || 0x00 == LookForTag(arg, arg_len, "--help", NULL, 0x01))
    {
        ////help(0x02);
    }
    else
    {
        res = InitMode(0x00, 0x00, 0x01);
        
        if(0x00 == res)
        {
            res = BuildNDEFMessage(arg_len, arg, &NDEFMsg, &NDEFMsgLen);
        }
        
        if(0x00 == res)
        {
            T4T_NDEF_EMU_SetRecord(NDEFMsg, NDEFMsgLen, NULL);
        }
        
        if(0x00 == res)
        {
            WaitDeviceArrival(0x04, NDEFMsg, NDEFMsgLen);
        }
        
        if(NULL != NDEFMsg)
        {
            free(NDEFMsg);
            NDEFMsg = NULL;
            NDEFMsgLen = 0x00;
        }
        
        res = DeinitPollMode();
    }
    
    printf("Leaving ...\n");
}
void cmd_write(int arg_len, char** arg)
{
	timer1.reset(); // Reset to zero
	timer1.start(); // Start

    int res = 0x00;
    unsigned char * NDEFMsg = NULL;
    unsigned int NDEFMsgLen = 0x00;
    
    printf("--------------------------------------------------------\n");
    printf("--                                       NFC demo                                      --\n");
    printf("--------------------------------------------------------\n");
    printf("--                                 Write mode activated                                --\n");
    printf("--------------------------------------------------------\n");
    
    InitEnv();
    
    if(0x00 == LookForTag(arg, arg_len, "-h", NULL, 0x00) || 0x00 == LookForTag(arg, arg_len, "--help", NULL, 0x01))
    {
        //help(0x02);
    }
    else
    {
        res = InitMode(0x01, 0x01, 0x00);
        
        if(0x00 == res)
        {
            res = BuildNDEFMessage(arg_len, arg, &NDEFMsg, &NDEFMsgLen);
        }
        
        if(0x00 == res)
        {
            WaitDeviceArrival(0x03, NDEFMsg, NDEFMsgLen);
        }
        
        if(NULL != NDEFMsg)
        {
            free(NDEFMsg);
            NDEFMsg = NULL;
            NDEFMsgLen = 0x00;
        }
        
        res = DeinitPollMode();
    }
    
    printf("Leaving ...\n");
}

void* ExitThread(void* pContext)
{
    // printf("                              ... press enter to quit ...\n\n");
    
    // getchar();
	while(!finished_processing)
	{
		double timer1_epoch = timer1.get_epoch();
		if( timer1_epoch >= timeout_poll || EXCEPTION != 0 ) // Timeout or exception happened
		{
			string str_snd = "ERROR";
			
			if( EXCEPTION )
			{
				// EXCEPTON HAPPENED
			}
			else
			{
				// Timeout reached... Bye
				cout << "TIMED OUT..." << endl;
			}
				
			putnfc_contents(str_snd); // Timeout reached!

			break;
		}
		usleep(100);
	}
    
    framework_LockMutex(g_SnepClientLock);
    
    if(eSnepClientState_WAIT_OFF == g_SnepClientState || eSnepClientState_WAIT_READY == g_SnepClientState)
    {
        g_SnepClientState = eSnepClientState_EXIT;
        framework_NotifyMutex(g_SnepClientLock, 0);
    }
    else
    {
        g_SnepClientState = eSnepClientState_EXIT;
    }
    framework_UnlockMutex(g_SnepClientLock);
    
    framework_LockMutex(g_devLock);
    
    if(eDevState_WAIT_ARRIVAL == g_DevState || eDevState_WAIT_DEPARTURE == g_DevState)
    {
        g_DevState = eDevState_EXIT;
        framework_NotifyMutex(g_devLock, 0);
    }
    else
    {
        g_DevState = eDevState_EXIT;
    }
    
    framework_UnlockMutex(g_devLock);
    return NULL;
}

int InitEnv()
{
    eResult tool_res = FRAMEWORK_SUCCESS;
    int res = 0x00;
    
    tool_res = framework_CreateMutex(&g_devLock);
    if(FRAMEWORK_SUCCESS != tool_res)
    {
        res = 0xFF;
    }
    
    if(0x00 == res)
    {
        tool_res = framework_CreateMutex(&g_SnepClientLock);
        if(FRAMEWORK_SUCCESS != tool_res)
        {
            res = 0xFF;
        }
     }
     
     if(0x00 == res)
    {
        tool_res = framework_CreateMutex(&g_HCELock);
        if(FRAMEWORK_SUCCESS != tool_res)
        {
            res = 0xFF;
        }
     }
     if(0x00 == res)
    {
        tool_res = framework_CreateThread(&g_ThreadHandle, ExitThread, NULL);
        if(FRAMEWORK_SUCCESS != tool_res)
        {
            res = 0xFF;
        }
    }
    return res;
}

int CleanEnv()
{
    if(NULL != g_ThreadHandle)
    {
        framework_JoinThread(g_ThreadHandle);
        framework_DeleteThread(g_ThreadHandle);
        g_ThreadHandle = NULL;
    }
        
    if(NULL != g_devLock)
    {
        framework_DeleteMutex(g_devLock);
        g_devLock = NULL;
    }
    
    if(NULL != g_SnepClientLock)
    {
        framework_DeleteMutex(g_SnepClientLock);
        g_SnepClientLock = NULL;
    }
    if(NULL != g_HCELock)
    {
        framework_DeleteMutex(g_HCELock);
        g_HCELock = NULL;
    }
    return 0x00;
}


void* framework_AllocMem(size_t size);
void  framework_FreeMem(void *ptr);


/****************** Thread ****************************/


typedef struct tLinuxThread
{
	pthread_t thread;
	void* ctx;
	void* (*threadedFunc)(void *);
	void* mutexCanDelete;
}tLinuxThread_t;

void* thread_object_func(void* obj)
{
	tLinuxThread_t *linuxThread = (tLinuxThread_t *)obj;
	void *res = NULL;
	framework_LockMutex(linuxThread->mutexCanDelete);
	res = linuxThread->threadedFunc(linuxThread->ctx);
	framework_UnlockMutex(linuxThread->mutexCanDelete);

	return res;
}


eResult framework_CreateThread(void** threadHandle, void * (* threadedFunc)(void *) , void * ctx)
{
	tLinuxThread_t *linuxThread = (tLinuxThread_t *)framework_AllocMem(sizeof(tLinuxThread_t));
	
	linuxThread->ctx = ctx;
	linuxThread->threadedFunc = threadedFunc;
	framework_CreateMutex(&(linuxThread->mutexCanDelete));
	
	if (pthread_create(&(linuxThread->thread), NULL, thread_object_func, linuxThread))
	{
		printf("Cannot create Thread\n");
		framework_DeleteMutex(linuxThread->mutexCanDelete);
		framework_FreeMem(linuxThread);
		
		return FRAMEWORK_FAILED;
	}
	pthread_detach(linuxThread->thread);
	
	*threadHandle = linuxThread;
	
	return FRAMEWORK_SUCCESS;
}

void framework_JoinThread(void * threadHandle)
{
	tLinuxThread_t *linuxThread = (tLinuxThread_t*)threadHandle;
	if (pthread_self() != linuxThread->thread)
	{
		// Will cause block if thread still running !!!
		framework_LockMutex(linuxThread->mutexCanDelete);
		framework_UnlockMutex(linuxThread->mutexCanDelete);
		// Thread now just ends up !
	}
}


void framework_DeleteThread(void * threadHandle)
{
	tLinuxThread_t *linuxThread = (tLinuxThread_t*)threadHandle;
	framework_DeleteMutex(linuxThread->mutexCanDelete);
	framework_FreeMem(linuxThread);
}

void * framework_GetCurrentThreadId()
{
	return (void*)pthread_self();
}

void * framework_GetThreadId(void * threadHandle)
{
	tLinuxThread_t *linuxThread = (tLinuxThread_t*)threadHandle;
	return (void*)linuxThread->thread;
}

void framework_MilliSleep(uint32_t ms)
{
	usleep(1000*ms);
}

/****************** Mutex ****************************/

typedef struct tLinuxMutex
{
	pthread_mutex_t *lock;
	pthread_cond_t  *cond;
}tLinuxMutex_t;

eResult framework_CreateMutex(void ** mutexHandle)
{
	tLinuxMutex_t *mutex = (tLinuxMutex_t *)framework_AllocMem(sizeof(tLinuxMutex_t));
	
	mutex->lock = (pthread_mutex_t*)framework_AllocMem(sizeof(pthread_mutex_t));
	mutex->cond = (pthread_cond_t*)framework_AllocMem(sizeof(pthread_cond_t));
	
	pthread_mutex_init(mutex->lock,NULL);
	pthread_cond_init(mutex->cond,NULL);
	
	*mutexHandle = mutex;
	
	return FRAMEWORK_SUCCESS;
}

void framework_LockMutex(void * mutexHandle)
{
	tLinuxMutex_t *mutex = (tLinuxMutex_t*)mutexHandle;
	
	int res = pthread_mutex_lock(mutex->lock);
	if (res)
	{
		printf("lock() failed errno %i\n",strerror (errno));
	}
}

void framework_UnlockMutex(void * mutexHandle)
{
	tLinuxMutex_t *mutex = (tLinuxMutex_t*)mutexHandle;
	int res = pthread_mutex_unlock(mutex->lock);
	if (res)
	{
		printf("unlock() failed %s\n",strerror (errno));
	}
}

void framework_WaitMutex(void * mutexHandle, uint8_t needLock)
{
	tLinuxMutex_t *mutex = (tLinuxMutex_t*)mutexHandle;
	
	if (needLock)
	{
		framework_LockMutex(mutexHandle);
	}

	pthread_cond_wait(mutex->cond,mutex->lock);
	
	if (needLock)
	{
		framework_UnlockMutex(mutexHandle);
	}
	
}

void framework_NotifyMutex(void * mutexHandle, uint8_t needLock)
{
	tLinuxMutex_t *mutex = (tLinuxMutex_t*)mutexHandle;
	
	if (needLock)
	{
		framework_LockMutex(mutexHandle);
	}

	pthread_cond_broadcast(mutex->cond);
	
	if (needLock)
	{
		framework_UnlockMutex(mutexHandle);
	}
}

void framework_DeleteMutex(void * mutexHandle)
{
	tLinuxMutex_t *mutex = (tLinuxMutex_t*)mutexHandle;
	
	pthread_mutex_destroy(mutex->lock);
	pthread_cond_destroy(mutex->cond);
	
	framework_FreeMem(mutex);
}

/****************** Memory Mgmt ****************************/


typedef struct sMemInfo
{
	uint32_t magic;
	size_t size;
} sMemInfo_t;

typedef struct sMemInfoEnd
{
	uint32_t magicEnd;
} sMemInfoEnd_t;


void* framework_AllocMem(size_t size)
{
	sMemInfo_t *info = NULL;
	sMemInfoEnd_t *infoEnd = NULL;
	uint8_t * pMem = (uint8_t *) malloc(size+sizeof(sMemInfo_t)+sizeof(sMemInfoEnd_t));
	info = (sMemInfo_t*)pMem;
	
	info->magic = 0xDEADC0DE;
	info->size  = size;

	pMem = pMem+sizeof(sMemInfo_t);

	memset(pMem,0xAB,size);

	infoEnd = (sMemInfoEnd_t*)(pMem+size);

	infoEnd->magicEnd = 0xDEADC0DE;
	return pMem;
}

void framework_FreeMem(void *ptr)
{
	if(NULL !=  ptr)
	{
		sMemInfoEnd_t *infoEnd = NULL;
		uint8_t *memInfo = (uint8_t*)ptr;
		sMemInfo_t *info = (sMemInfo_t*)(memInfo - sizeof(sMemInfo_t));

		infoEnd = (sMemInfoEnd_t*)(memInfo+info->size);

		if ((info->magic != 0xDEADC0DE)||(infoEnd->magicEnd != 0xDEADC0DE))
		{
			// Call Debugger
			*(int *)(uintptr_t)0xbbadbeef = 0;
		}else
		{
			memset(info,0x14,info->size+sizeof(sMemInfo_t)+sizeof(sMemInfoEnd_t));
		}
			
		free(info);
	}
}

// Put contents in pipe
void putnfc_contents(string put_bytes)
{
	IPC::ustring bytesrcvd; // ustring
	// Create pipe to communicate with another process
	IPC named_pipe("nfcnci");

	// Make conversion before sending 
	named_pipe.str2vect(&bytesrcvd, put_bytes);
	// Open pipe
	named_pipe.open_fifo('w');
	// write to FIFO (named pipe)
	named_pipe.write_fifo(bytesrcvd);
	// Close FIFO
	named_pipe.close_fifo();

	// Destroy
	// named_pipe.~IPC();
}

int main(int argc, char ** argv)
{
	// system("LD_LIBRARY_PATH=/usr/local/lib; LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/libnfc_nci_linux-1.so; export LD_LIBRARY_PATH"); // Export libary

	if (argc<2)
	{
		cout << "Missing argument" << endl;
		//help(0x00);
	}
	else if (strcmp(argv[1],"poll") == 0)
	{
		cmd_poll(argc - 2, &argv[2]);
	}
	else if(strcmp(argv[1],"write") == 0)
	{
		cmd_write(argc - 2, &argv[2]);
	}
	else if(strcmp(argv[1],"push") == 0)
	{
		cmd_push(argc - 2, &argv[2]);
	}
	else if(strcmp(argv[1],"share") == 0)
	{
		cmd_share(argc - 2, &argv[2]);
	}

	cout << endl;
    
	CleanEnv();


	return 0;
}
