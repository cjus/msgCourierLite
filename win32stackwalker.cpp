#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "win32stackwalker.h"

using namespace std;

cThreadSync cWin32StackWalker::m_WSThreadSync;
string cWin32StackWalker::m_sInfo;

void cWin32StackWalker::WalkStack(bool withLocalVariables)
{
	cAutoThreadSync ThreadSync(&m_WSThreadSync);

	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
    CONTEXT context;
	ZeroMemory(&context, sizeof(context));
	context.ContextFlags = CONTEXT_FULL;
	GetThreadContext(thread, &context);

	SymInitialize(process, 0 , true);

	STACKFRAME64 stackFrame;
	ZeroMemory(&stackFrame, sizeof(stackFrame));
	stackFrame.AddrPC.Offset = context.Eip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context.Ebp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;

	cLog::LogToString(m_sInfo, "\r\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\r\n");
	for (int frameNum = 0; StackWalk64(
		IMAGE_FILE_MACHINE_I386,
		process, 
		thread, 
		&stackFrame, 
		&context,
		0,
		SymFunctionTableAccess64,
		SymGetModuleBase64,
		NULL); frameNum++)
	{
		//if (frameNum > 0)
		//{
			DisplayFrame(frameNum, process, stackFrame);
			if (withLocalVariables)
				DisplayLocals(process, stackFrame);
		//}
	}
	cLog::LogToString(m_sInfo, "\r\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\r\n");

	SymCleanup(process);
}

void cWin32StackWalker::DisplayFrame(int frameNum, HANDLE &process, STACKFRAME64 &frame)
{
	const int maxNameLen = 512;
	SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + maxNameLen);
	ZeroMemory(symbol, sizeof(SYMBOL_INFO) + maxNameLen);
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol->MaxNameLen = maxNameLen;

	IMAGEHLP_LINE64 line;
	ZeroMemory(&line, sizeof(line));
	line.SizeOfStruct = sizeof(line);

	if (frame.AddrPC.Offset == 0)
	{
		cLog::LogToString(m_sInfo, "Error: EIP=0\r\n");
		free(symbol);
		return;
	}

	DWORD64 offset;
	if (SymFromAddr(process, frame.AddrPC.Offset, &offset, symbol ))
		cLog::LogToString(m_sInfo, "%d. %-20s\r\n", frameNum, symbol->Name);
	else
		cLog::LogToString(m_sInfo, "%d. (no symbol name)\r\n", frameNum);

	DWORD displacement;
	if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &displacement, &line))
		cLog::LogToString(m_sInfo, "    %s::%d\r\n", line.FileName, line.LineNumber);
	else
		cLog::LogToString(m_sInfo, "    (no file name)\r\n");

	free(symbol);
}

void cWin32StackWalker::DisplayLocals(HANDLE &process, STACKFRAME64 &frame)
{
	IMAGEHLP_STACK_FRAME sf;
	ZeroMemory(&sf, sizeof(sf));
	sf.BackingStoreOffset	= frame.AddrBStore.Offset;
	sf.FrameOffset			= frame.AddrFrame.Offset;
	sf.FuncTableEntry		= (ULONG64)frame.FuncTableEntry;
	sf.InstructionOffset	= frame.AddrPC.Offset;
	sf.Params[0]			= frame.Params[0];
	sf.Params[1]			= frame.Params[1];
	sf.Params[2]			= frame.Params[2];
	sf.Params[3]			= frame.Params[3];
	sf.ReturnOffset			= frame.AddrReturn.Offset;
	sf.StackOffset			= frame.AddrStack.Offset;
	sf.Virtual				= frame.Virtual;

	if (SymSetContext(process, &sf, 0))
	{
		if (!SymEnumSymbols(
			process,
			0, /* Base of Dll */
			"[a-zA-Z0-9_]*", /* mask */
			EnumSymbols,
			&frame.AddrStack.Offset /* User context */
			))
		{
			cLog::LogToString(m_sInfo, "      ==> (no symbols available)\r\n");
		}
	}
	else
	{
		cLog::LogToString(m_sInfo, "      ==> (no context information)\r\n");
	}
}

BOOL CALLBACK cWin32StackWalker::EnumSymbols(PSYMBOL_INFO symbol, ULONG symbolSize, PVOID userContext)
{
	DWORD64 addr = (DWORD64)userContext + symbol->Address - 8;
	DWORD	data = *((DWORD*)addr);

	cLog::LogToString(m_sInfo, "      ==> %c %-20s = %08x\r\n", 
		(symbol->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER) ? 'P' : ' ',
		symbol->Name,
		data);

	return true;
}

void cWin32StackWalker::CopyString(string &sInfo)
{
	sInfo += m_sInfo;
}

