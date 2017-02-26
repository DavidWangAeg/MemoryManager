/*----------------------------------------------------
Pointer.h

Custom pointer class for memory handles.
----------------------------------------------------*/

#ifndef Pointer_h
#define Pointer_h

#include "MemoryHandle.h"

namespace MemoryManager
{
  // Pointer class that manages Handles to allocated memory.
  template <typename T>
  class Pointer
  {
    template <typename U>
    friend class Pointer;

  public:
    // Default constructor. Initializes Pointer to null.
    Pointer() :
      handle(&Handle::Null)
    {
      handle->AddRef();
    }

    // Constructs a Pointer from a handle.
    Pointer(Handle & handle) :
      handle(&handle)
    {
      this->handle->AddRef();
    }

    // Constructs a null pointer.
    Pointer(std::nullptr_t p) :
      handle(&Handle::Null)
    {
      handle->AddRef();
    }

    // Copy constructor.
    Pointer(Pointer const & rhs) :
      handle(rhs.handle)
    {
      handle->AddRef();
    }

    // Movement constructor.
    Pointer(Pointer && rhs) :
      handle(rhs.handle)
    {
      handle->AddRef();
    }

    // Copy constructor for a different pointer type.
    template <typename U>
    Pointer(Pointer<U> & rhs) :
      handle(rhs.handle)
    {
      T * p = (U*)nullptr;
      handle->AddRef();
    }

    // Destructor
    ~Pointer()
    {
      handle->RemoveRef();
    }

    // Assignment operator.
    Pointer & operator=(Pointer const & rhs)
    {
      if (this != &rhs)
      {
        handle->RemoveRef();
        handle = rhs.handle;
        handle->AddRef();
      }
      return *this;
    }

    // Move assignment operator.
    Pointer & operator=(Pointer && rhs)
    {
      if (this != &rhs)
      {
        handle->RemoveRef();
        handle = rhs.handle;
        handle->AddRef();
      }
      return *this;
    }

    // Assignment operator for null.
    Pointer & operator=(std::nullptr_t const & rhs)
    {
      handle->RemoveRef();
      handle = &Handle::Null;
      handle->AddRef();
      return *this;
    }

    // Assignment operator for a different pointer type.
    template <typename U>
    Pointer<T> & operator=(Pointer<U> const & rhs)
    {
      T * p = (U*)nullptr;
      handle->RemoveRef();
      handle = rhs.handle;
      handle->AddRef();

      return *this;
    }

    // Conversion operator to bool. Returns true if the handle is not null.
    inline operator bool() const
    {
      return !handle->IsNull();
    }

    // Equality operator. Returns true if pointers refer to the same handle.
    inline bool operator==(Pointer const & rhs) const
    {
      return handle == rhs.handle;
    }

    // Inequality operator.
    inline bool operator!=(Pointer const & rhs) const
    {
      return handle != rhs.handle;
    }

    // Equality operator for null.
    inline bool operator==(std::nullptr_t const & rhs) const
    {
      return handle->IsNull();
    }

    // Inequality operator for null.
    inline bool operator!=(std::nullptr_t const & rhs) const
    {
      return !handle->IsNull();
    }

    // Negation operator
    inline bool operator!() const
    {
      return handle->IsNull();
    }

    // Dereference operator
    inline T & operator*()
    {
      return *(handle->Get<T>());
    }

    // Const dereference operator.
    inline T const & operator* () const
    {
      return *(handle->Get<T const>());
    }

    // Access operator.
    inline T * operator->()
    {
      return handle->Get<T>();
    }

    // Const access operator
    inline T const * operator->() const
    {
      return handle->Get<T const>();
    }

    // Casts pointer from type T to U with static_cast.
    template <typename U>
    Pointer<U> p_static_cast() const
    {
      U * p = static_cast<U *>(static_cast<T *>(nullptr));
      return Pointer<U>(*handle);
    }

    // Casts pointer from type T to U with dynamic_cast.
    template <typename U>
    Pointer<U> p_dynamic_cast() const
    {
      U * p = dynamic_cast<U *>(static_cast<T *>(handle->GetRawPointer()));
      if (p == nullptr)
      {
        // Dynamic cast failed. Return a null pointer.
        return Pointer<U>();
      }

      return Pointer<U>(*handle);
    }

#ifdef MEMORYMANAGER_DEBUG
    // Gets the handle referenced by the pointer.
    Handle & GetHandle() const
    {
      return *handle;
    }

    /*
      Frees the handle associated with the pointer.
      file      - the file where the allocation occurred
      line      - the line where the allocation occurerd
    */
    inline void Free(const char * file, unsigned line)
    {
      handle->Free<T>(file, line);
      handle->RemoveRef(file, line);
      handle = &Handle::Null;
      handle->AddRef();
    }
#else
    inline void Free()
    {
      handle->Free<T>();
      handle->RemoveRef();
      handle = &Handle::Null;
      handle->AddRef();
    }
#endif

  private:

    // the handle references by the Pointer
    Handle * handle;
  };

  // Helper function that creates a handle and returns a pointer referencing the handle.
#ifdef MEMORYMANAGER_DEBUG
  template <typename T>
  Pointer<T> PointerAllocate(ObjectAllocator<T> & allocator, void * memory, char const * file, unsigned line)
  {
    Handle & handle = Handle::CreateHandle(&allocator, memory, file, line);
    return Pointer<T>(handle);
  }
#else
  template <typename T>
  Pointer<T> PointerAllocate(ObjectAllocator<T> & allocator, void * memory)
  {
    Handle & handle = Handle::CreateHandle(&allocator, memory);
    return Pointer<T>(handle);
  }
#endif
}

#ifdef MEMORYMANAGER_DEBUG
#define MM_PALLOC(allocator, constructor) MemoryManager::PointerAllocate(allocator, (void*)MM_ALLOC(allocator, constructor), __FILE__, __LINE__)
#define MM_PFREE(pointer) pointer.Free(__FILE__, __LINE__)
#else
#define MM_PALLOC(allocator, constructor) MemoryManager::PointerAllocate(allocator, (void*)MM_ALLOC(allocator, constructor))
#define MM_PFREE(pointer) pointer.Free()
#endif

#endif // Pointer_h
