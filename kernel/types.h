typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;

#ifdef __RV32__
typedef unsigned long long uint64;
typedef uint32 word_t;
typedef uint32 pde_t;
#else // RV64
typedef unsigned long uint64;
typedef uint64 word_t;
typedef uint64 pde_t;
#endif

