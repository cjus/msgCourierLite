/* msgprioritycomp.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgprioritycomp.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgprioritycomp.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with msgprioritycomp.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file msgprioritycomp.h
 @brief STL Message Queue comparison function
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef _MSGP_RIORITY_COMP_H
#define _MSGP_RIORITY_COMP_H

#include <queue>
#include "imsg.h"

/**
 @class cMsgPriorityComp
 @brief STL Message Queue comparison function
*/
class cMsgPriorityComp : public std::binary_function<cIMsg*, cIMsg*, bool> 
{
public:
    bool operator()(const first_argument_type& left,
                    const second_argument_type& right) 
	{
        return left->GetMsgPriority() < right->GetMsgPriority();
    }
};


#endif //_MSGP_RIORITY_COMP_H
