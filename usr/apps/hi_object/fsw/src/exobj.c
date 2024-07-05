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
**    Implement the EXOBJ_Class methods 
**
**  Notes:
**    None
**
*/

/*
** Include Files:
*/

#include <string.h>
#include "exobj.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*******************************/
/** Local Function Prototypes **/
/*******************************/

const char *CounterModeStr(HI_OBJECT_CounterMode_Enum_t  CounterMode);

/**********************/
/** Global File Data **/
/**********************/

static EXOBJ_Class_t *ExObj = NULL;


/******************************************************************************
** Function: EXOBJ_Constructor
**
*/
void EXOBJ_Constructor(EXOBJ_Class_t *ExObjPtr,
                       const INITBL_Class_t *IniTbl)
{
 
   ExObj = ExObjPtr;

   CFE_PSP_MemSet((void*)ExObj, 0, sizeof(EXOBJ_Class_t));
    
   ExObj->CounterMode  = HI_OBJECT_CounterMode_Increment;
   ExObj->CounterValue = EXOBJ_COUNTER_LO_LIM;
   ExObj-> CounterLoLim = INITBL_GetIntConfig(IniTbl,CFG_EXOBJ_counter_LO_LIM);      
   ExObj-> CounterHiLim = INITBL_GetIntConfig(IniTbl,CFG_EXOBJ_counter_HI_LIM);


} /* End EXOBJ_Constructor */


/******************************************************************************
** Function: EXOBJ_Execute
**
*/
void EXOBJ_Execute(void)
{
   
   if (ExObj->CounterMode == HI_OBJECT_CounterMode_Increment)
   {
      if (ExObj->CounterValue < ExObj->CounterHiLim)
      {
         ExObj->CounterValue++;
      }
      else
      {
         ExObj->CounterValue = ExObj->CounterLoLim;
      }
   } /* End if increment */
   else
   {
      if (ExObj->CounterValue >  EXOBJ_COUNTER_LO_LIM)
      {
         ExObj->CounterValue--;
      }
      else
      {
         ExObj->CounterValue = ExObj->CounterHiLim;
      }
   } /* End if decrement */
   
   
   CFE_EVS_SendEvent (EXOBJ_EXECUTE_EID, CFE_EVS_EventType_INFORMATION,
                      "%s counter mode: Value %d", 
                      CounterModeStr(ExObj->CounterMode), ExObj->CounterValue);

   
} /* End EXOBJ_Execute() */


/******************************************************************************
** Function:  EXOBJ_ResetStatus
**
*/
void EXOBJ_ResetStatus()
{
 
   return;
   
} /* End EXOBJ_ResetStatus() */


/******************************************************************************
** Function: EXOBJ_SetModeCmd
**
*/
bool EXOBJ_SetModeCmd(void *DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{
   
   bool RetStatus = false;
   const HI_OBJECT_SetCounterMode_CmdPayload_t *Cmd = CMDMGR_PAYLOAD_PTR(MsgPtr, HI_OBJECT_SetCounterMode_t);
   HI_OBJECT_CounterMode_Enum_t PrevMode = ExObj->CounterMode;
   
   if ((Cmd->Mode == HI_OBJECT_CounterMode_Increment) ||
       (Cmd->Mode == HI_OBJECT_CounterMode_Decrement))
   {
   
      RetStatus = true;
      
      ExObj->CounterMode = Cmd->Mode;
      
      CFE_EVS_SendEvent (EXOBJ_SET_MODE_CMD_EID, CFE_EVS_EventType_INFORMATION,
                         "Counter mode changed from %s to %s",
                         CounterModeStr(PrevMode), CounterModeStr(ExObj->CounterMode));

   }
   else
   {
      CFE_EVS_SendEvent (EXOBJ_SET_MODE_CMD_EID, CFE_EVS_EventType_ERROR,
                         "Set counter mode rejected. Invalid mode %d",
                         Cmd->Mode);
   }
   
   return RetStatus;

} /* End EXOBJ_SetModeCmd() */


/******************************************************************************
** Function: CounterModeStr
**
** Type checking should enforce a valid CounterMode parameter adn the fact 
** that the array must match the HI_OBJECT_CounterMode_Enum_t definition.
*/
const char *CounterModeStr(HI_OBJECT_CounterMode_Enum_t  CounterMode)
{

   static const char *CounterModeEnumStr[] =
   {
      "UNDEFINED", 
      "INCREMENT",    /* HI_OBJECT_CounterMode_Increment */
      "DECREMENT"     /* HI_OBJECT_CounterMode_Deccrement */
   };
        
   return CounterModeEnumStr[CounterMode];

} /* End CounterModeStr() */


