/* uniqueidprovider.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

   uniqueidprovider.h is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   uniqueidprovider.cpp was developed by Carlos Justiniano for use on
   the msgCourier project and the ChessBrain Project and is now distributed
   in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
   even the implied warranty of MERCHANTABILITY or FITNESS FOR A
   PARTICULAR PURPOSE.  See the GNU General Public License for more
   details.

   You should have received a copy of the GNU General Public License
   along with main.cpp; if not, write to the Free Software 
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file uniqueidprovider.h
 @brief Unique ID Provider
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
 Unique ID Provider
*/

#ifndef UNIQUE_ID_PROVIDER_H
#define UNIQUE_ID_PROVIDER_H

#define MAX_UNIQUE_IDS 256

#include <vector>
#include <string>
#include "thread.h"
#include "threadsync.h"

/**
 @class cUniqueIDProvider 
 @brief Unique ID Provider
*/
class cUniqueIDProvider: cThread
{
public:
	static void Create();
	static void Destroy();
	static cUniqueIDProvider* GetInstance() { return m_pInstance; }
        
	void GetID(std::string &sGUID);

protected:
	cUniqueIDProvider();
	~cUniqueIDProvider();

	int Run();
private:
	static cUniqueIDProvider* m_pInstance;
	int m_iIndex;
	cThreadSync m_ThreadSync;
	std::string m_sGUID;
	std::vector<std::string> m_vector;

	void CreateNewID(std::string &sGUID);
};

#endif // UNIQUE_ID_PROVIDER_H
