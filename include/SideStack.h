#pragma once

#include <functional>

class SideStack
{
public:
  SideStack();
  ~SideStack();

  void executeOnSideStack(std::function<void()> func);
  void switchStacks();

private:
  void *m_storedBP;
  void *m_storedSP;
  
  std::function<void()> m_continuationFunction;

  void *m_stackTop;
  void *m_stackBottom;
  char *m_baseAddress;
  size_t m_totalSize;

  bool allocSideStack();
  void deallocSideStack();
  size_t getVirtualMemoryPageSize() const;
  
  enum class CallingMode;
  void handle(CallingMode mode);
  //void tryCatchWrapper();
};
