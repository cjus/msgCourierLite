/* imsg.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

imsg.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

imsg.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with imsg.h; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file imsg.h
 @brief Message (Interface) abstact based class
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef _IMSG_H
#define _IMSG_H

#define HRC_MSG_OK				0x0000
#define HRC_MSG_PARSE_FAILED	0x0001

/**
 @class cIMsg
 @brief Message (Interface) abstact based class
*/
class cIMsg
{
public:
	typedef enum MSG_CONNECTION_TYPE
	{
		MSG_CT_NOTSET = -1, MSG_CT_IB_UDP = 0, MSG_CT_OB_UDP = 1,
		MSG_CT_IB_TCP = 2, MSG_CT_OB_TCP = 3, MSG_CT_INTERNAL = 4
	} eMsgConnectionType;

	typedef enum MSG_FORMAT
	{
		MSG_FORMAT_NOTSET = -1, MSG_FORMAT_REQ = 0, MSG_FORMAT_RES = 1, MSG_FORMAT_NOTIFY = 2
	} eMsgFormat;

	typedef enum MSG_REPLY_ACTION
	{
		MSG_REPLY_ACTION_NONE,
		MSG_REPLY_ACTION_WAIT,
		MSG_REPLY_ACTION_NOWAIT,
	}  eMsgReplyActionType;

	typedef enum MSG_NOTIFY_ACTION
	{
		MSG_NOTIFY_ACTION_YES,
		MSG_NOTIFY_ACTION_NO,
	}  eMsgNotifyActionType;

	typedef enum MSG_PROTOCOL
	{
		MSG_PROTOCOL_UNKNOWN,
		MSG_PROTOCOL_UNKNOWN_TCP,
		MSG_PROTOCOL_UNKNOWN_UDP,
		MSG_PROTOCOL_HTTP_MCP,
		MSG_PROTOCOL_MCP,
		MSG_PROTOCOL_HTTP_GET,
		MSG_PROTOCOL_HTTP_POST
	}  eMsgProtocolType;

	typedef enum MSG_PRIORITY
	{
		MSG_PRIORITY_LOWEST = 0,
		MSG_PRIORITY_LOW = 64,
		MSG_PRIORITY_MID = 128,
		MSG_PRIORITY_HIGH = 192,
		MSG_PRIORITY_HIGHEST = 255
	} eMsgPriority;

	typedef enum MSG_STATE
	{
		MSG_STATE_INACTIVE = -1,		// message may be reused
		MSG_STATE_PENDING = 0,			// message has been dispatched and is pending processing
		MSG_STATE_INUSE = 1,			// message has been pushed onto the priority queue and will be processed
		MSG_STATE_UNDELIVERED = 2,		// message could not be delivered
		MSG_STATE_DELIVERED = 3,		// message was delivered
		MSG_STATE_PROCESSED = 4,		// message was successfully processed
		MSG_STATE_MARKFORREUSE = 5		// message was marked for reuse
	} eMsgState;

	virtual const char *GetRawBuffer() = 0;
	virtual int GetRawBufferSize() = 0;

	virtual const char *GetMsgID() = 0;
	virtual eMsgState GetMsgState() = 0;

	virtual int GetTCPSocketHandle() = 0;
	virtual int GetUDPSocketHandle() = 0;

	virtual eMsgConnectionType GetConnectionType() = 0;
	virtual eMsgProtocolType GetProtocolType() = 0;
	virtual eMsgFormat GetFormat() = 0;
	virtual int GetResponseCode() = 0;
	virtual const char *GetCommand() = 0;
	virtual const char *GetTo() = 0;
	virtual const char *GetFrom() = 0;
	virtual const char *GetSubject() = 0;
	virtual const char *GetOptions() = 0;
	virtual eMsgReplyActionType GetReplyAction() = 0;
	virtual eMsgNotifyActionType GetNotifyAction() = 0;
	virtual const char *GetMD5() = 0;
	virtual const char *GetCRC32() = 0;
	virtual const char* GetStatusLine() = 0;
	virtual const char* GetContentType() = 0;
	virtual int GetContentLength() = 0;
	virtual const char *GetContentPayload() = 0;
	virtual const char *GetSourceIP() = 0;
	virtual int GetSourcePort() = 0;
	virtual int GetHeaderSize() = 0;

	virtual int GetMsgPriority() = 0;
	virtual int GetArrivalPort() = 0;

	virtual const char* GetHeaderFieldValue(const char *pFieldName) = 0;

	virtual bool IsKeepAlive() = 0;
    virtual const char *ToString() = 0;

	virtual void SetTCPSocketHandle(int socketfd) = 0;
	virtual void SetUDPSocketHandle(int socketfd) = 0;
	virtual void SetMsgID(const char *pMsgID) = 0;
	virtual void SetConnectionType(eMsgConnectionType ConnectionType) = 0;
	virtual void SetProtocolType(eMsgProtocolType ProtocolType) = 0;
	virtual void SetFormat(eMsgFormat Format) = 0;
	virtual void SetResponseCode(int iResponseCode) = 0;
	virtual void SetCommand(const char *pCommand) = 0;
	virtual void SetTo(const char *pTo) = 0;
	virtual void SetFrom(const char *pFrom) = 0;
	virtual void SetSubject(const char *pSubject) = 0;
	virtual void SetOptions(const char *pOptions) = 0;
	virtual void SetReplyAction(eMsgReplyActionType ReplyActionType) = 0;
	virtual void SetNotifyAction(eMsgNotifyActionType NotifyActionType) = 0;
	virtual void SetContentType(const char *pContentType) = 0;
	virtual void SetContentLength(int iContentLength) = 0;
	virtual void SetContentPayload(const char *pBuffer, int iLength) = 0;
	virtual void SetMsgPriority(int iPriority) = 0;
	virtual void SetCRC32() = 0;
	virtual void SetMD5() = 0;

	virtual void SetArrivalPort(int iPort) = 0;
	virtual void SetSourceIP(const char *pIPAddress) = 0;
	virtual void SetSourcePort(int iPort) = 0;
	virtual void SetKeepAlive(bool bKeepAlive) = 0;

	virtual void MarkProcessed() = 0;
	virtual void DispatchMsg() = 0;
	virtual void DestroyMsg() = 0;
};

#endif //_IMSG_H
