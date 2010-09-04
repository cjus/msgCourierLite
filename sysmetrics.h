/* sysmetrics.h
   Copyright (C) 2005 Carlos Justiniano

sysmetrics.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

sysmetrics.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with sysmetrics.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file sysmetrics.h 
 @brief cSysMetrics handles system metrics logging
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 cSysMetrics handles system metrics logging
*/

#ifndef _SYSMETRICS_H
#define _SYSMETRICS_H

#include <string>

#include "thread.h"
#include "threadsync.h"
#include "timer.h"

#define HRC_SYSMETRICS_OK                  0x0000

class cSysMetricNumericalCounter
{
public:
	cSysMetricNumericalCounter();
	void Inc() { count++; totalCount++; }
	void ResetCount() { count = 0; }

	void GetHourData(int *data);
	int  GetCount() { return count; }
	long GetTotal() { return totalCount; }
	void SetHourlyEntry(int sec, int value) { index = sec; hourlyCount[index] = value; }
protected:
	int index;
	int count;
	long totalCount;
	int hourlyCount[3600];
};

/**
 @class cSysMetrics 
 @brief cSysMetrics handles system metrics logging
*/
class cSysMetrics : public cThread
{
public:
    cSysMetrics();
    ~cSysMetrics();

	int Start();
	int Stop();

	void FullReset();

	void IncMsgDelivered();
	void IncMsgUndelivered();
	void IncMsgRejected();

	void IncConAccepted();
	void IncConDropped();
	void IncConRejected();

	long GetTotalMsgDelivered();
	long GetTotalMsgUndelivered();
	long GetTotalMsgRejected();

	long GetTotalConAccepted();
	long GetTotalConDropped();
	long GetTotalConRejected();

	int GetSystemStartTime() { return m_iSystemTimeStamp; }

	int GetUptimeDays() { return m_iUTDays; }
	int GetUptimeHours() { return m_iUTHours; }
	int GetUptimeMinutes() { return m_iUTMins; }
	int GetUptimeSeconds() { return m_iUTSecs; }

	void GetConnectionAcceptedData(int *pArr);
	void GetConnectionDroppedData(int *pArr);
	void GetConnectionRejectedData(int *pArr);

	void GetMessageDeliveredData(int *pArr);
	void GetMessageUndeliveredData(int *pArr);
	void GetMessageRejectedData(int *pArr);

	void UpdateMsgResponseTime(double elapsed);
	double GetAvgMsgResponseTime() { return (m_AvgMsgResponseTime / (double)m_msgCount); }
protected:
    int Run();

private:
	cThreadSync m_ThreadSync;
	cTimer m_SecondTimer;
	cTimer m_HourTimer;
	cTimer m_AvgMsgResponseTimer;
	
	std::string m_statusline;
	int m_SecondCount;

	cSysMetricNumericalCounter m_MsgDelivered;
	cSysMetricNumericalCounter m_MsgUndelivered;
	cSysMetricNumericalCounter m_MsgRejected;

	cSysMetricNumericalCounter m_ConAccepted;	// Accepted for processing
	cSysMetricNumericalCounter m_ConDropped;	// Dropped due to too many connections
	cSysMetricNumericalCounter m_ConRejected;	// Rejected due to IP block rule

	int m_iSystemTimeStamp;
	int m_iUTDays;
	int m_iUTHours;
	int m_iUTMins;
	int m_iUTSecs;

	int m_msgCount;
	int m_msgCountPerSecond;
	int m_msgMaxCountPerSecond;
	double m_AvgMsgResponseTime;
	
	void UpdateUpTime();
};

#endif //_SYSMETRICS_H

