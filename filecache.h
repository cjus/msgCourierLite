/* filecache.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

filecache.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

filecache.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with virtualdirectorymap.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file filecache.h
 @brief File cache stores copies of files in memory for performance reasons
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef FILECACHE_H
#define FILECACHE_H

#include <string>
#include <map>
#include "buffer.h"
#include "threadsync.h"

/**
 @class cFileCacheEntry
 @brief File cache entry
*/
class cFileCacheEntry
{
public:
	std::string fileName;
	cBuffer fileData;
	int fileTimeStamp;
	int fileLastAccessedTimeStamp;
	long accessCount;
	std::string entityTag;
};

/**
 @class cFileCache
 @brief File cache stores copies of files in memory for performance reasons
*/
class cFileCache
{
public:
	cFileCache();
	~cFileCache();
	
	void SetCacheMaxSize(int cacheSize);
	const cFileCacheEntry* GetFileEntry(std::string &fileName);
	bool Store(std::string &fileName, cBuffer *pFileData, int fileTimeStamp);
	bool HasEntry(std::string &fileName);
	const char *GetEntityTag(std::string &fileName);
	void UpdateFileAccessTime(std::string &fileName);

private:
	cThreadSync m_ThreadSync;
	std::map<std::string,cFileCacheEntry*> m_Map;
	std::map<std::string,cFileCacheEntry*>::iterator m_MapIterator;
	int m_CacheSize;
	int m_CurrentCacheInUse;
};

#endif // FILECACHE_H

