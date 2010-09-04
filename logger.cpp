#include "logger.h"
#include "log.h"
#include "threadsync.h"

cThreadSync cLogger_ThreadSync;

cLogger::cLogger()
{
}

cLogger::~cLogger()
{
}

void cLogger::Write(const char *pFormat, ...)
{
	cAutoThreadSync ThreadSync(&cLogger_ThreadSync);
	va_list ap;
	va_start(ap, pFormat);
	vsprintf(m_CharBuffer, pFormat, ap);
	va_end(ap);
	LOG(m_CharBuffer);
}
