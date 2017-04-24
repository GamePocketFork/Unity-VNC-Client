#include "UnityLog.h"

#include "stdio.h"
#include <list>
#include <string>
#include <windows.h>

#include "UnityPlugin/PlatformBase.h"
#include "UnityPlugin/RenderAPI.h"

using namespace std;

list<string> logs;
list<string>::const_iterator logs_iterator;

// Global variable
CRITICAL_SECTION CriticalSection;

using namespace rfb::unity;

void UnityLog::Log(const char * format, ...)
{
	char logString[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(logString, 1024, format, args);

	OutputDebugString(logString);
	OutputDebugString("\n");


	// Request ownership of the critical section.
	EnterCriticalSection(&CriticalSection);

	logs.push_back(std::string(logString, logString + strlen(logString)));

	// Release ownership of the critical section.
	LeaveCriticalSection(&CriticalSection);
}

// --------------------------------------------------------------------------
// GetDebugLog, to access to the oldest log
extern "C" bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetDebugLog(void* log, int size)
{
	if (logs.size() > 0)
	{
		// Request ownership of the critical section.
		EnterCriticalSection(&CriticalSection);

		string res = logs.front();
		logs.pop_front();
		// Release ownership of the critical section.
		LeaveCriticalSection(&CriticalSection);
		memset(log, 0, size);
		memcpy_s(log, size, res.c_str(), res.length());
		return true;
	}
	return false;
}


// --------------------------------------------------------------------------
// UnitySetInterfaces

static UnityDebugLogger uLogger;
static SimpleLogger debuglogger;

void UnityLog::Init()
{
	uLogger.registerLogger();
	debuglogger.registerLogger();

	InitializeCriticalSection(&CriticalSection);
}

void UnityLog::Release()
{
	// Release resources used by the critical section object.
	DeleteCriticalSection(&CriticalSection);
}

void UnityLog::Clear()
{
	logs.clear();
}

UnityDebugLogger::UnityDebugLogger() : Logger("UnityDebugLogger")
{

}



void UnityDebugLogger::write(int level, const char *logname, const char *message)
{
	const char * profile;
	if (level == 0) profile = "Error";
	else if (level <= 10) profile = "Status";
	else if (level <= 30) profile = "Info";
	else profile = "Debug";

	UnityLog::Log("[%s](%s): %s\n", logname, profile, message);
}

SimpleLogger::SimpleLogger() : Logger("SimpleLogger")
{

}

void SimpleLogger::write(int level, const char *logname, const char *message)
{
	const char * profile;
	if (level == 0) profile = "Error";
	else if (level <= 10) profile = "Status";
	else if (level <= 30) profile = "Info";
	else profile = "Debug";

	char line[1024];

	sprintf(line, "[%s](%s): %s\n", logname, profile , message);

	OutputDebugString(line);
}

