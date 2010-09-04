/* msgpool.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgpool.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgpool.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with msgpool.h; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file msgpool.h
 @brief Message Pool
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef MSG_POOL_H
#define MSG_POOL_H

#include <list>
#include <stack>

#include "msg.h"
#include "imsg.h"
#include "msgprioritycomp.h"
#include "threadsync.h"

/**
 @class cMsgPool
 @brief Message Pool
*/
class cMsgPool
{
public:
    cMsgPool();
    ~cMsgPool();

	cIMsg* GetMessage();
	void Lock();
	void Unlock();
	cIMsg* GetHead();
	cIMsg* GetNext();
	void DeleteCurrent();

	void FlushFreeStack();
	void Compact(int iThreshold);
	void ReturnToPool(cMsg *pMsg);

	int Size();
private:
	bool m_bLocked;
	cThreadSync m_ThreadSync;
	std::list<cMsg*> m_Messages;
	std::list<cMsg*>::iterator m_it;
	std::stack<cMsg*> m_FreeMsgStack;

	cMsg* GetReusableMessage();
};

#endif // MSG_POOL_H
