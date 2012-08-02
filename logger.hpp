#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdio>
#include "msvcfix.hpp"

class Logger
{
public:
	Logger();
	~Logger();

	void log(const char *text, ...);
	void start(const char *fileName);
	void stop();

private:
	std::FILE *logFile;
	bool isReady;
};

extern Logger logger;

#endif	//LOGGER_HPP
