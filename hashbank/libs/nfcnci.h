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

#ifndef _NFCNCI_H
#define _NFCNCI_H

extern "C"
{
	#include <stdint.h>
	#include <pthread.h>
	#include <errno.h>
	#include <unistd.h>
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#include "linux_nfc_api.h"
}

#include <iostream>
#include <vector>
#include <string>

#include "IPC.h"
#include "stopwatch.h"

typedef enum _eResult
{
	FRAMEWORK_SUCCESS,
	FRAMEWORK_FAILED
}eResult;

/**
 * Create a thread using the given function in parameters
 * @param threadHandle Obfuscated thread handle allocated by this function.
 * @param threadedFunc function to start in new thread. ( void* myFunc(void* ctx) )
 * @param ctx Parameters given to threadedFunc.
 * @return SUCCESS if no error.
 */
eResult framework_CreateThread(void** threadHandle,void *(*threadedFunc)(void*),void *ctx);
/**
 * Wait until the given thread finish to run.
 * @param threadHandle Obfuscated thread handle allocated by framework_CreateThread()
 */
void framework_JoinThread(void* threadHandle);
/**
 * Delete the given thread. NOTE : framework_JoinThread() will be called before this function. So
 * the Thread is already stopped.
 * @param threadHandle Obfuscated thread handle allocated by framework_CreateThread()
 */
void framework_DeleteThread(void* threadHandle);
/**
 * Return the calling thread ID.
 * @return thread id.
 */
void* framework_GetCurrentThreadId();
/**
 * Get the thread id of the given thread handle.
 * @param threadHandle Obfuscated thread handle allocated by framework_CreateThread()
 * @return thread id.
 */
void* framework_GetThreadId(void* threadHandle);


/**
 * Create a mutex object. To gain performances, do not implement this function using interprocess
 * lock mechanism such as Semaphore.
 * @param mutexHandle Obfuscated mutex handle allocated by this function.
 * @return SUCCESS if no error.
 */
eResult framework_CreateMutex(void** mutexHandle);
/**
 * Lock the mutex.
 * @param mutexHandle Obfuscated mutex handle allocated by framework_CreateMutex().
 */
void framework_LockMutex(void* mutexHandle);
/**
 * Unlock the mutex
 * @param mutexHandle Obfuscated mutex handle allocated by framework_CreateMutex().
 */
void framework_UnlockMutex(void* mutexHandle);
/**
 * Block the current thread until wake up by framework_NotifyMutex(). 
 * The mutex need to be locked before blocking the thread. (needLock parameter can be used)
 * @param mutexHandle Obfuscated mutex handle allocated by framework_CreateMutex().
 * @param needLock Indicate if the mutex need to be locked internaly or not. This avoid to call lock();wait();unlock();
 */
void framework_WaitMutex(void* mutexHandle,uint8_t needLock);
/**
 * Wake up a thread blocked by the mutex. The mutex must be locked before waking up another thread.
 * The mutex need to be locked before waking up a thread. (needLock parameter can be used)
 * @param mutexHandle Obfuscated mutex handle allocated by framework_CreateMutex().
 * @param needLock Indicate if the mutex need to be locked internaly or not. This avoid to call lock();wait();unlock();
 */
void framework_NotifyMutex(void* mutexHandle,uint8_t needLock);
/**
 * Delete the mutex. If the mutex is locked, any locked thread will be unlocked.
 * @param mutexHandle Obfuscated mutex handle allocated by framework_CreateMutex().
 */
void framework_DeleteMutex(void* mutexHandle);

/**
 * Cause the calling thread to sleep until ms milliseconds elapsed.
 * @param ms Milliseconds to wait until wakeup.
 */
void framework_MilliSleep(uint32_t ms);



// ---------------------------------------------------------------------------------------------------------------------------------------


// void help(int mode);
int InitEnv();
int LookForTag(char** args, int args_len, char* tag, char** data, int format);

typedef enum eDevState
{
    eDevState_NONE,
    eDevState_WAIT_ARRIVAL,
    eDevState_PRESENT,
    eDevState_WAIT_DEPARTURE,
    eDevState_DEPARTED,
    eDevState_EXIT
}eDevState;
typedef enum eSnepClientState
{
    eSnepClientState_WAIT_OFF,
    eSnepClientState_OFF,
    eSnepClientState_WAIT_READY,
    eSnepClientState_READY,
    eSnepClientState_EXIT
}eSnepClientState;
typedef enum eHCEState
{
    eHCEState_NONE,
    eHCEState_WAIT_DATA,
    eHCEState_DATA_RECEIVED,
    eHCEState_EXIT
}eHCEState;
typedef enum eDevType
{
    eDevType_NONE,
    eDevType_TAG,
    eDevType_P2P,
    eDevType_READER
}eDevType;
typedef enum T4T_NDEF_EMU_state_t
{
    Ready,
    NDEF_Application_Selected,
    CC_Selected,
    NDEF_Selected
} T4T_NDEF_EMU_state_t;

static void* g_ThreadHandle = NULL;
static void* g_devLock = NULL;
static void* g_SnepClientLock = NULL;
static void* g_HCELock = NULL;
static eDevState g_DevState = eDevState_NONE;
static eDevType g_Dev_Type = eDevType_NONE;
static eSnepClientState g_SnepClientState = eSnepClientState_OFF;
static eHCEState g_HCEState = eHCEState_NONE;
static nfc_tag_info_t g_TagInfo;
static nfcTagCallback_t g_TagCB;
static nfcHostCardEmulationCallback_t g_HceCB;
static nfcSnepServerCallback_t g_SnepServerCB;
static nfcSnepClientCallback_t g_SnepClientCB;
extern unsigned char *HCE_data;
extern unsigned int HCE_dataLenght;
const unsigned char T4T_NDEF_EMU_APP_Select[] = {0x00,0xA4,0x04,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01};
const unsigned char T4T_NDEF_EMU_CC[] = {0x00, 0x0F, 0x20, 0x00, 0xFF, 0x00, 0xFF, 0x04, 0x06, 0xE1, 0x04, 0x00, 0xFF, 0x00, 0xFF};
const unsigned char T4T_NDEF_EMU_CC_Select[] = {0x00,0xA4,0x00,0x0C,0x02,0xE1,0x03};
const unsigned char T4T_NDEF_EMU_NDEF_Select[] = {0x00,0xA4,0x00,0x0C,0x02,0xE1,0x04};
const unsigned char T4T_NDEF_EMU_Read[] = {0x00,0xB0};
const unsigned char T4T_NDEF_EMU_OK[] = {0x90, 0x00};
const unsigned char T4T_NDEF_EMU_NOK[] = {0x6A, 0x82};
extern unsigned char *pT4T_NdefRecord;
extern unsigned short T4T_NdefRecord_size;

static bool finished_processing = false;

// Vector containing message received
static std::vector<unsigned char> msg_rcvd;

typedef void T4T_NDEF_EMU_Callback_t (unsigned char*, unsigned short);
static T4T_NDEF_EMU_state_t eT4T_NDEF_EMU_State = Ready;
static T4T_NDEF_EMU_Callback_t *pT4T_NDEF_EMU_PushCb = NULL;

/********************************** HCE **********************************/
static void T4T_NDEF_EMU_FillRsp (unsigned char *pRsp, unsigned short offset, unsigned char length);
void T4T_NDEF_EMU_SetRecord(unsigned char *pRecord, unsigned short Record_size, T4T_NDEF_EMU_Callback_t *cb);

void T4T_NDEF_EMU_Reset(void);
void T4T_NDEF_EMU_Next(unsigned char *pCmd, unsigned short Cmd_size, unsigned char *pRsp, unsigned short *pRsp_size);
/********************************** CallBack **********************************/
void onDataReceived(unsigned char *data, unsigned int data_length);
void onHostCardEmulationActivated();
void onHostCardEmulationDeactivated();

void onTagArrival(nfc_tag_info_t *pTagInfo);
void onTagDeparture(void);
void onDeviceArrival (void);

void onDeviceDeparture (void);

void onMessageReceived(unsigned char *message, unsigned int length);
void onSnepClientReady();

void onSnepClientClosed();
 
int InitMode(int tag, int p2p, int hce);
int DeinitPollMode();

int SnepPush(unsigned char* msgToPush, unsigned int len);

int WriteTag(nfc_tag_info_t TagInfo, unsigned char* msgToPush, unsigned int len);

void PrintfNDEFInfo(ndef_info_t pNDEFinfo);
void open_uri (const char* uri);

void PrintNDEFContent(nfc_tag_info_t* TagInfo, ndef_info_t* NDEFinfo, unsigned char* ndefRaw, unsigned int ndefRawLen);
/* mode=1 => poll, mode=2 => push, mode=3 => write, mode=4 => HCE */
int WaitDeviceArrival(int mode, unsigned char* msgToSend, unsigned int len);

void strtolower(char * string) ;

char* strRemovceChar(const char* str, char car);

int convertParamtoBuffer(char* param, unsigned char** outBuffer, unsigned int* outBufferLen);

int BuildNDEFMessage(int arg_len, char** arg, unsigned char** outNDEFBuffer, unsigned int* outNDEFBufferLen);

/* if data = NULL this tag is not followed by dataStr : for example -h --help
if format = 0 tag format -t "text" if format=1 tag format : --type=text */
int LookForTag(char** args, int args_len, char* tag, char** data, int format);
void cmd_poll(int arg_len, char** arg);
 
void cmd_push(int arg_len, char** arg);
void cmd_share(int arg_len, char** arg);
void cmd_write(int arg_len, char** arg);

void* ExitThread(void* pContext);

int InitEnv();

int CleanEnv();

void putnfc_contents(std::string);

static STOPWATCH timer1; 
static double timeout_poll = 5; // Timeout of NFC polling
static int EXCEPTION = 0; // Has any error code happened yet?
#endif
