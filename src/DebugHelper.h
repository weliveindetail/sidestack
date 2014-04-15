#pragma once

// -------------------------------------------------------------------------------------------------

#include <functional>

#include <cstdarg>
#include <cstdio>

// -------------------------------------------------------------------------------------------------

#define M_HALT() __debugbreak()
#define M_UNUSED(x) do { (void)sizeof(x); } while(0)

// -------------------------------------------------------------------------------------------------

#if DEBUG
  #define M_DEBUG_CALL(func) func
  #define M_DEBUG_DECL(type, name) type name
  #define M_DEBUG_ASSIGN(var, value) var = value
  #define M_IF_DEBUG(stmt) stmt
#else
  #define M_DEBUG_CALL(func)
  #define M_DEBUG_DECL(type, name)
  #define M_DEBUG_ASSIGN(var, value)
  #define M_IF_DEBUG(stmt)
#endif

// -------------------------------------------------------------------------------------------------

namespace Assert 
{
  enum FailBehavior
  {
    Halt,
    Continue,
  };

  using Handler = std::function<
    FailBehavior(const char* condition, 
                 const char* msg, 
                 const char* file, 
                 int line)
  >;

  FailBehavior ReportFailure(
    const char* condition, 
    const char* file, 
    int line, 
    const char* msg, 
    ...);
}

// -------------------------------------------------------------------------------------------------

#ifdef DEBUG
	#define M_ASSERT(cond) \
		do \
		{ \
			if (!(cond)) \
			{ \
				if (Assert::ReportFailure(                \
          #cond, __FILE__, __LINE__,              \
          nullptr) == Assert::Halt)               \
        {                                         \
					M_HALT();                               \
        }                                         \
			}                                           \
		} while(0)

	#define M_ASSERT_MSG(cond, msg, ...)            \
		do                                            \
		{                                             \
			if (!(cond))                                \
			{                                           \
				if (Assert::ReportFailure(                \
          #cond, __FILE__, __LINE__,              \
          (msg), __VA_ARGS__) == Assert::Halt)    \
        {                                         \
					M_HALT();                               \
        }                                         \
			}                                           \
		} while(0)

	#define M_FAIL(msg, ...)                        \
		do                                            \
		{                                             \
			if (Assert::ReportFailure(                  \
        nullptr, __FILE__, __LINE__,              \
        (msg), __VA_ARGS__) == Assert::Halt)      \
      {                                           \
        M_HALT();                                 \
      }                                           \
		} while(0)

#else
	#define M_ASSERT(condition) \
		do { M_UNUSED(condition); } while(0)

	#define M_ASSERT_MSG(condition, msg, ...) \
		do { M_UNUSED(condition); M_UNUSED(msg); } while(0)

	#define M_FAIL(msg, ...) \
		do { M_UNUSED(msg); } while(0)

#endif
