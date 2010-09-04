#include "eventlogger.h"

cEventLogger::cEventLogger()
{
}

cEventLogger::~cEventLogger()
{
}

int cEventLogger::Create(int argc, char **argv)
{
	int iRet = HRC_LOG_SUCCESS;
	if (!m_pInstance)
	{
		m_pInstance = new cEventLogger();
		if (!m_pInstance)
			return HRC_LOG_CREATE_FAILURE;

		m_pInstance->ResetTimeBase();

		if (argc > 1)
		{
			if (strstr(argv[1], "c") != 0)
			{
				m_pInstance->SetOptConsoleLogging();
			}
			if (strstr(argv[1], "d") != 0)
			{
				m_pInstance->SetOptDisplayLogging();
			}
			if (strstr(argv[1], "f") != 0)
			{
				m_pInstance->SetOptFileLogging();
			}
		}
	}
	return iRet;
}

int cEventLogger::Destroy()
{
	delete m_pInstance;
	m_pInstance = 0;
	return HRC_LOG_SUCCESS;
}

cEventLogger* cEventLogger::GetInstance()
{
	return m_pInstance;
}

void cEventLogger::Log(const char *pFormat, ...)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	va_list ap;
	va_start(ap, pFormat);
	vsprintf(m_buf, pFormat, ap);
	va_end(ap);

	time_t t = time(0);
	tm *t2 = localtime(&t);

	if (m_DisplayLogging)
	{
#ifndef _PLATFORM_WIN32
		Logprintf("%.2d%.2d%.2d-%.2d%.2d%.2d: %s\n",  t2->tm_year-100, t2->tm_mon+1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec , m_buf);
#endif //!_PLATFORM_WIN32
#ifdef _PLATFORM_WIN32
		Logprintf("%.2d%.2d%.2d-%.2d%.2d%.2d: %s\r\n",  t2->tm_year-100, t2->tm_mon+1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec , m_buf);
#endif //_PLATFORM_WIN32

	}
	if (m_ConsoleLogging)
	{
		FILE *fpcon = 0;
		fpcon = fopen(CONSOLE_OUTPUT,"w");
		if (fpcon)
		{
			fprintf(fpcon, "%.2d%.2d%.2d-%.2d%.2d%.2d: %s\n",  t2->tm_year-100, t2->tm_mon+1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec , m_buf);
			fclose(fpcon);
		}
	}
	if (m_FileLogging)
	{
		FILE *fp = 0;
		fp = fopen(LOGFILE,"a");
		if (fp)
		{
			fprintf(fp, "%.2d%.2d%.2d-%.2d%.2d%.2d: %s\n",  t2->tm_year-100, t2->tm_mon+1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec , m_buf);
			fclose(fp);
		}
	}
}

void cEventLogger::DebugDump(char *szText, char *pPtr, int nBytes)
{
	if (pPtr==0 || nBytes < 1)
		return;

	cAutoThreadSync ThreadSync(&m_ddThreadSync);
	
	int offset=0, i=0, oldi, j;
	int iloop;
	FILE *fp = 0;
	FILE *fpcon = 0;

	
	if (m_DisplayLogging)
	{
		printf("[%s]\n", szText);
		printf("[%d bytes dumped]\n", nBytes);
	}
	if (m_ConsoleLogging)
	{
		fpcon = fopen(CONSOLE_OUTPUT,"w");
		fprintf(fpcon,"[%s]\n", szText);
		fprintf(fpcon,"[%d bytes dumped]\n", nBytes);
	}
	if (m_FileLogging)
	{
		fp = fopen(LOGFILE,"a");
		fprintf(fp,"[%s]\n", szText);
		fprintf(fp,"[%d bytes dumped]\n", nBytes);
	}

	while (i<=nBytes)
	{
		// print hex values
		oldi=i;
		iloop=0;

		if (m_DisplayLogging)
			printf("%.6x  ",offset);
		if (m_ConsoleLogging)
			fprintf(fpcon,"%.6x  ",offset);
		if (m_FileLogging)
			fprintf(fp,"%.6x  ",offset);
		while ((i<=nBytes) && (i-oldi<16))
		{
			if (m_DisplayLogging)
				printf("%.2x ",(unsigned char)*(pPtr));
			if (m_ConsoleLogging)
				fprintf(fpcon,"%.2x ",(unsigned char)*(pPtr)); 
			if (m_FileLogging)
				fprintf(fp, "%.2x ",(unsigned char)*(pPtr));
			++pPtr;
			i++;
			iloop++;
		}

		if (iloop < 16)
		{
			for (j=0; j<16-iloop; j++)
			{
				if (m_DisplayLogging)
					printf("   ");
				if (m_ConsoleLogging)
					fprintf(fpcon,"   ");
				if (m_FileLogging)
					fprintf(fp,"   ");
			}
		}

		// print char values
		pPtr-=(i-oldi);

		if (m_DisplayLogging)
			printf(" [");
		if (m_ConsoleLogging)
			fprintf(fpcon," [");
		if (m_FileLogging)
			fprintf(fp," [");

		for (j=1;j<=(i-oldi);j++)
		{
			if (!isprint(*pPtr))
			{
				if (m_DisplayLogging)
					printf("%c",(char)'.');
				if (m_ConsoleLogging)
					fprintf(fpcon,"%c",(char)'.');
				if (m_FileLogging)
					fprintf(fp,"%c",(char)'.');
			}
			else
			{
				if (*pPtr < 0x09 || *pPtr == 0x09 || *pPtr == 0x0a || *pPtr == 0x0d)
				{
					if (m_DisplayLogging)
						printf("%c",(char)'.');
					if (m_ConsoleLogging)
						fprintf(fpcon,"%c",(char)'.');
					if (m_FileLogging)
						fprintf(fp,"%c",(char)'.');
				}
				else
				{
					if (m_DisplayLogging)
						printf("%c",(char)*pPtr);
					if (m_ConsoleLogging)
						fprintf(fpcon,"%c",(char)*pPtr);
					if (m_FileLogging)
						fprintf(fp,"%c",(char)*pPtr);
				}
			}
			pPtr++;
		}
		if (m_DisplayLogging)
			printf("]\n");
		if (m_ConsoleLogging)
			fprintf(fpcon,"]\n");
		if (m_FileLogging)
			fprintf(fp,"]\n");
		offset+=16;
	}
	
	if (m_ConsoleLogging)
	{
		fclose(fpcon);
	}
	if (m_FileLogging)
	{
		fclose(fp);
	}
}

void cEventLogger::GetLogBuffer(string &s)
{
	cAutoThreadSync ThreadSync(&m_ddThreadSync);

	vector<string>::iterator MessagesIterator;	
	for (MessagesIterator = m_MessageLog.begin();
		 MessagesIterator != m_MessageLog.end(); 
		 MessagesIterator++)
	{
		s += *MessagesIterator;
	}
}

