/* timer.cpp
   Copyright (C) 2002 Carlos Justiniano

timer.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

timer.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with timer.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file timer.cpp 
 @brief Polled timer object.  Useful for scheduling events. 
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)

 Polled timer object.  Useful for scheduling events.
*/
#include <time.h>
#include "timer.h"

cTimer::cTimer()
:m_bStopped(true)
,m_seconds(0)
,m_timestamp(0)
{
}

cTimer::~cTimer()
{
}

void cTimer::SetInterval(int seconds)
{
	m_seconds = seconds;
}

void cTimer::Start()
{
	Reset();
}

void cTimer::Stop()
{
	m_bStopped = true;
}

bool cTimer::IsReady()
{
	if (m_bStopped)
		return false;
	return (time(0) >= m_timestamp) ? true : false;
}

void cTimer::Reset()
{
	m_timestamp = time(0) + m_seconds;
	m_bStopped = false;
}

