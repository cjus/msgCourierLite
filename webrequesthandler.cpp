/* webrequesthandler.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

   webrequesthandler.cpp is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   webrequesthandler.cpp was developed by Carlos Justiniano for use on
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
 @file webrequesthandler.cpp
 @author Carlos Justiniano
 @brief Processes requests for the webserver component
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include <time.h>
#include "master.h"
#include "core.h"
#include "webrequesthandler.h"
#include "log.h"
#include "exception.h"
#include "virtualdirectorymap.h"
#include "sysmetrics.h"
#include "url.h"
#include "date.h"
#include "uniqueidprovider.h"
#include "headerextract.h"

#ifdef  _PLATFORM_LINUX
	#include <sys/stat.h>
	#include <unistd.h>
#endif //_PLATFORM_LINUX

using namespace std;

cWebRequestHandler::cWebRequestHandler()
: m_NewThread(1)
, m_bConnected(false)
{
}

cWebRequestHandler::~cWebRequestHandler()
{
}

int cWebRequestHandler::Start()
{
	if (m_NewThread)
	{
		if (cThread::Create() != HRC_THREAD_OK)
		{
			LOG("unable to create web request handler thread");
			return HRC_WEBREQUEST_HANDLER_HANDLER_CREATEFAILED;
		}
		m_NewThread = 0;
	}
	cThread::Start();
	return HRC_WEBREQUEST_HANDLER_HANDLER_OK;
}

int cWebRequestHandler::Stop()
{
	cThread::Destroy();
    return HRC_WEBREQUEST_HANDLER_HANDLER_OK;
}

bool cWebRequestHandler::IsConnected()
{
	return m_bConnected;
}

int cWebRequestHandler::Setup(cVirtualDirectoryMap *pVirtualDirectoryMap, cIMsgQueue *pMsgQueue, cIMsg *pMessage)
{
	m_bConnected = true;
	//LOG("Handler %p is processing socket %d", this, pMessage->GetTCPSocketHandle());
	m_pVirtualDirectoryMap = pVirtualDirectoryMap;
	m_pMsgQueue = pMsgQueue;
	m_pIMsg = pMessage;
	return HRC_WEBREQUEST_HANDLER_HANDLER_OK;
}

int cWebRequestHandler::Run()
{
	try
	{
		m_sServerLabel = "Server: msgCourier/";
    	m_sServerLabel += APP_VERSION;
	    //m_sServerLabel += " node(";
   		//m_sServerLabel += cCore::GetInstance()->GetServerName();
	    //m_sServerLabel += ")";
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}

	int ret;
	while (ThreadRunning())
	{
        ret = Process(m_pIMsg);

		// At this point this handler has completed
		// its task and so it suspends its thread.  The WebServer
		// manager will restart the thread when it has another
		// task for the this handler to process.
		Suspend();

		// This must be last!!!
		m_bConnected = false;
	}
	return HRC_WEBREQUEST_HANDLER_HANDLER_OK;
}

int cWebRequestHandler::Process(cIMsg *pMessage)
{
	try
	{
		m_processBench.Start();
		m_bWebPage = false;
		m_bCachedNotModified = false;
		m_sEntityTag = "";
		m_sMessageETag = "";

		// Process HTTP message
		ProcessHTTPHeader(pMessage);

		// post reply
		cIMsg *pMsgReply = m_pMsgQueue->CreateMessageReply(pMessage);
		if (pMsgReply)
		{
			stringstream header (stringstream::in | stringstream::out);

			string datestring;
			cDate::GetDate(datestring);
			string ver = APP_VERSION;

			GetResource();

			if (m_bCachedNotModified == true)
			{
				if (pMsgReply->IsKeepAlive()==false)
				{
					header	<< "HTTP/1.1 304 Not Modified\r\n"
							<< "Connection: close\r\n"
							<< "Date: " << datestring.c_str() << "\r\n"
							<< m_sServerLabel.c_str() << "\r\n"
							<< "\r\n";
				}
				else
				{
					header	<< "HTTP/1.1 304 Not Modified\r\n"
							<< "Connection: Keep-Alive\r\n"
							<< "Date: " << datestring.c_str() << "\r\n"
							<< m_sServerLabel.c_str() << "\r\n"
							<< "\r\n";
				}
				string sheader = header.str();
				m_Response.ReplaceWith((char*)sheader.c_str());
				pMsgReply->SetContentPayload((char*)m_Response.GetRawBuffer(), m_Response.GetBufferCount());
			}
			else if (m_Response.GetBufferCount() != 0)
			{
				if (m_sContentType.find("htm"))
				{
					ProcessServerSideScripts();
				}
				if (pMsgReply->IsKeepAlive()==false)
				{
					header	<< "HTTP/1.1 200 OK\r\n"
							<< "Connection: close\r\n"
							<< "Date: " << datestring.c_str() << "\r\n"
							<< m_sServerLabel.c_str() << "\r\n"
							<< "ETag: " << "\"" << m_sEntityTag.c_str() << "\"\r\n"
							<< "Content-Type: " << m_sContentType.c_str() << "\r\n"
							<< "Content-Length: " << m_Response.GetBufferCount() << "\r\n"
							<< "\r\n";
				}
				else
				{
					header	<< "HTTP/1.1 200 OK\r\n"
							<< "Connection: Keep-Alive\r\n"
							<< "Date: " << datestring.c_str() << "\r\n"
							<< m_sServerLabel.c_str() << "\r\n"
							<< "ETag: " << "\"" << m_sEntityTag.c_str() << "\"\r\n"
							<< "Content-Type: " << m_sContentType.c_str() << "\r\n"
							<< "Content-Length: " << m_Response.GetBufferCount() << "\r\n"
							<< "\r\n";
				}
				string sheader = header.str();
				m_Response.Prepend((char*)sheader.c_str());
				pMsgReply->SetContentPayload((char*)m_Response.GetRawBuffer(), m_Response.GetBufferCount());
			}
			else
			{
				string body;
				if (cCore::GetInstance()->IsZeroConfig())
				{
					body = "<html><head><title>Node: ";
					body += cCore::GetInstance()->GetServerName();
					body += "</title></head><body>";
					body += "<h2>Node: ";
					body += cCore::GetInstance()->GetServerName();
					body += "</h2>";
					body += "<p>Running based on zero-configuration. Note: this node will only accept connections on the same subnet.</p>";
					body += "<hr>";
					body += "<address>msgCourier/";
					body += APP_VERSION;
					body += "</address>";
					body += "</body></html>";

					header	<< "HTTP/1.1 200 OK\r\n"
							<< "Date: " << datestring.c_str() << "\r\n"
							<< "Server: msgCourier/" << APP_VERSION << " node(" << cCore::GetInstance()->GetServerName() << ")\r\n"
							<< "ETag: " << "\"" << m_sEntityTag << "\"\r\n"
							<< "Content-Type: text/html\r\n"
							<< "Content-Length: " << body.length() << "\r\n"
							<< "\r\n"
							<< body.c_str();
				}
				else
				{
					body = "<html><head><title>404 Not Found</title></head><body>";
					body += "<h1>Resource Not Found</h1>";
					body += "</title></head><body>";
					body += "<p>The requested resource was not found on this server</p>";
					body += "</h2>";
					body += "<hr>";
					body += "<address>msgCourier/";
					body += APP_VERSION;
					body += "</address>";
					body += "</body></html>";

					if (pMsgReply->IsKeepAlive()==false)
					{
						header	<< "HTTP/1.1 404 Not Found\r\n"
								<< "Connection: close\r\n"
								<< "Date: " << datestring.c_str() << "\r\n"
								<< "Server: msgCourier/" << APP_VERSION << " node(" << cCore::GetInstance()->GetServerName() << ")\r\n"
								<< "Content-Type: text/html\r\n"
								<< "Content-Length: " << body.length() << "\r\n"
								<< "\r\n"
								<< body.c_str();
					}
					else
					{
						header	<< "HTTP/1.1 404 Not Found\r\n"
								<< "Connection: Keep-Alive\r\n"
								<< "Date: " << datestring.c_str() << "\r\n"
								<< "Server: msgCourier/" << APP_VERSION << " node(" << cCore::GetInstance()->GetServerName() << ")\r\n"
								<< "Content-Type: text/html\r\n"
								<< "Content-Length: " << body.length() << "\r\n"
								<< "\r\n"
								<< body.c_str();
					}
				}
				string sheader = header.str();
				pMsgReply->SetContentPayload((char*)sheader.c_str(), sheader.length());
			}

			MARK_TRAVEL(pMsgReply);
			pMsgReply->DispatchMsg();
		}

		//LOG("Handler %p is *done* processing socket %d", this, pMessage->GetTCPSocketHandle());
		pMessage->MarkProcessed();

		m_CGI.erase(m_CGI.begin(), m_CGI.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return 0;
}

bool cWebRequestHandler::ProcessHTTPHeader(cIMsg *pIMsg)
{
	const char *pHTTPMessage = pIMsg->GetRawBuffer();
	int iSize = pIMsg->GetRawBufferSize();
//	bool bRet = false;

	try
	{
		string sPortAddr;

		m_CGIParams = "";

		// get message type
		m_bMessageTypeGet = (strncmp(pHTTPMessage, "GET", 3) == 0) ? true : false;
		int iMessageTypeOffset = (m_bMessageTypeGet) ? 3 : 4;
		iMessageTypeOffset++; // plus one for space character

		// finder command line end
		int iHeaderEnd = strstr(pHTTPMessage, " HTTP/") - pHTTPMessage;

		// get URI
		string srcURI;
		srcURI.assign(pHTTPMessage, iMessageTypeOffset, iHeaderEnd-iMessageTypeOffset);

		// url decode URI
		cURL urldec;
		string decURI;
		urldec.Decode(srcURI, decURI);

		// parse decoded URI
		m_URI.erase(m_URI.begin(), m_URI.end());
		if (strcmp(decURI.c_str(), "/") == 0)
		{
			m_URI.push_back("index.html");
		}
		else
		{
			cBuffer b;
			b.Append((char*)decURI.c_str());
			if (b.FindChar('/') == 0)
				b.Remove(0,1);
			m_URI.push_back(b.c_str());
		}

		// process HTTP POST message
		cHeaderExtract he;
		if (m_bMessageTypeGet == false)
		{
			cBuffer buf;
			string sContentType;
			string sContentLength;
			int length = 0;
			buf.ReplaceWith((char*)pHTTPMessage, iSize);
			he.Parse(&buf);
			string sETag;
			he.GetValue("If-None-Match", sETag);
			if (sETag.length() != 0)
				m_sMessageETag.assign(sETag, 1, sETag.length()-2);

			he.GetValue("Content-Type", sContentType);
			he.GetValue("Content-Length", sContentLength);
			length = atoi(sContentLength.c_str());

			if (length > 0)
			{
				char *p = (char*)buf.GetBuffer();
				p += iSize - length;
				srcURI.assign((char*)p, length);
				urldec.Decode(srcURI, decURI);
				if (sContentType == "application/x-www-form-urlencoded")
				{
					m_CGIParams += decURI;
				}
			}
		}
		else
		{
			cBuffer buf;
			buf.ReplaceWith((char*)pHTTPMessage, iSize);
			he.Parse(&buf);
			string sETag;
			he.GetValue("If-None-Match", sETag);
			if (sETag.length() != 0)
				m_sMessageETag.assign(sETag, 1, sETag.length()-2);
		}

		// get message virtual directory
		m_sPath = "";

		// use host field first...
		string sHost;
		he.GetValue("Host", sHost);

		if (sHost.length() > 0)
		{
			string s;
			s = "host_";
			s += sHost;
			m_sPath = m_pVirtualDirectoryMap->GetVirtualDirectory(s);
		}

		if (m_sPath.length() < 1)
		{
			int iPort = pIMsg->GetArrivalPort();
			ostringstream oPort;
    		if (!(oPort << iPort))
				return false;
			string sPort;
		   	sPort = "port_";
			sPort += oPort.str();
			m_sPath = m_pVirtualDirectoryMap->GetVirtualDirectory(sPort);
			if (m_sPath.length()==0)
				return false;
		}

		int iURIElements = m_URI.size();
		for (int i=0; i < iURIElements; i++)
		{
#ifndef _PLATFORM_LINUX
			m_sPath += "\\";
#else
			m_sPath += "/";
#endif //! _PLATFORM_LINUX
			m_sPath += m_URI[i];
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return true;
}

void cWebRequestHandler::GetResource()
{
	try
	{
		m_Response.Reset();
		int iread;
		FILE *fp;
		int fileTimeStamp = 0;

		string::size_type htmIT = m_sPath.find(".htm");
		if (htmIT != string::npos)
			m_bWebPage = true;

		cFileCache *pFileCache = cCore::GetInstance()->GetFileCache();

		cFileCacheEntry* pFileEntry = 0;
		bool bFileInCache = pFileCache->HasEntry(m_sPath);
		if (bFileInCache)
		{
			pFileEntry = (cFileCacheEntry*)pFileCache->GetFileEntry(m_sPath);
			fileTimeStamp = pFileEntry->fileTimeStamp;
		}

		if (!pFileEntry || pFileEntry->fileLastAccessedTimeStamp < time(0))
		{
		#ifdef _PLATFORM_WIN32
			struct tm ctm;
			FILETIME ft;
			SYSTEMTIME st;
			OFSTRUCT of;

			HFILE hFile = OpenFile(m_sPath.c_str(), &of, OF_READ);
			if (hFile != -1)
			{
				GetFileTime((HANDLE)hFile, NULL,NULL,&ft);
				FileTimeToSystemTime(&ft, &st);
				ctm.tm_year = st.wYear - 1900;
				ctm.tm_mon = st.wMonth - 1;
				ctm.tm_mday = st.wDay;
				ctm.tm_hour = st.wHour;
				ctm.tm_min = st.wMinute;
				ctm.tm_sec = st.wSecond;
				ctm.tm_isdst = 0;
				ctm.tm_wday = 0;
				ctm.tm_yday = 0;
				fileTimeStamp = mktime(&ctm);
				CloseHandle((HANDLE)hFile);
			}
	#endif //_PLATFORM_WIN32
	#ifdef _PLATFORM_LINUX
			struct stat st;
			int fd = open(m_sPath.c_str(), O_RDONLY);
			if (fd != -1)
			{
				int ret = fstat(fd, &st);
				if (ret != -1)
					fileTimeStamp = st.st_mtime;
			}
			close(fd);
	#endif //_PLATFORM_LINUX
		}

		pFileCache->UpdateFileAccessTime(m_sPath);

		if (bFileInCache && pFileEntry->fileTimeStamp == fileTimeStamp)
		{
			m_sEntityTag = pFileEntry->entityTag;
			if (m_sMessageETag == m_sEntityTag)
			{
				// if the cached item is a web page and it has an embedded MC script
				// then we mark the cache as modified so that an HTTP 304 isn't sent
				// to the client.
				if (m_bWebPage && (pFileEntry->fileData).Find("<?mc") != -1)
				{
					m_bCachedNotModified = false;
				}
				else
				{
					m_bCachedNotModified = true;
					return;
				}
			}
			cBuffer *pBuffer = const_cast<cBuffer*>(&pFileEntry->fileData);
			m_Response.ReplaceWith((char*)pBuffer->GetRawBuffer(), pBuffer->GetBufferCount());
			GetResourceContentType();
			return;
		}

		// file not in cache...
		m_Response.Reset();
		if (m_sPath.length() == 0)
			return;

		fp = fopen(m_sPath.c_str(),"rb");
		if (fp)
		{
			while ((iread = fread(m_tempBuffer,1,16384,fp)))
			{
				m_Response.Append(m_tempBuffer, iread);
			}
			fclose(fp);

			// add to cache
			bool bStored = pFileCache->Store(m_sPath, &m_Response, fileTimeStamp);
			if (bStored)
				m_sEntityTag = pFileCache->GetEntityTag(m_sPath);
			GetResourceContentType();
		}
		else
		{
			//LOG("Unable to load [%s] from disk.", m_sPath.c_str());
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("exception in cWebRequestHandler::GetResource()");
	}
}

void cWebRequestHandler::GetResourceContentType()
{
	try
	{
		// get resource content type
		string::size_type idx;
		idx = m_sPath.rfind('.');
		if (idx != string::npos)
		{
			m_sContentType = m_sPath.substr(idx);
			m_MIMEMap.GetContentType(m_sContentType, m_sContentType);
		}
		else
		{
			m_sContentType = "application/octet-stream";
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cWebRequestHandler::ProcessServerSideScripts()
{
	int index = 0;

	try
	{
		// Post process special tags
		int offset = 0;
		while ((index = m_Response.Find("{{MC_", offset)) != -1)
		{
			if (m_Response.Find("{{MC_SERVERADDR}}", index) !=-1)
			{
				int len = strlen("{{MC_SERVERADDR}}");
				m_Response.Remove(index, len);
				m_Response.Insert(index, cCore::GetInstance()->GetServerPrimaryAddr());
				offset = index + len;
			}
			else if (m_Response.Find("{{MC_SERVERTIME}}", index) != -1)
			{
				int len = strlen("{{MC_SERVERTIME}}");
				m_Response.Remove(index, len);
				time_t t = time(0);
				tm *t2 = localtime(&t);
				char *pTime = asctime(t2);
				m_Response.Insert(index, pTime, strlen(pTime) - 1);
				offset = index + len;
			}
			else if (m_Response.Find("{{MC_PAGETIME}}", index) !=-1)
			{
				char buf[60];
				int len = strlen("{{MC_PAGETIME}}");
				m_Response.Remove(index, len);
				m_processBench.Stop();
				double e = m_processBench.Elapsed();
				sprintf(buf, "%.6lf seconds", e);
				m_Response.Insert(index, buf);
				offset = index + len;
			}
			else
			{
				offset += index;
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}


