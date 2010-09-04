#ifndef ILOGGER_H
#define ILOGGER_H

class cILogger
{
public:
	virtual void Write(const char *pFormat, ...) = 0;
};

#endif // ILOGGER_H
