/* msg.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msg.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msg.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with msg.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file msg.cpp
 @brief Message implementation class
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
 Core message implementation class.  See the cIMsg interface.
*/

#include "uniqueidprovider.h"
#include "msg.h"
#include "core.h"
#include "log.h"
#include "exception.h"
#include "url.h"
#include "routingtag.h"

using namespace std;

#define CMSG_MAX_RETRY_DELAY 1 // one second

cMsg::cMsg()
{
	ResetMsg();
}

cMsg::~cMsg()
{
	CloseIfCloned();
	DestroyMsg();
}

cBuffer *cMsg::GetBuffer()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return const_cast<cBuffer*>(&MsgBuffer);
}

int cMsg::Parse(eMsgConnectionType ct, eMsgFormat ms)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
	    MsgRawBuffer.ReplaceWith((char*)MsgBuffer.GetRawBuffer(), MsgBuffer.GetBufferCount());

		char *pData = (char*)MsgBuffer.GetRawBuffer();
		MC_ASSERT(pData);
		int iFirstLine = MsgBuffer.BinaryFind(0, (unsigned char *)"\r\n", 2);
		if (iFirstLine)
			MsgStatusLine.assign(pData, iFirstLine);

		int iEndOfHeader = MsgBuffer.BinaryFind(0, (unsigned char *)"\r\n\r\n", 4);
		if (iEndOfHeader != -1)
		{
			MsgHeaderSize = iEndOfHeader;
			int i = MsgBuffer.Find(" ");
			if (i != -1)
				MsgCommand.assign(pData, i);
		}
		else
		{
			LOGMEMDUMP("pData", (char*)pData, 256);
			MC_ASSERT(0);
		}

		UpdateMsgConnectionType(ct);
		MsgFormat = ms;

		m_HeaderExtract.Parse(&MsgBuffer);

		string connection;
		m_HeaderExtract.GetValue("Connection", connection);
		if (connection.length() > 0 && (connection == "close" || connection == "Close"))
		{
			m_bKeepAlive = false;
		}

		m_HeaderExtract.GetValue("MsgID", MsgID);
		if (MsgID.length() == 0 || MsgID == "0")
		{
			cUniqueIDProvider *pUniqueIDProvider = cUniqueIDProvider::GetInstance();
			pUniqueIDProvider->GetID(MsgID);
		}

		m_HeaderExtract.GetValue("To", MsgTo);
		if (MsgTo.length() != 0)
		{
			cRoutingTag toTag;
			toTag.SetTag(MsgTo.c_str());
			MsgTo = toTag.GetTag();
		}

		m_HeaderExtract.GetValue("From", MsgFrom);
		if (MsgFrom.length() != 0)
		{
			cRoutingTag fromTag;
			fromTag.SetTag(MsgFrom.c_str());
			MsgFrom = fromTag.GetTag();
		}

		// fixup From address for security reasons.
		/*
		string srcIP = SourceIP.c_str();
		string::size_type idx1;
		string::size_type idx2;
		idx1 = MsgFrom.find("@");
		idx2 = MsgFrom.find(":");
		if (idx1 == string::npos)
		{
			MsgFrom.erase(0, idx2);
			MsgFrom.insert(0, srcIP);
		}
		else if (idx1 != string::npos && idx2 != string::npos)
		{
			MsgFrom.erase(idx1+1, idx2-(idx1+1));
			MsgFrom.insert(idx1+1, srcIP);
		}
		*/

		m_HeaderExtract.GetValue("Subject", MsgSubject);

		string sMsgOptions;
		m_HeaderExtract.GetValue("Options", sMsgOptions);
		if (string::npos != sMsgOptions.find("priority-lowest"))
			MsgPriority = cIMsg::MSG_PRIORITY_LOWEST;
		else if (string::npos != sMsgOptions.find("priority-low"))
			MsgPriority = cIMsg::MSG_PRIORITY_LOW;
		else if (string::npos != sMsgOptions.find("priority-mid"))
			MsgPriority = cIMsg::MSG_PRIORITY_MID;
		else if (string::npos != sMsgOptions.find("priority-highest"))
			MsgPriority = cIMsg::MSG_PRIORITY_HIGHEST;
		else if (string::npos != sMsgOptions.find("priority-high"))
			MsgPriority = cIMsg::MSG_PRIORITY_HIGH;
		else
			MsgPriority = cIMsg::MSG_PRIORITY_LOWEST;

		if (string::npos != sMsgOptions.find("reply-none"))
			MsgReplyActionType = cMsg::MSG_REPLY_ACTION_NONE;
		else if (string::npos != sMsgOptions.find("reply-wait"))
			MsgReplyActionType = cMsg::MSG_REPLY_ACTION_WAIT;
		else if (string::npos != sMsgOptions.find("reply-nowait"))
			MsgReplyActionType = cMsg::MSG_REPLY_ACTION_NOWAIT;
		else
		{
			if (MsgConnectionType == cMsg::MSG_CT_OB_UDP || MsgConnectionType == cMsg::MSG_CT_IB_UDP)
				MsgReplyActionType = cMsg::MSG_REPLY_ACTION_NOWAIT;
			if (MsgConnectionType == cMsg::MSG_CT_OB_TCP || MsgConnectionType == cMsg::MSG_CT_IB_TCP)
				MsgReplyActionType = cMsg::MSG_REPLY_ACTION_WAIT;
		}

		if (string::npos != sMsgOptions.find("notify-yes"))
			MsgNotifyActionType = cMsg::MSG_NOTIFY_ACTION_YES;
		else if (string::npos != sMsgOptions.find("notify-no"))
			MsgNotifyActionType = cMsg::MSG_NOTIFY_ACTION_NO;
		else
		{
			MsgNotifyActionType = cMsg::MSG_NOTIFY_ACTION_NO;
		}

		m_HeaderExtract.GetValue("CRC32",MsgCRC32);
		m_HeaderExtract.GetValue("Content-Type",MsgContentType);

		MsgContentLength = 0;
		string sContentLength;
		m_HeaderExtract.GetValue("Content-Length",sContentLength);
		if (sContentLength.length())
		{
			MsgContentLength = atoi(sContentLength.c_str());
			m_pPayload = (const char*)((char*)MsgBuffer.GetRawBuffer());
			m_pPayload += iEndOfHeader+4;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return HRC_MSG_OK;
}

const char *cMsg::GetRawBuffer()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	//LOGMEMDUMP("MsgBuffer", (char*)MsgRawBuffer.GetRawBuffer(), MsgRawBuffer.GetBufferCount());
	return reinterpret_cast<const char*>(MsgRawBuffer.GetRawBuffer());
}

int cMsg::GetRawBufferSize()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgRawBuffer.GetBufferCount();
}

int cMsg::GetTCPSocketHandle()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return TCPSocketHandle;
}

int cMsg::GetUDPSocketHandle()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return UDPSocketHandle;
}

const char *cMsg::GetMsgID()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgID.c_str();
}

cIMsg::eMsgState cMsg::GetMsgState()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgState;
}

int cMsg::GetSendRetryDelay()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgSendRetryDelay;
}

int cMsg::GetSendRetryCount()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgSendRetryCount;
}

cIMsg::eMsgConnectionType cMsg::GetConnectionType()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgConnectionType;
}

cIMsg::eMsgProtocolType cMsg::GetProtocolType()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgProtocolType;
}

cIMsg::eMsgFormat cMsg::GetFormat()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgFormat;
}

int cMsg::GetResponseCode()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgResponseCode;
}

const char *cMsg::GetCommand()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgCommand.c_str();
}

const char *cMsg::GetTo()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgTo.c_str();
}

const char *cMsg::GetFrom()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgFrom.c_str();
}

const char *cMsg::GetSubject()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgSubject.c_str();
}

const char *cMsg::GetOptions()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgOptions.c_str();
}

cIMsg::eMsgReplyActionType cMsg::GetReplyAction()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgReplyActionType;
}

cIMsg::eMsgNotifyActionType cMsg::GetNotifyAction()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgNotifyActionType;
}

const char* cMsg::GetStatusLine()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return (char*)MsgStatusLine.c_str();
}

const char* cMsg::GetContentType()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgContentType.c_str();
}

int cMsg::GetContentLength()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgContentLength;
}

const char *cMsg::GetContentPayload()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return m_pPayload;
}

const char *cMsg::GetCRC32()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgCRC32.c_str();
}

const char *cMsg::GetMD5()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgMD5.c_str();
}

const char *cMsg::GetSourceIP()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return SourceIP.c_str();
}

int cMsg::GetSourcePort()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return SourcePort;
}

int cMsg::GetMsgPriority()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgPriority;
}

cHeaderExtract *cMsg::GetHeaderExtract()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return &m_HeaderExtract;
}

cBench *cMsg::GetBench()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return &MsgProfiler;
}

int cMsg::GetHeaderSize()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgHeaderSize;
}

int cMsg::GetArrivalPort()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgArrivalPort;
}

const char* cMsg::GetHeaderFieldValue(const char *pFieldName)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	HeaderFieldTemp = m_HeaderExtract.GetValue(pFieldName);
	return HeaderFieldTemp.c_str();
}

void cMsg::SetTCPSocketHandle(int socketfd)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	TCPSocketHandle = socketfd;
}

void cMsg::SetMsgID(const char *pMsgID)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgID = pMsgID;
}

void cMsg::SetMsgState(cMsg::eMsgState State)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MARK_TRAVEL(this);
	MsgState = State;
}

void cMsg::SetSendRetryDelay(int iSendRetryDelay)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MARK_TRAVEL(this);
	MsgSendRetryDelay = iSendRetryDelay;
}

void cMsg::SetSendRetryCount(int iSendRetryCount)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgSendRetryCount = iSendRetryCount;
}

void cMsg::SetStatusLine(const char *pStatusLine, int iStatusLineLength)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgStatusLine.assign(pStatusLine, iStatusLineLength);
}

void cMsg::SetConnectionType(cIMsg::eMsgConnectionType ConnectionType)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MARK_TRAVEL(this);
	UpdateMsgConnectionType(ConnectionType);
	MsgConnectionType = ConnectionType;
	if (MsgConnectionType == cIMsg::MSG_CT_INTERNAL)
	{
		MsgPriority = cIMsg::MSG_PRIORITY_HIGH;
		MsgProtocolType = cIMsg::MSG_PROTOCOL_MCP;
		MsgTo = "";
		MsgFrom = "";
	}
}

void cMsg::SetProtocolType(cIMsg::eMsgProtocolType ProtocolType)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgProtocolType = ProtocolType;
}

void cMsg::SetFormat(cIMsg::eMsgFormat Format)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgFormat = Format;
}

void cMsg::SetResponseCode(int iResponseCode)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgResponseCode = iResponseCode;
}

void cMsg::SetCommand(const char *pCommand)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgCommand = pCommand;
}

void cMsg::SetTo(const char *pTo)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgTo = pTo;
}

void cMsg::SetFrom(const char *pFrom)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgFrom = pFrom;
}

void cMsg::SetSubject(const char *pSubject)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgSubject = pSubject;
}

void cMsg::SetOptions(const char *pOptions)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgOptions = pOptions;
}

void cMsg::SetReplyAction(cIMsg::eMsgReplyActionType ReplyActionType)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgReplyActionType = ReplyActionType;
}

void cMsg::SetNotifyAction(cIMsg::eMsgNotifyActionType NotifyActionType)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgNotifyActionType = NotifyActionType;
}

void cMsg::SetContentType(const char *pContentType)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgContentType = pContentType;
}

void cMsg::SetContentLength(int iContentLength)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgContentLength = iContentLength;
}

void cMsg::SetContentPayload(const char *pBuffer, int iLength)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgBuffer.ReplaceWith((char*)pBuffer, iLength);
	m_pPayload = (const char*)MsgBuffer.GetRawBuffer();
	MsgContentLength = iLength;
}

void cMsg::SetContentPayloadPointer(const char *pBuffer)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_pPayload = pBuffer;
}

void cMsg::SetCRC32()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	//TODO: calc CRC32 for existing payload
	MsgCRC32 = "0";
}

void cMsg::SetMD5()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	//TODO: calc MD5 for existing payload
}

void cMsg::SetSourceIP(const char *pIPAddress)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MC_ASSERT(pIPAddress != NULL);
	SourceIP = pIPAddress;
}

void cMsg::SetSourcePort(int iPort)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	SourcePort = iPort;
}

void cMsg::SetMsgPriority(int iPriority)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgPriority = iPriority;
}

void cMsg::SetArrivalPort(int iPort)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgArrivalPort = iPort;
}

void cMsg::SetUDPSocketHandle(int iSocket)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	UDPSocketHandle = iSocket;
}

void cMsg::UpdateMsgID()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	cUniqueIDProvider *pUniqueIDProvider = cUniqueIDProvider::GetInstance();
	pUniqueIDProvider->GetID(MsgID);
MC_ASSERT(MsgID.c_str());
	//LOG2("MsgID was blank or 0. Setting MsgID to [%s]", MsgID.c_str());
}

void cMsg::MarkProcessed()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MARK_TRAVEL(this);
	CloseIfCloned();
	MsgState = MSG_STATE_DELIVERED;
}

void cMsg::DispatchMsg()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
		MARK_TRAVEL(this);
		if (MsgConnectionType == MSG_CT_NOTSET)
			MsgConnectionType = MSG_CT_INTERNAL;
		else if (MsgConnectionType == MSG_CT_OB_UDP)
		{
			cCore *pCore = cCore::GetInstance();
			stringstream ss (stringstream::in | stringstream::out);
			ss << pCore->GetServerName() << "@"
				<< pCore->GetServerPrimaryAddr() << ":"
				<< MsgArrivalPort;
			MsgFrom = ss.str();
		}
		MsgState = MSG_STATE_PENDING;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cMsg::DestroyMsg()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MARK_TRAVEL(this);
}

void cMsg::ResetMsg()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	try
	{
		CloseIfCloned();

		///m_QueuedForDB = false;
		MsgProfiler.Stop();
		MsgProfiler.Start();

        cUniqueIDProvider *pUniqueIDProvider = cUniqueIDProvider::GetInstance();
        pUniqueIDProvider->GetID(MsgID);

		TCPSocketHandle = -1;

		MsgSendRetryCount = 0;
		MsgSendRetryDelay = CMSG_MAX_RETRY_DELAY;
		MsgReplyActionType = MSG_REPLY_ACTION_NONE;
		MsgNotifyActionType = MSG_NOTIFY_ACTION_NO;
		MsgConnectionType = MSG_CT_NOTSET;
		MsgFormat = MSG_FORMAT_NOTSET;
		MsgProtocolType = MSG_PROTOCOL_UNKNOWN;
		MsgResponseCode = 0;
		MsgState = MSG_STATE_INACTIVE;
		MsgCommand = "";
		MsgTo = "";
		MsgFrom = "";
		MsgSubject = "";
		MsgOptions = "";
		MsgCRC32 = "";
		MsgStatusLine = "";
		MsgContentType = "";
		MsgContentLength = 0;
		MsgPriority = MSG_PRIORITY_MID;
		m_pPayload = 0;
		MsgHeaderSize = 0;
		MsgBuffer.Reset();
		MsgRawBuffer.Reset();
		MsgArrivalPort = 0;
		MsgInUseTS = time(0);
		MsgLastRetryTS = 0;
		MsgCloned = false;
		SourceIP = "";
		SourcePort = -1;
		UDPSocketHandle = -1;
//		MsgService = 0;
		m_bKeepAlive = true;
#ifndef NDEBUG
		m_touchCode = 0;
		m_Travels.erase(m_Travels.begin(), m_Travels.end());
#endif //!NDEBUG
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

int  cMsg::GetMsgInUseTS()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgInUseTS;
}

int  cMsg::GetMsgLastRetryTS()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return MsgLastRetryTS;
}

void cMsg::SetMsgLastRetryTS()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgLastRetryTS = time(0);
}

void cMsg::SetBench(cBench *pBench)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgProfiler = *pBench;
}

void cMsg::SetCloned()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	MsgCloned = true;
}

void cMsg::CloseIfCloned()
{
/*
	if (MsgCloned == false && MsgConnectionType==MSG_CT_IB_TCP && TCPSocketHandle != -1)
	{
		// used for inbound TCP message which was processed
		// but did not result in a response message.  This means
		// that this message needs to close its TCP socket handle.
		#ifdef _PLATFORM_LINUX
LOG2("close(%d) called", TCPSocketHandle);
			close(TCPSocketHandle);
		#endif //_PLATFORM_LINUX
		#ifdef _PLATFORM_WIN32
			closesocket(TCPSocketHandle);
		#endif //_PLATFORM_WIN32
		TCPSocketHandle = -1;
	}
*/
}

void  cMsg::CloseSocket()
{
	if (TCPSocketHandle != -1)
	{
		#ifdef _PLATFORM_LINUX
LOG2("close(%d) called", TCPSocketHandle);
			close(TCPSocketHandle);
		#endif //_PLATFORM_LINUX
		#ifdef _PLATFORM_WIN32
			closesocket(TCPSocketHandle);
		#endif //_PLATFORM_WIN32
		TCPSocketHandle = -1;
	}
}

void cMsg::UpdateMsgConnectionType(cIMsg::eMsgConnectionType ConnectionType)
{
	MsgConnectionType = ConnectionType;
	if (MsgConnectionType == cIMsg::MSG_CT_INTERNAL)
	{
		MsgPriority = cIMsg::MSG_PRIORITY_HIGH;
		MsgProtocolType = cIMsg::MSG_PROTOCOL_MCP;
		MsgTo = "";
		MsgFrom = "";
	}
}

void cMsg::DeepCopy(cMsg *pToMsg)
{
	pToMsg->SetTCPSocketHandle(this->GetTCPSocketHandle());
	pToMsg->SetUDPSocketHandle(this->GetUDPSocketHandle());
	pToMsg->SetMsgID(this->GetMsgID());
	pToMsg->SetMsgState(this->GetMsgState());
	pToMsg->SetConnectionType(this->GetConnectionType());
	pToMsg->SetProtocolType(this->GetProtocolType());
	pToMsg->SetFormat(this->GetFormat());
	pToMsg->SetResponseCode(this->GetResponseCode());
	pToMsg->SetCommand(this->GetCommand());
	pToMsg->SetTo(this->GetTo());
	pToMsg->SetFrom(this->GetFrom());
	pToMsg->SetSubject(this->GetSubject());
	pToMsg->SetOptions(this->GetOptions());
	pToMsg->SetReplyAction(this->GetReplyAction());
	pToMsg->SetNotifyAction(this->GetNotifyAction());
	pToMsg->SetStatusLine(this->GetStatusLine(), strlen(this->GetStatusLine()));
	pToMsg->SetContentType(this->GetContentType());
	pToMsg->SetContentLength(this->GetContentLength());
	pToMsg->SetContentPayload(this->m_pPayload, this->MsgContentLength);
	pToMsg->SetContentPayloadPointer(this->m_pPayload);
	pToMsg->SetSourceIP(this->GetSourceIP());
	pToMsg->SetSourcePort(this->GetSourcePort());
	pToMsg->SetMsgPriority(this->GetMsgPriority());
	pToMsg->SetArrivalPort(this->GetArrivalPort());
	pToMsg->SetBench(this->GetBench());
	pToMsg->SetKeepAlive(this->IsKeepAlive());
}

void cMsg::SetKeepAlive(bool bKeepAlive)
{
	m_bKeepAlive = bKeepAlive;
}

bool cMsg::IsKeepAlive()
{
	return m_bKeepAlive;
}

const char *cMsg::ToString()
{
    MsgToString = "";
    if (MsgCommand.length())
    {
        MsgToString += MsgCommand.c_str();
        MsgToString += "\n";
    }
    if (MsgID.length())
    {
        MsgToString += "MsgID: ";
        MsgToString += MsgID.c_str();
        MsgToString += "\n";
    }
    if (IsKeepAlive())
        MsgToString += "Connection: keep-alive\n";
    else
        MsgToString += "Connection: close\n";
    if (MsgTo.length())
    {
        MsgToString += "To: ";
        MsgToString += MsgTo.c_str();
        MsgToString += "\n";
    }
    if (MsgFrom.length())
    {
        MsgToString += "From: ";
        MsgToString += MsgFrom.c_str();
        MsgToString += "\n";
    }

    return MsgToString.c_str();
}

#ifndef NDEBUG
void cMsg::MarkTravel(char *pFunctionName, int linenumber)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
		stringstream ss (stringstream::in | stringstream::out);
		ss	<< pFunctionName << "::" << linenumber;
		m_Travels.push_back(ss.str());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}
void cMsg::DumpTravel()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
		LOG("Dumping msg(%X)", this);
		vector<string>::iterator it;
		for (it = m_Travels.begin(); it != m_Travels.end(); it++)
		{
			if ((*it).size() > 0)
				LOG("     %s", (*it).c_str());
		}
		LOG(" ");
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}
#endif //!NDEBUG
