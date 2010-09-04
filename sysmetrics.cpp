/* sysmetrics.cpp
   Copyright (C) 2005 Carlos Justiniano

sysmetrics.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

sysmetrics.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with sysmetrics.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file sysmetrics.cpp
 @brief cSysMetrics handles system metrics logging
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 cSysMetrics handles system metrics logging
*/

#include <time.h>
#include <vector>
#include <sstream>

#include "log.h"
#include "sysmetrics.h"
#include "exception.h"

using namespace std;

cSysMetricNumericalCounter::cSysMetricNumericalCounter()
: index(0), count(0), totalCount(0)
{
	memset(hourlyCount, 0, sizeof(int)*3600);
}

void cSysMetricNumericalCounter::GetHourData(int *data)
{
	// treat hourlyCount array as a circular buffer
	// and extract data in reverse order
	int i, j=0;
	for (i = index; i > -1; i--)
		data[j++] = hourlyCount[i];
	for (i = 3599; i > index; i--)
		data[j++] = hourlyCount[i];
}

cSysMetrics::cSysMetrics()
{
	m_iUTDays = 0;
	m_iUTHours = 0;
	m_iUTMins = 0;
	m_iUTSecs = 0;
	m_AvgMsgResponseTime = 0.0;
	m_msgCount = 0;
	m_msgCountPerSecond = 0;
	m_msgMaxCountPerSecond = 0;
	m_iSystemTimeStamp = time(0);

	FullReset();
	SetThreadName("cSysMetrics");
}

cSysMetrics::~cSysMetrics()
{
}

int cSysMetrics::Run()
{
	m_SecondTimer.SetInterval(1);
	m_SecondTimer.Start();

	m_HourTimer.SetInterval(3600);
	m_HourTimer.Start();

	m_AvgMsgResponseTimer.SetInterval(60);
	m_AvgMsgResponseTimer.Start();

    while (ThreadRunning())
    {
		m_ThreadSync.Lock();

        if (m_SecondTimer.IsReady())
		{
			UpdateUpTime();

			m_SecondCount++;
			if (m_msgCountPerSecond > m_msgMaxCountPerSecond)
				m_msgMaxCountPerSecond = m_msgCountPerSecond;
			m_msgCountPerSecond = 0;

			m_MsgDelivered.ResetCount();
			m_MsgUndelivered.ResetCount();
			m_MsgRejected.ResetCount();

			m_ConAccepted.ResetCount();
			m_ConRejected.ResetCount();
			m_ConDropped.ResetCount();

			m_SecondTimer.Reset();
		}

        if (m_HourTimer.IsReady())
		{
			FullReset();
			m_HourTimer.Reset();
		}

/*
		if (m_AvgMsgResponseTimer.IsReady() && m_AvgMsgResponseTime > 0)
		{
			LOG("Msg processing: Avg: %.2f ms,  RPS: %d, RPM: %d",
				(GetAvgMsgResponseTime() * 1000.0), m_msgMaxCountPerSecond, m_msgMaxCountPerSecond * 60);
			m_AvgMsgResponseTimer.Reset();
		}
*/
		m_ThreadSync.Unlock();
		this->Sleep(250);
    }
    return HRC_SYSMETRICS_OK;
}

int cSysMetrics::Start()
{
	cThread::Create();
	cThread::Start();
	return HRC_SYSMETRICS_OK;
}

int cSysMetrics::Stop()
{
	cThread::Destroy();
	return HRC_SYSMETRICS_OK;
}

void cSysMetrics::FullReset()
{
	m_SecondCount = 0;

	m_MsgDelivered.ResetCount();
	m_MsgUndelivered.ResetCount();
	m_MsgRejected.ResetCount();

	m_ConAccepted.ResetCount();
	m_ConRejected.ResetCount();
	m_ConDropped.ResetCount();
}

void cSysMetrics::IncMsgDelivered()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_MsgDelivered.Inc();
	m_MsgDelivered.SetHourlyEntry(m_SecondCount, m_MsgDelivered.GetCount());
}

void cSysMetrics::IncMsgUndelivered()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_MsgUndelivered.Inc();
	m_MsgUndelivered.SetHourlyEntry(m_SecondCount, m_MsgUndelivered.GetCount());
}

void cSysMetrics::IncMsgRejected()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_MsgRejected.Inc();
	m_MsgRejected.SetHourlyEntry(m_SecondCount, m_MsgRejected.GetCount());
}

void cSysMetrics::IncConAccepted()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_ConAccepted.Inc();
	m_ConAccepted.SetHourlyEntry(m_SecondCount, m_ConAccepted.GetCount());
}

void cSysMetrics::IncConDropped()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_ConDropped.Inc();
	m_ConDropped.SetHourlyEntry(m_SecondCount, m_ConDropped.GetCount());
}

void cSysMetrics::IncConRejected()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_ConRejected.Inc();
	m_ConRejected.SetHourlyEntry(m_SecondCount, m_ConRejected.GetCount());
}

long cSysMetrics::GetTotalMsgDelivered()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return m_MsgDelivered.GetTotal();
}

long cSysMetrics::GetTotalMsgUndelivered()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return m_MsgUndelivered.GetTotal();
}

long cSysMetrics::GetTotalMsgRejected()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return m_MsgRejected.GetTotal();
}

long cSysMetrics::GetTotalConAccepted()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return m_ConAccepted.GetTotal();
}

long cSysMetrics::GetTotalConDropped()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return m_ConDropped.GetTotal();
}

long cSysMetrics::GetTotalConRejected()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	return m_ConRejected.GetTotal();
}

void cSysMetrics::UpdateUpTime()
{
	int oneminute = 60;
	int onehour = 3600;
	int oneday = 86400;
	int idays = 0;
	int ihours = 0;
	int iminutes = 0;
	int iseconds = 0;

	int isecs = time(0) - m_iSystemTimeStamp;

	// compute days
	if (isecs > oneday)
	{
		idays = isecs / oneday;
		isecs -= (idays * oneday);
	}

	// compute hours
	if (isecs > onehour)
	{
		ihours = isecs / onehour;
		isecs -= (ihours * onehour);
	}

	// compute minutes
	if (isecs > oneminute)
	{
		iminutes = isecs / oneminute;
		isecs -= (iminutes * oneminute);
	}

	m_iUTDays = idays;
	m_iUTHours = ihours;
	m_iUTMins = iminutes;
	m_iUTSecs = isecs;
}

void cSysMetrics::GetConnectionAcceptedData(int *pArr)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_ConAccepted.GetHourData(pArr);
}

void cSysMetrics::GetConnectionDroppedData(int *pArr)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_ConDropped.GetHourData(pArr);
}

void cSysMetrics::GetConnectionRejectedData(int *pArr)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_ConRejected.GetHourData(pArr);
}

void cSysMetrics::GetMessageDeliveredData(int *pArr)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_MsgDelivered.GetHourData(pArr);
}

void cSysMetrics::GetMessageUndeliveredData(int *pArr)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_MsgUndelivered.GetHourData(pArr);
}

void cSysMetrics::GetMessageRejectedData(int *pArr)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_MsgRejected.GetHourData(pArr);
}

void cSysMetrics::UpdateMsgResponseTime(double elapsed)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_msgCount++;
	m_msgCountPerSecond++;
	m_AvgMsgResponseTime += elapsed;
}

