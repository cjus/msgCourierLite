#ifndef CLOGGER_H
#define CLOGGER_H

#include "ilogger.h"

class cLogger : cILogger
{
public:
	cLogger();
	~cLogger();
	void Write(const char *pFormat, ...);
private:
	char m_CharBuffer[65000];
};

#endif // CLOGGER_H
