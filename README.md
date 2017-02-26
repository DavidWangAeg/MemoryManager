# MemoryManager

TODO: Add example project that uses these classes

## Description
This is a personal Memory Manager project in C++. The goal is to create a specialized allocator for use with objects that are created/destroyed often, for example game objects.

## Object Allocator
The base object allocator class with allocate and return pointers to the object type that is given to the allocator. This will track some basic error cases, however will not be able to detect dangling pointer access (access to memory that has been reallocated).

## Pointer/Handler
The Handler class acts as a wrapper around the raw Object Allocator pointer, and is made to work in conjunction with the Pointer class. Using this interface instead of the base Object Allocator class, this will be able to detect dangling pointer access. However, raw C++ pointers cannot be used, and all operations must go through the Pointer<T> class. This class should support most pointer operations, and behave similarly to T * type. Note that there is no direct conversion from T * to Pointer<T> since Pointer<T> is made only to manage memory given by Object Allocators, not any free memory.

## Options
* MEMORYMANAGER_DEBUG - With this define, all debug options for the memory manager are enabled. This includes additional debug information stored with memory, as well as validation checks performed on free.

* MEMORYMANAGER_ENABLE_EXCEPTIONS - Note that debug must also be enabled. This will cause the manager to throw MemoryManagerException when it encounters an error case rather than logging. This was mostly added to simplify test scenarios, and is generally not recommended to use normally.


