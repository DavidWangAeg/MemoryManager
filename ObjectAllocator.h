/*----------------------------------------------------
ObjectAllocator.h

Generic object allocator class
----------------------------------------------------*/
#ifndef ObjectAllocator_h
#define ObjectAllocator_h

#ifdef MEMORYMANAGER_DEBUG
#include <iostream>
#include <fstream>
#include <iomanip>
#include <exception>
#include <sstream>
#endif
#include <cassert>
#include <cstring>

namespace MemoryManager
{
#ifdef MEMORYMANAGER_DEBUG
#ifdef MEMORYMANAGER_ENABLE_EXCEPTIONS
  // Memory manager exception class
  class MemoryManagerException : public std::exception
  {
  public:
    /*
      Constructor.
      msg       - message to display
      filename  - file for the allocation/deallocation
      line      - line number of the allocation/deallocation
    */
    MemoryManagerException(char const * msg, char const * filename, unsigned line) :
      std::exception(msg),
      filename(filename == nullptr ? "" : filename),
      line(line)
    {}

    // Returns a string representing the exception.
    virtual char const * what() const throw()
    {
      std::stringstream msg;
      msg << "[MemoryManagerException]: " << exception::what() << " File: " << filename << " Line: " << line;
      return msg.str().c_str();
    }

    // Line number of the allocation/deallocation
    unsigned line;
    
    // File of the allocation/deallocation
    std::string filename;
  };
#endif // MEMORYMANAGER_ENABLE_EXCEPTIONS

  // Debug header for allocations
  struct DebugHeader
  {
    // Whether the current block is allocated.
    bool          allocated;

    // File where the allocation occurred.
    char const *	filename;

    // Line where the allocation occurred.
    unsigned		  line;
  };
#endif // MEMORYMANAGER_DEBUG

  // Represents a generic pointer in the object allocator
  struct GenericObject
  {
    // Next block
    GenericObject * next;

    // Constructor
    GenericObject() : next(nullptr) {}
  };

#ifdef MEMORYMANAGER_DEBUG
  // Byte signature for allocated (but uninitialized) memory.
  static const unsigned char ALLOCATED = 0xAA;

  // Memory signature for freed memory.
  static const unsigned char FREED = 0xBB;

  // Memory signature for pad bytes.
  static const unsigned char PAD = 0xDD;

  // Memory signature for alignment bytes.
  static const unsigned char ALIGN = 0xEE;

  // Memory signature for unallocated memory.
  static const unsigned char UNALLOCATED = 0xFF;

  // Tracks various statistics associated with the memory manager.
  struct Stats
  {
    // Number of free (unused) blocks.
    unsigned freeBlocks = 0;

    // Number of blocks currently in use.
    unsigned blocksInUse = 0;

    // Number of pages in use.
    unsigned pagesInUse = 0;

    // The most number of blocks in use at one time.
    unsigned mostBlocksInUse = 0;

    // The most number of pages in use at one time.
    unsigned mostPagesInUse = 0;

    // Total number of allocations.
    unsigned allocations = 0;

    // Total number of deallocations
    unsigned deallocations = 0;
  };
#endif

  // Settings for ObjectAllocator
  struct ObjectAllocatorSettings
  {
    // Number of blocks per page.
    unsigned  blocksPerPage = 1024;

    // Number of pad bytes
#ifdef MEMORYMANAGER_DEBUG
    unsigned  padBytes = 2;
#else
    unsigned  padBytes = 0;
#endif

    // Block alignment
    unsigned  alignment = 4;
  };

  //Pushes a GenericObject onto a stack
  static inline void Push(GenericObject * & stack, GenericObject * obj)
  {
    obj->next = stack;
    stack = obj;
  }

  //Pops a GenericObject off the stack
  static inline GenericObject * Pop(GenericObject * & stack)
  {
    if (stack == nullptr)
    {
      return nullptr;
    }
    GenericObject * p = stack;
    stack = stack->next;
    return p;
  }

  // ObjectAllocator class
  template <typename T>
  class ObjectAllocator
  {
    // Size of the header for allocations.
#ifdef MEMORYMANAGER_DEBUG
    unsigned headerSize = sizeof(DebugHeader);
#else
    unsigned headerSize = 0;
#endif

    // Prevent copy and assignment.
    ObjectAllocator(ObjectAllocator const & rhs) {}
    ObjectAllocator & operator=(ObjectAllocator const & rhs) {}

    // Settings for the allocator.
    ObjectAllocatorSettings settings;

    // Size of each block.
    unsigned        blockSize;

    // Size of each page.
    unsigned        pageSize;

    // Number of bytes for left alignment
    unsigned        leftAlign;

    // Numebr of bytes for alignment between bytes.
    unsigned        interAlign;
#ifdef MEMORYMANAGER_DEBUG
    // Size of the left chunk. Chunks include all debug bytes with the block.
    unsigned        leftChunkSize;

    // Size of the chunk in the middle of a page. Chunks include all debug bytes with the block.
    unsigned        interChunkSize;

    // Statistics of the allocator.
    Stats           stats;

    // Output stream to send logging information.
    std::ostream *  logStream;

    // Indicates whether the log stream is owned by the allocator, and should be deleted when the allocator is deleted.
    bool            ownsLogStream;
#endif

    // List of current pages being used.
    GenericObject * pageList;

    // List of free blocks available to be used.
    GenericObject * freeList;

  public:
#ifdef MEMORYMANAGER_DEBUG
    /*
      Constructor.
      logStream - The log stream to use
      settings  - settings for the allocator
    */
    ObjectAllocator(std::ostream * logStream = nullptr, ObjectAllocatorSettings settings = ObjectAllocatorSettings());
    
    /*
      Constructor.
      logFile - The log file to open. The allocator will manage this output stream.
      settings  - settings for the allocator
    */
    ObjectAllocator(char const * logFile, ObjectAllocatorSettings settings = ObjectAllocatorSettings());
#else
    ObjectAllocator(ObjectAllocatorSettings settings = ObjectAllocatorSettings());
#endif

    /*
      Destructor.
      Cleans up pages. In debug mode, dumps all remaining used blocks to the log stream.
    */
    ~ObjectAllocator();

#ifdef MEMORYMANAGER_DEBUG
    /*
      Dumps all memory in use to the output stream.
      outputStream - output stream to dump to.
    */
    void DumpMemoryInUse(std::ostream & outputStream) const;

    // Get allocator statistics.
    Stats GetStats() const { return stats; }

    // Get the log stream for the allocator.
    std::ostream & GetLogStream() { return *logStream; }

    // Get the debug header for the given block. This does not check the validity of the block.
    DebugHeader const * GetDebugHeader(void const * mem) const;

    /*
      Allocates and returns a block.
      file - the file the allocation came from. Used in debug header.
      line - the line the allocation came from. Used in debug header.
    */
    void * Allocate(const char * file, unsigned line);

    /*
      Frees an allocated block. Checks the validity of the free and returns an error code
      or throws if the free is invalid.
      mem  - the block to free.
      file - the file the allocation came from. Used in debug header.
      line - the line the allocation came from. Used in debug header.
    */
    unsigned char Free(void * mem, char const * file, unsigned line);
#else
    void * Allocate();
    void Free(void * mem);
#endif

  private:
    // Calculates the size a page should be
    int CalculatePageSize();

    // Creates a page and populates the free list with the created blocks.
    void CreatePage();

  }; //class ObjectAllocator

#ifdef MEMORYMANAGER_DEBUG
  template <typename T>
  ObjectAllocator<T>::ObjectAllocator(char const * logFile, ObjectAllocatorSettings settings)
    : ObjectAllocator(new std::ofstream(), settings)
  {
    ownsLogStream = true;
    ((std::ofstream*)logStream)->open(logFile);
  }

  template <typename T>
  ObjectAllocator<T>::ObjectAllocator(std::ostream * logStream, ObjectAllocatorSettings settings) :
#else
  template <typename T>
  ObjectAllocator<T>::ObjectAllocator(ObjectAllocatorSettings settings) :
#endif
    settings(settings),
    pageList(nullptr),
    freeList(nullptr),
    blockSize(sizeof(T)),
    pageSize(0),
    leftAlign(0),
    interAlign(0)
  {
    if (blockSize < sizeof(GenericObject*))
    {
      blockSize = sizeof(GenericObject*);
    }

    //Set alignment sizes
    if (settings.alignment > 1)
    {
      leftAlign = (settings.alignment - (sizeof(GenericObject*) + headerSize + settings.padBytes)) % settings.alignment;
      interAlign = (settings.alignment - (blockSize + headerSize + 2 * settings.padBytes)) % settings.alignment;
    }
#ifdef MEMORYMANAGER_DEBUG
    leftChunkSize = sizeof(GenericObject*) + leftAlign + headerSize + 2 * settings.padBytes + blockSize;
    interChunkSize = blockSize + 2 * settings.padBytes + interAlign + headerSize;
#endif
    pageSize = CalculatePageSize();

#ifdef MEMORYMANAGER_DEBUG
    this->logStream = logStream;
#endif
  }

  // Destructor
  template <typename T>
  ObjectAllocator<T>::~ObjectAllocator()
  {
#ifdef MEMORYMANAGER_DEBUG
    if (logStream != nullptr)
    {
      DumpMemoryInUse(*logStream);
    }
#endif
    char * p = nullptr;
    while (pageList)
    {
      char * page = reinterpret_cast<char*>(Pop(pageList));
      delete[] page;
    }
#ifdef MEMORYMANAGER_DEBUG
    if (ownsLogStream && logStream != nullptr)
    {
      delete logStream;
    }
#endif
  }

  template <typename T>
  int ObjectAllocator<T>::CalculatePageSize()
  {
    return sizeof(GenericObject*) + leftAlign + settings.blocksPerPage * (blockSize + 2 * settings.padBytes + headerSize + interAlign) - interAlign;
  }

#ifdef MEMORYMANAGER_DEBUG
  template <typename T>
  void * ObjectAllocator<T>::Allocate(const char * file, unsigned line)
  {
    if (freeList == nullptr)
    {
      CreatePage();
    }

    ++stats.allocations;
    ++stats.blocksInUse;
    if (stats.blocksInUse > stats.mostBlocksInUse)
    {
      stats.mostBlocksInUse = stats.blocksInUse;
    }
    --stats.freeBlocks;

    //Pop the top off the free list
    char * p = reinterpret_cast<char*>(Pop(freeList));
    memset(p, ALLOCATED, blockSize);

    //Set the debug header
    DebugHeader * dbg = reinterpret_cast<DebugHeader*>(p - headerSize - settings.padBytes);
    dbg->allocated = true;
    dbg->line = line;
    dbg->filename = file;

    return (void*)p;
  }

  template <typename T>
  unsigned char ObjectAllocator<T>::Free(void * mem, char const * filename, unsigned line)
  {
    DebugHeader const * header = GetDebugHeader(mem);
    unsigned char * del = static_cast<unsigned char*>(mem);

    //Check for valid memory address
    GenericObject * pages = pageList;

    while (pages)
    {
      //Determine if address lies in current block
      unsigned int d = reinterpret_cast<unsigned int>(del) - reinterpret_cast<unsigned int>(pages);
      if (d < pageSize)
      {
        unsigned left_offset = leftChunkSize - settings.padBytes - blockSize;
        //Page found. Check the alignment of pointer
        if (((d - left_offset) % interChunkSize) != 0)
        {
          if (logStream != nullptr)
          {
            *logStream << "Invalid alignment on free from #" << line << " in file " << filename << std::endl;
          }

#ifdef MEMORYMANAGER_ENABLE_EXCEPTIONS
          throw MemoryManagerException("Invalid alignment on free.", filename, line);
#endif
          return ALIGN;
        }
        else //Location is valid, check flags
        {
          if (!header->allocated)
          {
            if (logStream != nullptr)
            {
              *logStream << "Attempt to free already freed memory from #" << line << " in file " << filename << std::endl;
            }
#ifdef MEMORYMANAGER_ENABLE_EXCEPTIONS
            throw MemoryManagerException("Attempt to free already freed memory.", filename, line);
#endif
            return FREED;
          }

          //Check if object invalidated pad bytes
          unsigned char * pad_left = reinterpret_cast<unsigned char*>(del - 1), *pad_right = reinterpret_cast<unsigned char*>(del + blockSize);
          for (unsigned i = 0; i < settings.padBytes; ++i, --pad_left, ++pad_right)
          {
            if (*pad_left != PAD || *pad_right != PAD)
            {
              if (logStream != nullptr)
              {
                *logStream << "Pad bytes invalidated for object allocated at #" << header->line << " in file " << header->filename << std::endl;
              }
#ifdef MEMORYMANAGER_ENABLE_EXCEPTIONS
              throw MemoryManagerException("Pad bytes invalidated for object.", filename, line);
#endif
              return PAD;
            }
          }
        }

        //Checks successful. Break out
        pages = nullptr;
      }
      else
      {
        pages = pages->next;
      }
    }

    static_cast<T *>(mem)->~T();

    //Set the freed signature
    memset(static_cast<unsigned char*>(mem), FREED, blockSize);
    //Clear the header
    memset(static_cast<unsigned char *>(mem) - headerSize - settings.padBytes, 0, headerSize);

    //Add object to free list
    Push(freeList, reinterpret_cast<GenericObject*>(mem));

    //Update stats
    ++stats.deallocations;
    --stats.blocksInUse;
    ++stats.freeBlocks;
    return 0;
  }
#else
  template <typename T>
  void * ObjectAllocator<T>::Allocate()
  {
    //If no memory available
    if (freeList == nullptr)
    {
      CreatePage();
    }

    // Pop object off free list and return
    return Pop(freeList);
  }

  template <typename T>
  void ObjectAllocator<T>::Free(void * mem)
  {
    if (mem != nullptr)
    {
      static_cast<T *>(mem)->~T();

      //Add object to free list
      Push(freeList, reinterpret_cast<GenericObject*>(mem));
    }
  }
#endif

  template <typename T>
  void ObjectAllocator<T>::CreatePage()
  {
    char * p = nullptr;
    p = new char[pageSize];

    //Add page on to page list
    Push(pageList, reinterpret_cast<GenericObject*>(p));

    //Add objects on to the free list

    //Move past GenericObject pointer
    p += sizeof(GenericObject*);

#ifdef MEMORYMANAGER_DEBUG
    //Set align signature
    memset(p, ALIGN, leftAlign);
#endif
    //Most past left alignment
    p += leftAlign;

    //Zero header block
    memset(p, 0, headerSize);
    //Move past header
    p += headerSize;

#ifdef MEMORYMANAGER_DEBUG
    //Set pad signature
    memset(p, PAD, settings.padBytes);
#endif
    //Move past pad bits
    p += settings.padBytes;

    //Populate the free list except for last block
    for (unsigned i = 0; i < settings.blocksPerPage - 1; ++i)
    {
      //Block memory
#ifdef MEMORYMANAGER_DEBUG
      //Set unallocated signature
      memset(p, UNALLOCATED, blockSize);
#endif
      //Push onto free list and move past block
      Push(freeList, reinterpret_cast<GenericObject*>(p));
      p += blockSize;

#ifdef MEMORYMANAGER_DEBUG
      //Set padding signature
      memset(p, PAD, settings.padBytes);
#endif
      //Move past pad bits
      p += settings.padBytes;

#ifdef MEMORYMANAGER_DEBUG
      //Set alignment signature
      memset(p, ALIGN, interAlign);
#endif
      //Move past align bits
      p += interAlign;

      //Zero header block
      memset(p, 0, headerSize);
      //Move past header
      p += headerSize;

#ifdef MEMORYMANAGER_DEBUG
      //Set padding signature
      memset(p, PAD, settings.padBytes);
#endif
      //Move past pad bits
      p += settings.padBytes;
    }

    //Add last object separately
    //Block memory
#ifdef MEMORYMANAGER_DEBUG
    //Set unallocated signature
    memset(p, UNALLOCATED, blockSize);
#endif
    //Push onto free list and move past block
    Push(freeList, reinterpret_cast<GenericObject*>(p));
    p += blockSize;

#ifdef MEMORYMANAGER_DEBUG
    //Set padding signature
    memset(p, PAD, settings.padBytes);

    //Update stats
    ++stats.pagesInUse;
    stats.freeBlocks += settings.blocksPerPage;

    if (stats.pagesInUse > stats.mostPagesInUse)
    {
      stats.mostPagesInUse = stats.pagesInUse;
    }
#endif
  }

#ifdef MEMORYMANAGER_DEBUG
  template <typename T>
  void ObjectAllocator<T>::DumpMemoryInUse(std::ostream & outputStream) const
  {
    GenericObject * pages = pageList;
    while (pages)
    {
      //Walk through each block
      char * p = reinterpret_cast<char*>(pages);
      //Point to first block
      p += sizeof(GenericObject*) + leftAlign + headerSize + settings.padBytes;
      //Loop through blocks
      for (unsigned i = 0; i < settings.blocksPerPage; ++i)
      {
        //Check if block is still allocated
        DebugHeader const * dbg = GetDebugHeader(p);
        if (dbg->allocated)
        {
          outputStream << blockSize << "b allocated at line #" << dbg->line << " in file " << dbg->filename;
        }
        p += interChunkSize;
      }
      pages = pages->next;
    }
  }

  template <typename T>
  DebugHeader const * ObjectAllocator<T>::GetDebugHeader(void const * mem) const
  {
    return reinterpret_cast<DebugHeader const *>(reinterpret_cast<char const*>(mem) - settings.padBytes - headerSize);
  }
#endif
}

#ifdef MEMORYMANAGER_DEBUG
#define MM_ALLOC(allocator, constructor) (new (allocator.Allocate(__FILE__, __LINE__)) constructor)
#define MM_FREE(allocator, pointer) (allocator.Free(pointer, __FILE__, __LINE__))
#else
#define MM_ALLOC(allocator, constructor) (new (allocator.Allocate()) constructor)
#define MM_FREE(allocator, pointer) (allocator.Free(pointer))
#endif

#endif //AE_ObjectAllocator_h