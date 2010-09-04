/* commandhandlerfactory.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

commandhandlerfactory.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

commandhandlerfactory.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with commandhandlerfactory.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file commandhandlerfactory.cpp
 @brief The Command handler factory is used to return cICommandHandler
 interfaces to internal and external handlers that implement command handling
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "exception.h"
#include "log.h"
#include "commandhandlerfactory.h"

#include <iostream>

using namespace std;

cCommandHandlerFactory::cCommandHandlerFactory()
{
}

cCommandHandlerFactory::~cCommandHandlerFactory()
{
	try
	{	
		cCommandHandlerDecorator *pDecorator;
		for (m_CommandMapIterator = m_CommandMap.begin();
			 m_CommandMapIterator != m_CommandMap.end();
			 m_CommandMapIterator++)
		{
			pDecorator = m_CommandMapIterator->second;
			delete pDecorator;
		}
		m_CommandMap.erase(m_CommandMap.begin(), m_CommandMap.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

cICommandHandler *cCommandHandlerFactory::GetCommandHandler(const char *pCommand)
{
	cICommandHandler *handler = NULL;	
	string sName = pCommand;
	string sCommandName;

	try
	{
		string::size_type idx;
		idx = sName.find('.');

		if (idx == string::npos)
			sCommandName = sName;
		else
			sCommandName = sName.substr(0, idx);

		m_CommandMapIterator = m_CommandMap.find(sCommandName.c_str());
		if (m_CommandMapIterator == m_CommandMap.end())
			return NULL;

		//Dynamic cast warning due to multiple inheritendence. I'll need to revist this!
		//for now the old cast method will have to do.
		//return dynamic_cast<cICommandHandler*>(dynamic_cast<cICommandHandler*>((*m_CommandMapIterator).second));
		handler = (cICommandHandler*)(*m_CommandMapIterator).second;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return handler;
}

void cCommandHandlerFactory::RegisterCommandHandler(const char *pCommand, cICommandHandler *pHandler, cISysInfo *pSysInfo)
{
	try
	{	
		// does a handler already exist for the command specified in pCommand?
		cICommandHandler *pCommandHandler = cCommandHandlerFactory::GetCommandHandler(pCommand);
		if (pCommandHandler != 0)
		{
			string sMsg;
			sMsg = "Error, a command handler already exists for command: ";
			sMsg += pCommand;
			LOGALL(sMsg.c_str());
			return;
		}

		// New command, decorate and register it
		LOG("Registering command: [%s]", pCommand);
		cCommandHandlerDecorator *pDecorator = 0;
		MC_NEW(pDecorator, cCommandHandlerDecorator(pHandler));
		m_CommandMap.insert(pair<string,cCommandHandlerDecorator*>(string(pCommand), pDecorator));
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}


