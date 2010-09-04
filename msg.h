/* msg.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msg.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msg.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with msg.h; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file msg.h
 @brief Message implementation class
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
 Core message implementation class.  See the cIMsg interface.
*/

#ifndef MSG_H
#define MSG_H

#ifdef NDEBUG
	#define MARK_TRAVEL(MSG_PTR) ((void)0)
	#define SET_TOUCH_CODE(MSG_PTR, VAL) ((void)0)
#else
	#define MARK_TRAVEL(MSG_PTR) ((void)0)
//	#define MARK_TRAVEL(MSG_PTR) ((cMsg*)MSG_PTR)->MarkTravel((char*)__PRETTY_FUNCTION__,__LINE__)
	#define SET_TOUCH_CODE(MSG_PTR, VAL) ((cMsg*)MSG_PTR)->SetTouchCode(VAL);
#endif //NDEBUG

#include <string>
#include <sstream>

#include "bench.h"
#include "imsg.h"
#include "threadsync.h"
#include "headerextract.h"
#include "buffer.h"

/**
 @class cMsg
 @brief Message implementation class
*/
class cMsg : public cIMsg
{
public:
	cMsg();
	~cMsg();

	cBuffer *GetBuffer();
	int Parse(eMsgConnectionType ct, eMsgFormat ms);

	const char *GetRawBuffer();
	int GetRawBufferSize();

	int GetTCPSocketHandle();
	int GetUDPSocketHandle();
	const char *GetMsgID();
	cIMsg::eMsgState GetMsgState();
	int GetSendRetryDelay();
	int GetSendRetryCount();
	cIMsg::eMsgConnectionType GetConnectionType();
	cIMsg::eMsgProtocolType GetProtocolType();
	cIMsg::eMsgFormat GetFormat();

	int GetResponseCode();
	const char *GetCommand();
	const char *GetCommandLine();
	const char *GetService();
	const char *GetTo();
	const char *GetFrom();
	const char *GetSubject();
	const char *GetOptions();

	eMsgReplyActionType GetReplyAction();
	eMsgNotifyActionType GetNotifyAction();

	const char* GetStatusLine();

	const char* GetContentType();
	int GetContentLength();
	const char *GetContentPayload();
	const char *GetMD5();
	const char *GetCRC32();
	const char *GetSourceIP();
	int GetSourcePort();
	int GetMsgPriority();

	cHeaderExtract *GetHeaderExtract();
	cBench *GetBench();
	int GetHeaderSize();
	int GetArrivalPort();
	const char* GetHeaderFieldValue(const char *pFieldName);

	void SetTCPSocketHandle(int socketfd);
	void SetUDPSocketHandle(int socketfd);

	void SetMsgID(const char *pMsgID);
	void SetMsgState(cMsg::eMsgState State);
	void SetSendRetryDelay(int iSendRetryDelay);
	void SetSendRetryCount(int iSendRetryCount);

	void SetConnectionType(cIMsg::eMsgConnectionType ConnectionType);
	void SetProtocolType(cIMsg::eMsgProtocolType ProtocolType);
	void SetFormat(cIMsg::eMsgFormat Format);

	void SetResponseCode(int iResponseCode);
	void SetCommand(const char *pCommand);
	void SetTo(const char *pTo);
	void SetFrom(const char *pFrom);
	void SetSubject(const char *pSubject);
	void SetOptions(const char *pOptions);
	void SetReplyAction(eMsgReplyActionType ReplyActionType);
	void SetNotifyAction(eMsgNotifyActionType NotifyActionType);
	void SetStatusLine(const char *pStatusLine, int iStatusLineLength);
	void SetContentType(const char *pContentType);
	void SetContentLength(int iContentLength);
	void SetContentPayload(const char *pBuffer, int iLength);
	void SetContentPayloadPointer(const char *pBuffer);

	void SetMD5();
	void SetCRC32();
	void SetSourceIP(const char *pIPAddress);
	void SetSourcePort(int iPort);
	void SetMsgPriority(int iPriority);
	void SetArrivalPort(int iPort);

	void SetBench(cBench *pBench);
	void UpdateMsgID();
	void MarkProcessed();
	void DispatchMsg();
	void DestroyMsg();
	void ResetMsg();

	int  GetMsgInUseTS();
	int  GetMsgLastRetryTS();
	void SetMsgLastRetryTS();
	void SetCloned();
	void CloseSocket();

	void DeepCopy(cMsg *pToMsg);
	void SetKeepAlive(bool bKeepAlive);
	bool IsKeepAlive();

    const char *ToString();

#ifndef NDEBUG
	void SetTouchCode(int iCode) { m_touchCode = iCode; }
	void MarkTravel(char *pFunctionName, int linenumber);
	void DumpTravel();
#endif //!NDEBUG

private:
	cThreadSync m_ThreadSync;
	cHeaderExtract m_HeaderExtract;
	int TCPSocketHandle;
	int UDPSocketHandle;

	int MsgSendRetryCount;
	int MsgSendRetryDelay;

	std::string MsgID;			// Message GUID
	cIMsg::eMsgConnectionType MsgConnectionType;	// eConnectionType
	cIMsg::eMsgProtocolType MsgProtocolType;		// Protocol message type: MSG_PROTOCOL_HTTP_MCP, MSG_PROTOCOL_MCP
	cIMsg::eMsgFormat  MsgFormat;		// eMsgFormat, Request or Response
	int  MsgResponseCode;		// Message response code if message is not a request
	cMsg::eMsgState MsgState;	// Message state: inactive, pending, inuse
	std::string MsgCommand;		// Message Command type: REGISTER, PRESENCE, SEND etc...
	std::string MsgTo;			// Destination server
	std::string MsgFrom;		// From server
	std::string MsgSubject;		// Subject text
	std::string MsgOptions;		// Message options
	eMsgReplyActionType	MsgReplyActionType; // reply action
	eMsgNotifyActionType MsgNotifyActionType; // notification action
	std::string MsgStatusLine;	// Message Status line
	std::string MsgContentType;	// MIME content type: text/xml, text/base64, etc...
	int  MsgContentLength;		// Actual size of payload content

	cBuffer MsgBuffer;			//
	cBuffer MsgRawBuffer;		// full message
	const char *m_pPayload;		// pointer to start of payload
	int MsgHeaderSize;			// Length of header
	std::string MsgMD5;			// MD5SUM
	std::string MsgCRC32;		// CRC-32 checksum value for content payload
	std::string SourceIP;		// IP address of connecting socket
	int SourcePort;				// Destination Port
	int MsgPriority;			// Message priority, higher values rank the message as more important
	cBench MsgProfiler;			// Used for timing/benchmarks

	int MsgArrivalPort;			// port address that this message arrived on
	std::string HeaderFieldTemp;

	int MsgInUseTS;				// Message In Use Timestamp;
	int MsgLastRetryTS;			// Last time this message was tried to send
	bool MsgCloned;				// true if message was copied into a reply message

	bool m_bKeepAlive;			// should connection be kept alive?
	std::string MsgToString;

#ifndef NDEBUG
	std::vector<std::string> m_Travels;
	int m_touchCode;
#endif //NDEBUG

	void CloseIfCloned();
	void UpdateMsgConnectionType(cIMsg::eMsgConnectionType ConnectionType);
};

#endif //MSG_H

