#ifndef __vos_fastq_Compat_h
#define __vos_fastq_Compat_h 1

/** 模块名大小 */
#define MODULE_NAME_LEN (48)

/** 模块最大ID限制 */
#define MODULE_ID_MAX (256/*1024*/)

#define MODULE_ID_IS_ILLEGAL(id) ((id) >= MODULE_ID_MAX || (id) < 0)



ULONG VOS_QueCreate(LONG lMaxMsgs, LONG lOptions);
LONG VOS_QueDelete(ULONG msgQId);
LONG VOS_QueSend(ULONG msgQId, ULONG aulMsg[VOS_QUEUE_MSG_SIZE], LONG lMsec, LONG lPriority);
LONG VOS_QueReceive(ULONG msgQId, ULONG aulMsg[VOS_QUEUE_MSG_SIZE], LONG lMsec);
VOID VOS_QueBindTask(VOS_HANDLE hTask, ULONG ulQId);
LONG VOS_QueNum(ULONG msgQId);
LONG VOS_QueClean();


LONG VOS_SendAsynMsg2Module(LONG dst_slot,CHAR dst_moduleName[MODULE_NAME_LEN],CHAR src_moduleName[MODULE_NAME_LEN], LONG msgCode,VOID *msgData,LONG msgDataLen);

LONG VOS_SendAsynMsg2Module_byID(LONG dst_slot,ULONG dst_moduleID,ULONG src_moduleID, LONG msgCode,VOID *msgData,LONG msgDataLen,LONG memType);

LONG VOS_SendSynMsg2Module(LONG dst_slot,CHAR dst_moduleName[MODULE_NAME_LEN],CHAR src_moduleName[MODULE_NAME_LEN], LONG msgCode,VOID *msgData,LONG msgDataLen,VOID *ackData,LONG *ackDataLen,LONG timeout);

LONG VOS_SendSynAckMsg(ULONG aulMsg[VOS_QUEUE_MSG_SIZE],VOID *ackData,LONG ackDataLen);

LONG VOS_RegTimerMsg(LONG module_ID,LONG msg_code,LONG interval,VOS_TIMER_TYPE_EN type,VOID  *pArg);

LONG VOS_DeregTimerMsg(LONG module_ID,LONG msg_code);




#endif /* __vos_fastq_Compat_h */

