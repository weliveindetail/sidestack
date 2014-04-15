#include <SideStack.h>
#include <DebugHelper.h>
#include <Windows.h>

#include <new>
#include <cassert>

// ----------------------------------------------------------------------------
//                                           Check platform specific data types

static_assert(sizeof(SIZE_T) == sizeof(size_t), 
              "Platform data type size mismatch: size_t");

static_assert(sizeof(DWORD) == sizeof(unsigned long), 
              "Platform data type size mismatch: DWORD");

static_assert(sizeof(BYTE) == sizeof(char), 
              "Platform data type size mismatch: char");

static_assert(sizeof(LPVOID) == sizeof(char *),
              "Platform data type size mismatch: pointer");

// ----------------------------------------------------------------------------
//                                                           Stack frame access

// copy values of stack frame registers to member variables
#define STORE_STACK_FRAME(bp_storage, sp_storage)                             \
  __asm { mov ecx, this }                                                     \
  __asm { mov [ecx]SideStack.bp_storage, ebp };                               \
  __asm { mov [ecx]SideStack.sp_storage, esp };

// exchange value between stack pointer and given member variable
#define XCHG_STACK_POINTER(sp_storage)                                        \
  __asm { mov ecx, this }                                                     \
  __asm { xchg [ecx]SideStack.sp_storage, esp };

// exchange both, stack pointer and base pointer with the respective variable
#define XCHG_FRAME_POINTERS(bp_storage, sp_storage)                           \
  __asm { mov ecx, this }                                                     \
  __asm { xchg [ecx]SideStack.bp_storage, ebp };                              \
  __asm { xchg [ecx]SideStack.sp_storage, esp };

// ----------------------------------------------------------------------------

enum class SideStack::CallingMode
{
  INIT_SIDESTACK,
  CONTINUE_ON_SIDESTACK,
  SWITCH_STACKS
};

// ----------------------------------------------------------------------------

SideStack::SideStack()
{
  if (!allocSideStack())
  {
    throw new std::bad_alloc();
  }
  
  XCHG_STACK_POINTER(m_stackTop);

  try
  {
    handle(CallingMode::INIT_SIDESTACK);
  }
  catch (...) //< doesn't work, we never arrive here
  {
    XCHG_FRAME_POINTERS(m_storedBP, m_storedSP);
    throw;
  }
  
  XCHG_STACK_POINTER(m_stackTop);
}

// ----------------------------------------------------------------------------

SideStack::~SideStack()
{
  deallocSideStack();
}

// ----------------------------------------------------------------------------

void SideStack::switchStacks()
{
  handle(CallingMode::SWITCH_STACKS);
}

// ----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4731)
  // frame pointer register 'ebp' modified by inline assembly code
  // that's by purpose

void SideStack::executeOnSideStack(std::function<void ()> func)
{
  m_continuationFunction = func;

  try
  {
    handle(CallingMode::CONTINUE_ON_SIDESTACK);
  }
  catch (...) //< doesn't work, we never arrive here
  {
    XCHG_FRAME_POINTERS(m_storedBP, m_storedSP);
    throw;
  }
}

// ----------------------------------------------------------------------------

void SideStack::handle(CallingMode mode)
{
  switch (mode)
  {
    case CallingMode::INIT_SIDESTACK:
    {
      STORE_STACK_FRAME(m_storedBP, m_storedSP);

      return;
    }

    case CallingMode::CONTINUE_ON_SIDESTACK:
    {
      XCHG_FRAME_POINTERS(m_storedBP, m_storedSP);

      m_continuationFunction();

      XCHG_FRAME_POINTERS(m_storedBP, m_storedSP);
      return;
    }

    case CallingMode::SWITCH_STACKS:
    {
      XCHG_FRAME_POINTERS(m_storedBP, m_storedSP);
      return;
    }
  }
}

// ----------------------------------------------------------------------------

/*void SideStack::tryCatchWrapper()
{
  try
  {
    m_continuationFunction();
  }
  catch (...) //< doesn't work, we never arrive here
  {
    XCHG_FRAME_POINTERS(m_storedBP, m_storedSP);
    throw;
  }
}*/

#pragma warning(pop)

// ----------------------------------------------------------------------------

bool SideStack::allocSideStack()
{
  size_t pageSize = getVirtualMemoryPageSize();
  m_totalSize = 3 * pageSize;
    
  unsigned long unusedOldMode;
  unsigned long allocType = MEM_COMMIT | MEM_RESERVE;
  
  m_baseAddress = reinterpret_cast<char *>(
    VirtualAlloc(nullptr, m_totalSize, allocType, PAGE_READWRITE));

  if (m_baseAddress == nullptr)
    return false;

  char *firstPage = m_baseAddress;
  if (!VirtualProtect(firstPage, pageSize, PAGE_NOACCESS, &unusedOldMode))
    return false;
    
  char *thirdPage = m_baseAddress + 2*pageSize;
  if (!VirtualProtect(thirdPage, pageSize, PAGE_NOACCESS, &unusedOldMode))
    return false;

  // note: stacks grow with decreasing addresses
  m_stackBottom = m_baseAddress + pageSize;
  m_stackTop = m_baseAddress + 2*pageSize;

  return true;
}

// ----------------------------------------------------------------------------

void SideStack::deallocSideStack()
{
  if (VirtualFree(m_baseAddress, m_totalSize, MEM_RELEASE) != 0)
  {
    M_FAIL("Failed to free SideStack allocated memory pages");
  }
}

// ----------------------------------------------------------------------------

size_t SideStack::getVirtualMemoryPageSize() const
{
  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);

  int pageSize = systemInfo.dwPageSize;
  M_ASSERT(pageSize > 0);

  return pageSize;
}

// ----------------------------------------------------------------------------
