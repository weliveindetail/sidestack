#include <DebugHelper.h>

#include <cstdarg>
#include <cstdio>

// -------------------------------------------------------------------------------------------------

namespace 
{
  Assert::FailBehavior defaultHandler(const char* condition, const char* msg,
                                      const char* file, const int line)
  {
	  std::printf("%s(%d): Assert Failure: ", file, line);
	
	  if (condition != NULL)
		  std::printf("'%s' ", condition);

	  if (msg != NULL)
		  std::printf("%s", msg);

	  std::printf("\n");
	  return Assert::Halt;
  }

  Assert::Handler &getAssertHandlerInstance()
  {
	  static Assert::Handler s_handler = &defaultHandler;
	  return s_handler;
  }
}

// -------------------------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4996)
  // vsnprintf function may be unsafe, but we don't consider using vsnprintf_s

Assert::FailBehavior Assert::ReportFailure(const char* condition, const char* file, 
                                           const int line, const char* msg, ...)
{
	const char* message = NULL;
	if (msg != nullptr)
	{
		char messageBuffer[1024];
		{
			va_list args;
			va_start(args, msg);
			vsnprintf(messageBuffer, 1024, msg, args);
			va_end(args);
		}

		message = messageBuffer;
	}

	return getAssertHandlerInstance()(condition, message, file, line);
}

#pragma warning(pop)

// -------------------------------------------------------------------------------------------------
