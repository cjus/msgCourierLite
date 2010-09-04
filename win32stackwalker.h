#ifndef _WIN32_STACK_WALKER_H
#define _WIN32_STACK_WALKER_H

#include <windows.h>
#include <Dbghelp.h>

#include <string>
#include "log.h"
#include "threadsync.h"

#define HRC_WIN32STACKWALKER_SUCCESS 0x0000
#define HRC_WIN32STACKWALKER_FAILURE 0x0001

/**
 @class cWin32StackWalker 
 @brief Walks the debug stack and traces path
*/
class cWin32StackWalker
{
public:
	static void WalkStack(bool withLocalVariables);
	static void CopyString(std::string &sInfo);
private:
	static cThreadSync m_WSThreadSync;
	static std::string m_sInfo;
	static void DisplayFrame(int frameNum, HANDLE &process, STACKFRAME64 &frame);
	static void DisplayLocals(HANDLE &process, STACKFRAME64 &frame);

	static BOOL CALLBACK EnumSymbols(PSYMBOL_INFO symbol, ULONG symbolSize, PVOID userContext);
};

#endif //_WIN32_STACK_WALKER_H
