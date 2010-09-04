/* commandhandlerfactory.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

commandhandlerfactory.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

commandhandlerfactory.h was developed by Carlos Justiniano for use on the
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
 @file commandhandlerfactory.h
 @brief The Command handler factory is used to return cICommandHandler
 interfaces to internal and external handlers that implement command handling
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef COMMAND_HANDLER_FACTORY_H
#define COMMAND_HANDLER_FACTORY_H

#include <string>
#include <map>
#include <iostream>

#include "commandhandlerdecorator.h"
#include "icommandhandler.h"
#include "isysinfo.h"

/**
 @class cCommandHandlerFactory
 @brief The Command handler factory is used to return cICommandHandler
 interfaces to internal and external handlers that implement command handling
*/
class cCommandHandlerFactory
{
public:
	cCommandHandlerFactory();
	~cCommandHandlerFactory();
	cICommandHandler *GetCommandHandler(const char *pCommand);
	void RegisterCommandHandler(const char *pCommand, cICommandHandler *pHandler, cISysInfo *pSysInfo);
private:
	static cCommandHandlerFactory *m_pInstance;
	std::map<std::string,cCommandHandlerDecorator*> m_CommandMap;
	std::map<std::string,cCommandHandlerDecorator*>::iterator m_CommandMapIterator;
};

#endif // COMMAND_HANDLER_FACTORY_H
