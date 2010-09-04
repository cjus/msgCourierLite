/* timer.h
   Copyright (C) 2002 Carlos Justiniano

timer.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

timer.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with timer.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file timer.h 
 @brief Polled timer object.  Useful for scheduling events. 
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)

 Polled timer object.  Useful for scheduling events.
*/

#ifndef _TIMER_H
#define _TIMER_H

/**
 @class cTimer
 @brief Polled timer object.  Useful for scheduling events. 
*/
class cTimer
{
public:
	cTimer();
	~cTimer();
	void SetInterval(int seconds);
	void Start();
	void Stop();
	bool IsReady();
	void Reset();
	int  GetInterval() { return m_seconds; }
private:
	bool m_bStopped;
	int  m_seconds;
	int  m_timestamp;
};

#endif //_TIMER_H
