#include "logger.hpp"
#include <cstdarg>
#include <cassert>
#include <cstdio>

Logger logger;

Logger::Logger()
{
	isReady = false;
	logFile = 0;
}

Logger::~Logger()
{
	stop();
}

void Logger::log(const char *text, ...)
{
	assert(isReady);

	va_list varArgs;
	va_start(varArgs, text);

	static char buf[1024];
	vsnprintf(buf, 1024, text, varArgs);
	va_end(varArgs);
	
	fprintf(logFile, buf);
}

void Logger::start(const char *fileName)
{
	if (logFile != 0)
	{
		fclose(logFile);
		logFile = 0;
	}

	logFile = fopen(fileName, "wt+");
	if (logFile == 0)
		assert(0);

	isReady = true;
}

void Logger::stop()
{
	if (logFile != 0)
		fclose(logFile);

	logFile = 0;
	isReady = false;
}