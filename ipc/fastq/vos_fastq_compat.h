#ifndef __vos_fastq_Compat_h
#define __vos_fastq_Compat_h 1

/** 模块名大小 */
#define MODULE_NAME_LEN (48)

/** 模块最大ID限制 */
#define MODULE_ID_MAX (256/*1024*/)

#define MODULE_ID_IS_ILLEGAL(id) ((id) >= MODULE_ID_MAX || (id) < 0)


typedef LONG (*receive_handler)(VOID *buf,LONG size,VOID *opval,LONG opvalLen);

LONG VOS_module_get_name(ULONG id,CHAR name[MODULE_NAME_LEN]);
ULONG VOS_module_get_Id(CHAR *name);
LONG VOS_SendAsynMsg2Module(LONG dst_slot,CHAR dst_moduleName[MODULE_NAME_LEN],CHAR src_moduleName[MODULE_NAME_LEN],
                                    LONG msgCode,VOID *msgData,LONG msgDataLen);
LONG VOS_SendAsynMsg2Module_byID(LONG dst_slot,ULONG dst_moduleID,ULONG src_moduleID,
                                    LONG msgCode,VOID *msgData,LONG msgDataLen,LONG memType);
LONG VOS_SendAsynMsg2Module_byID_withQue(LONG dst_slot,ULONG dst_moduleID,ULONG src_moduleID,
								LONG msgCode,VOID *msgData,LONG msgDataLen,LONG memType);
LONG VOS_SendSynMsg2Module(LONG dst_slot,CHAR dst_moduleName[MODULE_NAME_LEN],CHAR src_moduleName[MODULE_NAME_LEN],
                                  LONG msgCode,VOID *msgData,LONG msgDataLen,VOID *ackData,LONG *ackDataLen,LONG timeout);
//LONG VOS_SendAsynMsg2ModuleEx(LONG dst_slot,CHAR dst_moduleName[MODULE_NAME_LEN],CHAR src_moduleName[MODULE_NAME_LEN],
//                                      LONG msgCode,VOID *msgData,LONG msgDataLen,
//                                      module_comm_type_t commType,VOID *optval,LONG optlen);
//LONG VOS_SendSynMsg2ModuleEx(LONG dst_slot,CHAR dst_moduleName[MODULE_NAME_LEN],CHAR src_moduleName[MODULE_NAME_LEN],
//                                    LONG msgCode,VOID *msgData,LONG msgDataLen,VOID *ackData,LONG *ackDataLen,LONG timeout,
//                                    module_comm_type_t commType,VOID *optval,LONG optlen);
//LONG VOS_SendAsynMsg2Module_Raw(LONG dst_slot,CHAR dst_moduleName[MODULE_NAME_LEN],CHAR src_moduleName[MODULE_NAME_LEN],
//                                    ULONG aulMsg[VOS_QUEUE_MSG_SIZE]);
//LONG VOS_SendSynAckMsg(ULONG aulMsg[VOS_QUEUE_MSG_SIZE],VOID *ackData,LONG ackDataLen);
//LONG VOS_RegTimerMsg(LONG module_ID,LONG msg_code,LONG interval,VOS_TIMER_TYPE_EN type,VOID  *pArg);
//LONG VOS_DeregTimerMsg(LONG module_ID,LONG msg_code);


/** 从原始消息中获取消息类型 （同步/异步） */
#define VOS_MOD_MSG_GET_EVENT_TYPE(aulMsg)       aulMsg[0]

/** 从原始消息中获取消息类码 */
#define VOS_MOD_MSG_GET_CODE(aulMsg)         ((VOS_MOD_EVENT_TYPE_ASYN_ID == VOS_MOD_MSG_GET_EVENT_TYPE(aulMsg)) ? ((int)aulMsg[5]) : (((SYS_MSG_S *)(aulMsg[3]))->usMsgCode))


/** 从原始消息中获取源模块号 */
#define VOS_MOD_MSG_GET_SRC_ID(aulMsg)       ((VOS_MOD_EVENT_TYPE_ASYN_ID == VOS_MOD_MSG_GET_EVENT_TYPE(aulMsg)) ? (aulMsg[2]) : (((SYS_MSG_S *)(aulMsg[3]))->ulSrcModuleID))


/** 从原始消息中获取目的模块号 */
#define VOS_MOD_MSG_GET_DST_ID(aulMsg)       ((VOS_MOD_EVENT_TYPE_ASYN_ID == VOS_MOD_MSG_GET_EVENT_TYPE(aulMsg)) ? (aulMsg[1]) : (((SYS_MSG_S *)(aulMsg[3]))->ulDstModuleID))


/** 从原始消息中获取源槽位号 */
#define VOS_MOD_MSG_GET_SRC_SLOT(aulMsg)     ((VOS_MOD_EVENT_TYPE_ASYN_ID == VOS_MOD_MSG_GET_EVENT_TYPE(aulMsg)) ? (0) :(((SYS_MSG_S *)(aulMsg[3]))->ulSrcSlotID))


/** 从原始消息中获取目的槽位号 */
#define VOS_MOD_MSG_GET_DST_SLOT(aulMsg)     ((VOS_MOD_EVENT_TYPE_ASYN_ID == VOS_MOD_MSG_GET_EVENT_TYPE(aulMsg)) ? (0) :(((SYS_MSG_S *)(aulMsg[3]))->ulDstSlotID))


/** 从原始消息中获取消息长度 */
#define VOS_MOD_MSG_GET_LEN(aulMsg)        ((VOS_MOD_EVENT_TYPE_ASYN_ID == VOS_MOD_MSG_GET_EVENT_TYPE(aulMsg)) ? (aulMsg[4]) :(((SYS_MSG_S *)(aulMsg[3]))->usFrameLen))

/** 从原始消息中获取消息buf 的指针 */
#define VOS_MOD_MSG_GET_DATAPTR(aulMsg)    ((VOS_MOD_EVENT_TYPE_ASYN_ID == VOS_MOD_MSG_GET_EVENT_TYPE(aulMsg)) ? ((void*)aulMsg[3]) :(((SYS_MSG_S *)(aulMsg[3]))->ptrMsgBody))

/** 从原始消息中拷贝消息到dst中 */
#define VOS_MOD_MSG_GET_DATA(dst,aulMsg)   if( NULL != (VOID *)(VOS_MOD_MSG_GET_DATAPTR(aulMsg)) )                                                 \
                                           {  VOS_MemCpy(dst,VOS_MOD_MSG_GET_DATAPTR(aulMsg),VOS_MOD_MSG_GET_LEN(aulMsg));  }           \
                                           else                                                                                         \
                                           {  VOS_ASSERT(0);   }

/** 释放原始消息中的sys msg */
#define VOS_MOD_MSG_FREE(aulMsg)           (VOS_MOD_EVENT_TYPE_ASYN_ID == VOS_MOD_MSG_GET_EVENT_TYPE(aulMsg)) ? VOS_Printf("can't free ASYN_ID msg data\r\n"),VOS_ASSERT(0) :(VOS_Free((VOID *)aulMsg[3]))


/** 释放原始消息中的mem type */
#define VOS_MOD_MSG_MEM_TYPE(aulMsg)       (VOS_MOD_EVENT_TYPE_ASYN_ID == VOS_MOD_MSG_GET_EVENT_TYPE(aulMsg)) ? (aulMsg[6]) : 0


#endif /* __vos_fastq_Compat_h */

