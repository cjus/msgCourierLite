/* filecache.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

filecache.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

filecache.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with filecache.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file filecache.cpp
 @brief File cache stores copies of files in memory for performance reasons
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include <iostream>
#include <fstream>

#include <time.h>
#include "exception.h"
#include "log.h"
#include "filecache.h"
#include "uniqueidprovider.h"

using namespace std;

cFileCache::cFileCache()
: m_CacheSize(4096000)
, m_CurrentCacheInUse(0)
{
}

cFileCache::~cFileCache()
{
	try
	{
		cFileCacheEntry* pEntry;
		map<string,cFileCacheEntry*>::iterator mapIterator;
		for (mapIterator = m_Map.begin(); mapIterator != m_Map.end(); mapIterator++)
		{
			pEntry = (*mapIterator).second;
			delete pEntry;
		}
		m_Map.erase(m_Map.begin(), m_Map.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

const cFileCacheEntry* cFileCache::GetFileEntry(string &fileName)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	cFileCacheEntry *pEntry = NULL;
	try
	{
		m_MapIterator = m_Map.find(fileName);
		if (m_MapIterator == m_Map.end())
			return 0;

		pEntry = (*m_MapIterator).second;
		pEntry->accessCount++;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return pEntry;
}

void cFileCache::UpdateFileAccessTime(std::string &fileName)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
		m_MapIterator = m_Map.find(fileName);
		if (m_MapIterator != m_Map.end())
		{
			cFileCacheEntry *pEntry = (*m_MapIterator).second;
			pEntry->fileLastAccessedTimeStamp = time(0);
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

bool cFileCache::Store(std::string &fileName, cBuffer *pFileData, int fileTimeStamp)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	cFileCacheEntry *pEntry = 0;

	try
	{
		// check if new item is larger than there is room in the cache
		if (pFileData->GetBufferCount() > m_CacheSize)
			return false;

		// check if we have to remove least recently used items to make
		// room in the cache
		if (m_CurrentCacheInUse + pFileData->GetBufferCount() > m_CacheSize)
		{
			// TODO: make room for new item.
		}

		// if cached entry exists remove it because we're
		// about to add a replacement record
		m_MapIterator = m_Map.find(fileName);
		if (m_MapIterator != m_Map.end())
		{
			m_Map.erase(m_MapIterator);
		}

		MC_NEW(pEntry, cFileCacheEntry());

		pEntry->fileName = fileName;
		pEntry->fileData.ReplaceWith(pFileData->cstr(), pFileData->GetBufferCount());
		pEntry->fileTimeStamp = fileTimeStamp;
		pEntry->fileLastAccessedTimeStamp = time(0);
		pEntry->accessCount = 0;

		//cBuffer ent;
		//ent.Sprintf(1024, "%s%d", fileName.c_str(), fileTimeStamp);
		//ent.MakeString();
		//HashFunction* hash = get_hash("MD5");
		//SymmetricKey sykey = hash->process(ent.cstr());

		std::string uuid;
		cUniqueIDProvider::GetInstance()->GetID(uuid);
		pEntry->entityTag.append(uuid);

		//LOG("Caching [%s](%s)", fileName.c_str(), pEntry->entityTag.c_str());
		m_Map.insert(pair<string,cFileCacheEntry*>(fileName, pEntry));
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return true;
}

bool cFileCache::HasEntry(string &fileName)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	bool bRet;
	try
	{
		m_MapIterator = m_Map.find(fileName);
		bRet = (m_MapIterator == m_Map.end()) ? false : true;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return bRet;
}

const char *cFileCache::GetEntityTag(string &fileName)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
		m_MapIterator = m_Map.find(fileName);
		if (m_MapIterator != m_Map.end())
		{
			cFileCacheEntry *pEntry = (*m_MapIterator).second;
			return pEntry->entityTag.c_str();
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return 0;
}

void cFileCache::SetCacheMaxSize(int cacheSize)
{
	try
	{
		m_CacheSize = cacheSize;
		if (!m_Map.empty())
		{
			// TODO: build "least recently used list"
			// remove items in order of least recently used to reduce memory usage to the new size.
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}



