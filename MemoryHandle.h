/*----------------------------------------------------
MemoryHandle.h

Memory handle class.
----------------------------------------------------*/

#ifndef MemoryHandle_h
#define MemoryHandle_h

#include "ObjectAllocator.h"

namespace MemoryManager
{
  // Memory handle class. Stores and manages an ObjectAllocator pointer.
  class Handle
  {
    // Handle allocator.
    static ObjectAllocator<Handle> HandleAllocator;

    // Private constructor to prevent creating own handles
    Handle(void * memory, void * allocator);
    Handle(Handle const & rhs) {}

    // Null handle constructor
    Handle();

    // Memory managed by this handle instance.
    void *	memory;

    // Allocator that owns the memory.
    void *	allocator;

    // Reference count for the handle.
    int		  refCount;

  public:
    // Handle representing the null instance. This is not managed by the allocator.
    static Handle Null;

#ifdef MEMORYMANAGER_DEBUG
    /*
      Allocates and initializes a handle
      allocator - the allocator that owns the memory
      memory    - the memory pointer managed by this handle
      file      - the file where the allocation occurred
      line      - the line where the allocation occurerd
    */
    static Handle & CreateHandle(void * allocator, void * memory, char const * file, unsigned line);
    
    // Gets the number of handles currently allocated. Used for testing.
    static int GetNumberOfAllocatedHandles();

    /*
      Remove a reference from the handle
      file      - the file where the reference was removed.
      line      - the line where the reference was removed.
    */
    void RemoveRef(char const * filename = nullptr, unsigned line = 0);
#else
    static Handle & CreateHandle(void * allocator, void * memory);
    void RemoveRef();
#endif

    // Add reference to the handle
    inline void Handle::AddRef()
    {
      ++refCount;
    }

    // Gets the value stored by the handle.
    template <typename T>
    inline T * Get() const
    {
#ifdef MEMORYMANAGER_DEBUG
      if (memory == nullptr)
      {
        // Dangling pointer access. Log error.
        DebugHeader const * dbg = Handle::HandleAllocator.GetDebugHeader(this);
        Handle::HandleAllocator.GetLogStream() << "[Handle]: Attempt to access freed memory. Memory allocated at " << dbg->filename << " #" << dbg->line;
#ifdef MEMORYMANAGER_ENABLE_EXCEPTIONS
        throw MemoryManagerException("Attempt to access freed memory.", dbg->filename, dbg->line);
#endif
      }
#endif
      return static_cast<T *>(memory);
    }

#ifdef MEMORYMANAGER_DEBUG
    /*
      Free the memory associated with this handle.
      file - the file where the memory was freed.
      line - the line where the memory was freed.
    */
    template <typename T>
    inline void Free(const char * file, unsigned line)
    {
      if (memory == nullptr)
      {
        //Dangling pointer free
        DebugHeader const * dbg = Handle::HandleAllocator.GetDebugHeader(this);
        Handle::HandleAllocator.GetLogStream() 
          << "[Handle]: Attempt to free freed memory. Free attempt at: " 
          << file << " #" << line
          << "Memory allocated at: "
          << dbg->filename << " #" << dbg->line;
#ifdef MEMORYMANAGER_ENABLE_EXCEPTIONS
        throw MemoryManagerException("Attempt to free freed memory.", file, line);
#endif
      }
      else
      {
        unsigned char errorCode = static_cast<ObjectAllocator<T>*>(allocator)->Free(memory, file, line);
        if (errorCode != 0)
        {
          DebugHeader const * dbg = Handle::HandleAllocator.GetDebugHeader(this);
          Handle::HandleAllocator.GetLogStream()
            << "[Handle]: Invalid free attempt failed at: "
            << file << " #" << line
            << "Memory allocated at: "
            << dbg->filename << " #" << dbg->line;
#ifdef MEMORYMANAGER_ENABLE_EXCEPTIONS
          throw MemoryManagerException("Invalid free attempt.", file, line);
#endif
        }

        memory = nullptr;
      }
    }
#else
    //Frees held data and sets memory to NULL
    template <typename T>
    inline void Free()
    {
      if (memory != nullptr)
      {
        static_cast<ObjectAllocator<T>*>(allocator)->Free(memory);
        memory = nullptr;
      }
    }
#endif

#ifdef MEMORYMANAGER_DEBUG
    // Gets the current reference count for the handle.
    inline int GetRefCount() const
    {
      return refCount;
    }

    // Gets the allocator for the memory associated with the handle.
    inline void const * GetAllocator() const
    {
      return allocator;
    }
#endif
    // Gets the raw pointer managed by the handle.
    inline void * GetRawPointer() const
    {
      return memory;
    }

    // Checks whether the current handle is null.
    inline bool IsNull() const
    {
      return memory == nullptr;
    }
  };

  // Static instance of ObjectAllocator for all handles.
  static ObjectAllocator<Handle> handleAllocator;
}
#endif // MemoryHandle_h
