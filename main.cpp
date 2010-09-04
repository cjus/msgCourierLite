/* main.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

main.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

main.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with main.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file main.cpp
 @brief MsgCourier Main
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence
 @note This is where a number of core objects are instantiated
*/

#include <iostream>
#include <fstream>
#include <botan/botan.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include "exception.h"
#include "doxygen-include.h"
#include "master.h"
#include "core.h"
#include <string>

using namespace std;
using namespace Botan;

int	main(int argc, char*argv[]);
void WritePID(const char *pModuleName);
void DoShutdown();

#ifdef _PLATFORM_LINUX
	void SignalCatcher(int signum);
#endif //_PLATFORM_LINUX

void ForcedExit(char *pMsg);
bool bNotShutdown=true;
bool bBeginTerminate=false;

int main(int argc, char *argv[])
{
	// change directory to path specified by commandline argument
	if (argc > 1)
	{
		string sCommandLine = (const char*)argv[1];
		string::size_type idx;
		idx = sCommandLine.rfind('"');
		if (idx != string::npos)
			sCommandLine.erase(idx, 1);
		idx = sCommandLine.find('"');
		if (idx != string::npos)
			sCommandLine.erase(idx, 1);
		chdir(sCommandLine.c_str());
	}

#ifdef _PLATFORM_LINUX
    //
	// tweak process stack size
    //
	struct rlimit lim;
	lim.rlim_cur = 1 * 256 * 1024;
	lim.rlim_max = 1 * 256 * 1024;
	setrlimit(RLIMIT_STACK, &lim);
#endif //_PLATFORM_LINUX

	// init Botan cryptography library
	LibraryInitializer init;

#ifdef _PLATFORM_LINUX
    //
    // setup signal handler
    //
	signal(SIGPIPE, SignalCatcher);

    //signal(SIGINT, SignalCatcher);
    signal(SIGTERM, SignalCatcher);
    signal(SIGHUP, SignalCatcher);
	signal(SIGSEGV, SignalCatcher);
	signal(SIGKILL, SignalCatcher);
	signal(SIGQUIT, SignalCatcher);
	signal(SIGSTOP, SignalCatcher);
#endif //_PLATFORM_LINUX

	try
	{
		//
		// Init Log object
		//
		char *pargv[3];
		pargv[0] = "msgCourier";
		if (argv[1] != 0)
			pargv[1] = argv[1];
		else
			pargv[1] = " ";
		pargv[2] = 0;
		int rc = cLog::Create(2, pargv);

		//
		// start system core
		//
		string path = argv[0];
		string moduleName;
		string::size_type idxStart = path.rfind("/");
		string::size_type idxEnd = path.rfind(".");
		if (idxStart != string::npos && idxEnd != string::npos)
		{
			moduleName = path.substr(idxStart+1, idxEnd - idxStart - 1);
		}
		cCore::Create(moduleName.c_str());

    	//
    	// write process ID
    	//
    	WritePID(moduleName.c_str());

		// sleep until app is shutdown
		while (bNotShutdown)
		{
			if (bBeginTerminate == true)
			{
				DoShutdown();
				exit(-1);
			}
			#ifdef _PLATFORM_LINUX
				//sched_yield();
				usleep(10);
			#endif //_PLATFORM_LINUX
			#ifdef _PLATFORM_WIN32
				::Sleep(0);	// Sleep zero under Win32 causes thread to relinquish its remaining timeslice
			#endif //_PLATFORM_WIN32
		}

		DoShutdown();
	}
	catch (exception &e)
	{
		cException ex;
		ex.Log();
	}
	catch (cException &ex)
	{
		ex.Log();
	}
	catch (...)
	{
		printf("msgCourier termindated due to unknown exception\n");
	}
	return 0;
}

void WritePID(const char* pModuleName)
{
	char mod[260];
	strncpy(mod, pModuleName, 255);
	strcat(mod,".pid");

	FILE *fp;
	fp = fopen(mod, "w");
	if (fp)
	{
		fprintf(fp, "%d\n", getpid());
		fclose(fp);
	}
}

#ifdef _PLATFORM_LINUX
void SignalCatcher(int signum)
{
	printf("signum: %d caught - exiting program\n", signum);
	cException ex;
	ex.Show_stackframe();
	abort();
}

void SignalCatcherOld(int signum)
{
	// Ignore this signal from now on
	signal (signum, SIG_IGN);
 printf("signum: %d\n", signum);
	if (signum == SIGPIPE)
    {
		// not requried under BSD and GNU/Linux but good practice so code potentially works properly under other operating systems
		signal(SIGPIPE, SignalCatcher);
		return;
    }
	// mark need for shutdown
	bBeginTerminate = true;

	cException ex;
	ex.Show_stackframe();

	// restore default signal behavior
	signal (signum, SIG_DFL);

	// Now resend the signal
	//raise (signum);
	abort();
	
}
#endif //_PLATFORM_LINUX

void ForcedExit(char *pMsg)
{
    LOG(pMsg);
    cLog::Destroy();
	exit(99);
}

void DoShutdown()
{
    LOG("Initiating shutdown sequence...");

//	cCore::Destroy();
//   LOG("Clean shutdown complete.\n");
//    cLog::Destroy();
	exit(99);
}

