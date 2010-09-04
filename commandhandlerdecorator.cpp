/* commandhandlerdecorator.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

commandhandlerdecorator.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

commandhandlerdecorator.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with commandhandlerdecorator.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file commandhandlerdecorator.cpp
 @brief Decorates command handler objects with support for multithreading and queing
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
 Decorates command handler objects with support for multithreading and queing
*/

#include "log.h"
#include "commandhandlerdecorator.h"

using namespace std;

cCommandHandlerDecorator::cCommandHandlerDecorator(cICommandHandler *pICommandHandler)
{
	m_pICommandHandler = pICommandHandler;
}

char *cCommandHandlerDecorator::GetCommandName()
{
	return m_pICommandHandler->GetCommandName();
}

void cCommandHandlerDecorator::OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pILogger)
{
	m_pMsgQueue = pMsgQueue;
	m_pICommandHandler->OnStartup(m_pMsgQueue, pSysInfo, pILogger);
	SetThreadName(m_pICommandHandler->GetCommandName());
	Create();
	Resume();
}

void cCommandHandlerDecorator::OnShutdown()
{
	m_pICommandHandler->OnShutdown();
	Stop();
}

int cCommandHandlerDecorator::OnProcess(cIMsg *pMessage)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
    	m_jobQueue.push(pMessage);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}

	/*
	if (IsSuspended())
	{
		//LOG("OOO Resuming %s thread", GetCommandName());
		LOG("Resuming [%s] commandhandlerdecorator", m_pICommandHandler->GetCommandName());
		Resume();
	}
	*/
	return HRC_COMMAND_HANDLER_OK;
}

const char *cCommandHandlerDecorator::OnProcessInternal(const char *pMessage)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	const char *p = m_pICommandHandler->OnProcessInternal(pMessage);
	if (p==0)
		return "";
	return p;
}

int cCommandHandlerDecorator::Run()
{
	cIMsg *pMessage;
	//int rc;

    m_ChronTimer.SetInterval(1);
    m_ChronTimer.Start();

	m_timer.SetInterval(10);
	m_timer.Start();

	while (ThreadRunning())
	{
		m_ThreadSync.Lock();
		try
		{
			int size = m_jobQueue.size();
			if (size != 0)
			{
				pMessage = m_jobQueue.top();
				if (pMessage != 0)
				{
                    if (m_timer.IsReady())
                    {
                        if (size > 1)
                        {
                            LOG("\n[%s]\n", pMessage->ToString());
                            LOG("[%s] internal message queue size is now %d requests deep", m_pICommandHandler->GetCommandName(), size);
                        }
                        m_timer.Reset();
                    }

					m_pICommandHandler->OnProcess(pMessage);
				}
				m_jobQueue.pop();
			}
			//else
			//{
			//	LOG("Suspending [%s] commandhandlerdecorator", m_pICommandHandler->GetCommandName());
			//	Suspend();
			//}

            if (m_ChronTimer.IsReady())
            {
                m_pICommandHandler->OnChron();
                m_ChronTimer.Reset();
            }

		}
		catch (char* str)
		{
			throw str;
		}
		catch (const char * str)
		{
			throw str;
		}
		catch (...)
		{
			string s;
			s = "Exception in cCommandHandlerDecorator::Run() for handler: ";
			s += m_pICommandHandler->GetCommandName();
			throw s.c_str();
		}
		m_ThreadSync.Unlock();
		YieldSleep();
	}

    return 0;
}
