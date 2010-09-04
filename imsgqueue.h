/* imsgqueue.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

imsgqueue.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

imsgqueue.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with main.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file imsgqueue.h
 @brief Message Queue Interface
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)

 cIMsgQueue is an interface to the cMsgQueue object.
*/

#ifndef IMSG_QUEUE_H
#define IMSG_QUEUE_H

class cIMsg;
class cLogger;

/**
 @class cIMsgQueue 
 @brief cIMsgQueue is an interface to the cMsgQueue object
*/
class cIMsgQueue
{
public:
	virtual cIMsg* CreateMessage() = 0;
	virtual cIMsg* CreateMessageReply(cIMsg* pToThisMsg) = 0;
	virtual cIMsg* CreateInternalMessage() = 0;
};

#endif // IMSG_QUEUE_H

