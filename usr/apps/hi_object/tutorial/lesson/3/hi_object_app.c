/*
**  Copyright 2022 bitValence, Inc.
**  All Rights Reserved.
**
**  This program is free software; you can modify and/or redistribute it
**  under the terms of the GNU Affero General Public License
**  as published by the Free Software Foundation; version 3 with
**  attribution addendums as found in the LICENSE.txt
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Affero General Public License for more details.
**
**  Purpose:
**    Implement the Hi_object application
**
**  Notes:
**   1. This file was automatically generated by cFS Basecamp's app
**      creation tool. If you edit it, your changes will be lost if
**      a new app with the same name is created. 
**
*/


/*
** Includes
*/

#include <string.h>
#include "hi_object_app.h"
#include "hi_object_eds_cc.h"

/***********************/
/** Macro Definitions **/
/***********************/

/* Convenience macros */
#define  INITBL_OBJ    (&(Hi_object.IniTbl))
#define  CMDMGR_OBJ    (&(Hi_object.CmdMgr))
#define  EXOBJ_OBJ     (&(Hi_object.ExObj))


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static int32 InitApp(void);
static int32 ProcessCommands(void);
static void SendStatusTlm(void);

//EX1
/**********************/
/** File Global Data **/
/**********************/

/* 
** Must match DECLARE ENUM() declaration in app_cfg.h
** Defines "static INILIB_CfgEnum_t IniCfgEnum"
*/
DEFINE_ENUM(Config,APP_CONFIG)  

static CFE_EVS_BinFilter_t  EventFilters[] =
{  
   /* Event ID                  Mask */
   {EXOBJ_EXECUTE_EID,  CFE_EVS_FIRST_8_STOP}, // Use CFE_EVS_NO_FILTER to remove the filter
};
//EX1

/*****************/
/** Global Data **/
/*****************/

HI_OBJECT_Class_t  Hi_object;


//EX2
/******************************************************************************
** Function: HI_OBJECT_AppMain
**
*/
void HI_OBJECT_AppMain(void)
{

   uint32 RunStatus = CFE_ES_RunStatus_APP_ERROR;
   
   CFE_EVS_Register(EventFilters, sizeof(EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                    CFE_EVS_EventFilter_BINARY);
//EX2

   if (InitApp() == CFE_SUCCESS)      /* Performs initial CFE_ES_PerfLogEntry() call */
   {
      RunStatus = CFE_ES_RunStatus_APP_RUN; 
   }
   
   /*
   ** Main process loop
   */
   while (CFE_ES_RunLoop(&RunStatus))
   {
      
      RunStatus = ProcessCommands();  /* Pends indefinitely & manages CFE_ES_PerfLogEntry() calls */
      
   } /* End CFE_ES_RunLoop */

   CFE_ES_WriteToSysLog("HI_OBJECT App terminating, run status = 0x%08X\n", RunStatus);   /* Use SysLog, events may not be working */

   CFE_EVS_SendEvent(HI_OBJECT_EXIT_EID, CFE_EVS_EventType_CRITICAL, "HI_OBJECT App terminating, run status = 0x%08X", RunStatus);

   CFE_ES_ExitApp(RunStatus);  /* Let cFE kill the task (and any child tasks) */

} /* End of HI_OBJECT_AppMain() */


/******************************************************************************
** Function: HI_OBJECT_NoOpCmd
**
*/
bool HI_OBJECT_NoOpCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_SendEvent (HI_OBJECT_NOOP_EID, CFE_EVS_EventType_INFORMATION,
                      "No operation command received for HI_OBJECT App version %d.%d.%d",
                      HI_OBJECT_MAJOR_VER, HI_OBJECT_MINOR_VER, HI_OBJECT_PLATFORM_REV);

   return true;


} /* End HI_OBJECT_NoOpCmd() */


//EX3
/******************************************************************************
** Function: HI_OBJECT_ResetAppCmd
**
** Notes:
**   1. Framework objects require an object reference since they are
**      reentrant. Applications use the singleton pattern and store a
**      reference pointer to the object data during construction.
*/
bool HI_OBJECT_ResetAppCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_ResetAllFilters();
   CMDMGR_ResetStatus(CMDMGR_OBJ);
   EXOBJ_ResetStatus();    
   return true;

} /* End HI_OBJECT_ResetAppCmd() */
//EX3


/******************************************************************************
** Function: InitApp
**
*/
static int32 InitApp(void)
{

   int32 Status = APP_C_FW_CFS_ERROR;
   

   /*
   ** Initialize objects 
   */
   
   if (INITBL_Constructor(INITBL_OBJ, HI_OBJECT_INI_FILENAME, &IniCfgEnum))
   {
   
      Hi_object.PerfId  = INITBL_GetIntConfig(INITBL_OBJ, CFG_APP_PERF_ID);
      CFE_ES_PerfLogEntry(Hi_object.PerfId);

      Hi_object.CmdMid        = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_HI_OBJECT_CMD_TOPICID));
      Hi_object.ExecuteMid    = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_HI_OBJECT_EXE_TOPICID));
      Hi_object.SendStatusMid = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_HI_OBJECT_SEND_STATUS_TOPICID));
      
      /*
      ** Constuct app's contained objects
      */
            
      EXOBJ_Constructor(EXOBJ_OBJ, INITBL_OBJ);
      
      /*
      ** Initialize app level interfaces
      */
      
      CFE_SB_CreatePipe(&Hi_object.CmdPipe, INITBL_GetIntConfig(INITBL_OBJ, CFG_APP_CMD_PIPE_DEPTH), INITBL_GetStrConfig(INITBL_OBJ, CFG_APP_CMD_PIPE_NAME));  
      CFE_SB_Subscribe(Hi_object.CmdMid,        Hi_object.CmdPipe);
      CFE_SB_Subscribe(Hi_object.ExecuteMid,    Hi_object.CmdPipe);
      CFE_SB_Subscribe(Hi_object.SendStatusMid, Hi_object.CmdPipe);

      CMDMGR_Constructor(CMDMGR_OBJ);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, HI_OBJECT_NOOP_CC,  NULL, HI_OBJECT_NoOpCmd,     0);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, HI_OBJECT_RESET_CC, NULL, HI_OBJECT_ResetAppCmd, 0);
      
      CMDMGR_RegisterFunc(CMDMGR_OBJ, HI_OBJECT_SET_COUNTER_MODE_CC, EXOBJ_OBJ, EXOBJ_SetModeCmd, sizeof(HI_OBJECT_SetCounterMode_CmdPayload_t));
      /*
      ** Initialize app messages 
      */

      CFE_MSG_Init(CFE_MSG_PTR(Hi_object.StatusTlm.TelemetryHeader), 
                   CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_HI_OBJECT_STATUS_TLM_TOPICID)),
                   sizeof(HI_OBJECT_StatusTlm_t));

      /*
      ** Application startup event message
      */
      CFE_EVS_SendEvent(HI_OBJECT_INIT_APP_EID, CFE_EVS_EventType_INFORMATION,
                        "HI_OBJECT App Initialized. Version %d.%d.%d",
                        HI_OBJECT_MAJOR_VER, HI_OBJECT_MINOR_VER, HI_OBJECT_PLATFORM_REV);

      Status = CFE_SUCCESS; 

   } /* End if INITBL constructed */
   
   return(Status);

} /* End of InitApp() */


/******************************************************************************
** Function: ProcessCommands
**
** 
*/
static int32 ProcessCommands(void)
{
   
   int32  RetStatus = CFE_ES_RunStatus_APP_RUN;
   int32  SysStatus;

   CFE_SB_Buffer_t  *SbBufPtr;
   CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;


   CFE_ES_PerfLogExit(Hi_object.PerfId);
   SysStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, Hi_object.CmdPipe, CFE_SB_PEND_FOREVER);
   CFE_ES_PerfLogEntry(Hi_object.PerfId);

   if (SysStatus == CFE_SUCCESS)
   {
      SysStatus = CFE_MSG_GetMsgId(&SbBufPtr->Msg, &MsgId);

      if (SysStatus == CFE_SUCCESS)
      {

         if (CFE_SB_MsgId_Equal(MsgId, Hi_object.CmdMid))
         {
            CMDMGR_DispatchFunc(CMDMGR_OBJ, &SbBufPtr->Msg);
         } 
         else if (CFE_SB_MsgId_Equal(MsgId, Hi_object.ExecuteMid))
         {
            EXOBJ_Execute();
         }
         else if (CFE_SB_MsgId_Equal(MsgId, Hi_object.SendStatusMid))
         {   
            SendStatusTlm();
         }
         else
         {   
            CFE_EVS_SendEvent(HI_OBJECT_INVALID_MID_EID, CFE_EVS_EventType_ERROR,
                              "Received invalid command packet, MID = 0x%08X", 
                              CFE_SB_MsgIdToValue(MsgId));
         }

      } /* End if got message ID */
   } /* End if received buffer */
   else
   {
      RetStatus = CFE_ES_RunStatus_APP_ERROR;
   } 

   return RetStatus;
   
} /* End ProcessCommands() */


/******************************************************************************
** Function: SendStatusTlm
**
*/
static void SendStatusTlm(void)
{

   HI_OBJECT_StatusTlm_Payload_t *Payload = &Hi_object.StatusTlm.Payload;

   /*
   ** Framework Data
   */
   
   Payload->ValidCmdCnt   = Hi_object.CmdMgr.ValidCmdCnt;
   Payload->InvalidCmdCnt = Hi_object.CmdMgr.InvalidCmdCnt;
   
   /*
   ** Example Object Data
   */

   Payload->CounterMode  = Hi_object.ExObj.CounterMode;
   Payload->CounterValue = Hi_object.ExObj.CounterValue;
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(Hi_object.StatusTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(Hi_object.StatusTlm.TelemetryHeader), true);

} /* End SendStatusTlm() */

