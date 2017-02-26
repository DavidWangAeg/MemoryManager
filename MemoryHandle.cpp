#include <new>
#include "MemoryHandle.h"

#ifndef MEMORYHANDLE_ALLOCATOR_LOGFILE
#define MEMORYHANDLE_ALLOCATOR_LOGFILE "MemoryHandle_AllocatorLog.txt"
#endif

namespace MemoryManager
{
  // Handle allocator
#ifdef MEMORYMANAGER_DEBUG
  ObjectAllocator<Handle> Handle::HandleAllocator(MEMORYHANDLE_ALLOCATOR_LOGFILE);
#else
  ObjectAllocator<Handle> Handle::HandleAllocator;
#endif

  // Constructs a memory handle
#ifdef MEMORYMANAGER_DEBUG
  Handle & Handle::CreateHandle(void * allocator, void * memory, char const * file, unsigned line)
  {
    return *(new (Handle::HandleAllocator.Allocate(file, line)) Handle(allocator, memory));
  }

  int Handle::GetNumberOfAllocatedHandles()
  {
    return HandleAllocator.GetStats().blocksInUse;
  }

#else
  Handle & Handle::CreateHandle(void * allocator, void * memory)
  {
    return *MM_ALLOC(Handle::HandleAllocator, Handle(allocator, memory));
  }
#endif
  // Null memory handle
  Handle Handle::Null;

  // Standard constructor
  Handle::Handle(void * allocator, void * memory) :
    memory(memory),
    allocator(allocator),
    refCount(0)
  {
  }

  // Constructor for null memory handle.
  Handle::Handle() :
    memory(nullptr),
    allocator(nullptr),
    refCount(1)
  {
  }

  // Remove reference from handle
#ifdef MEMORYMANAGER_DEBUG
  void Handle::RemoveRef(char const * filename, unsigned line)
#else
  void Handle::RemoveRef()
#endif
  {
    --refCount;

#ifdef MEMORYMANAGER_DEBUG
    if (refCount < 0)
    {
      DebugHeader const * dbg = Handle::HandleAllocator.GetDebugHeader(this);
      Handle::HandleAllocator.GetLogStream()
        << "[Handle]: Negative RefCount detected from remove at: "
        << filename << " #" << line
        << "Memory allocated at: "
        << dbg->filename << " #" << dbg->line;
#ifdef MEMORYMANAGER_ENABLE_EXCEPTIONS
      throw MemoryManagerException("Negative RefCount detected.", filename, line);
#endif
    }
#endif

    //Delete handle when there are no remaining references to it
    if (refCount <= 0)
    {
      //Memory should be freed before all references are removed
#if defined(MEMORYMANAGER_ENABLE_EXCEPTIONS) && defined(MEMORYMANAGER_DEBUG)
      if (memory != nullptr)
      {
        throw MemoryManagerException("Dangling reference: All references removed before pointer freed.", filename, line);
      }
#endif
      MM_FREE(Handle::HandleAllocator, this);
    }
  }
}
