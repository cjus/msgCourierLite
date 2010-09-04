/* msgpool.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgpool.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgpool.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with msgpool.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file msgpool.cpp
 @brief Message Pool
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "msgpool.h"
#include "exception.h"
#include "log.h"

using namespace std;

#define MP_MIN_MESSAGES 100 // minimum number of messages retained during compact

/**
 * cMsgPool Constructor
 */
cMsgPool::cMsgPool()
: m_bLocked(false)
{
}

/**
 * cMsgPool Destructor
 */
cMsgPool::~cMsgPool()
{
	try
	{
		list<cMsg*>::iterator MessagesIterator;
		for (MessagesIterator = m_Messages.begin();
			 MessagesIterator != m_Messages.end();
			 MessagesIterator++)
		{
			delete (*MessagesIterator);
			*MessagesIterator = 0;
		}
		m_Messages.erase(m_Messages.begin(),m_Messages.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

cMsg* cMsgPool::GetReusableMessage()
{
	cMsg *pNewMsg = 0;
	try
	{
		if (m_FreeMsgStack.empty() == false)
		{
			pNewMsg = m_FreeMsgStack.top();
			m_FreeMsgStack.pop();
		}
		else
		{
			MC_NEW(pNewMsg, cMsg);
			m_Messages.push_back(pNewMsg);
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("Problem in cMsgPool::GetReusableMessage()");
		THROW("Problem in cMsgPool::GetReusableMessage()");
	}

	MARK_TRAVEL(pNewMsg);
	pNewMsg->ResetMsg();

	// Mark message inuse so that another thread doesn't grab it
	pNewMsg->SetMsgState(cMsg::MSG_STATE_INUSE);

	SET_TOUCH_CODE(pNewMsg, 6);
	return pNewMsg;
}

void cMsgPool::ReturnToPool(cMsg *pMsg)
{
	try
	{
		SET_TOUCH_CODE(pMsg, 7);
		pMsg->SetMsgState(cMsg::MSG_STATE_INACTIVE);
		m_FreeMsgStack.push(pMsg);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

cIMsg *cMsgPool::GetMessage()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return GetReusableMessage();
}

void cMsgPool::Lock()
{
	m_ThreadSync.Lock();
	m_bLocked = true;
}

void cMsgPool::Unlock()
{
	m_ThreadSync.Unlock();
	m_bLocked = false;
}

cIMsg* cMsgPool::GetHead()
{
	cIMsg *pIMsg = NULL;
	try
	{
		if (!m_bLocked) return 0;
		m_it = m_Messages.begin();
		if (m_it == m_Messages.end())
			return 0;
		pIMsg = *m_it;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return pIMsg;
}

cIMsg* cMsgPool::GetNext()
{
	cIMsg *pIMsg = NULL;
	try
	{
		if (!m_bLocked) return 0;
		if (m_it == m_Messages.end())	// CJNEW 20091127
			return 0;
		m_it++;
		if (m_it == m_Messages.end())
			return 0;
		pIMsg = *m_it;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return pIMsg;
}

void cMsgPool::DeleteCurrent()
{
	m_it = m_Messages.erase(m_it);
	if (m_it == m_Messages.end())	// CJNEW 20091127
		return;
	m_it++;
}

void cMsgPool::FlushFreeStack()
{
	while (!m_FreeMsgStack.empty())
		m_FreeMsgStack.pop();
}

int cMsgPool::Size()
{
	return m_FreeMsgStack.size();
}
