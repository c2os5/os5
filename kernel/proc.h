// Saved registers for kernel context switches.
struct context {
  word_t ra;
  word_t sp;

  // callee-saved
  word_t s0;
  word_t s1;
  word_t s2;
  word_t s3;
  word_t s4;
  word_t s5;
  word_t s6;
  word_t s7;
  word_t s8;
  word_t s9;
  word_t s10;
  word_t s11;
};

// Per-CPU state.
struct cpu {
  struct proc *proc;          // The process running on this cpu, or null.
  struct context scheduler;   // swtch() here to enter scheduler().
  int noff;                   // Depth of push_off() nesting.
  int intena;                 // Were interrupts enabled before push_off()?
};

extern struct cpu cpus[NCPU];

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// the sscratch register points here.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
struct trapframe {
  /*   0 */ word_t kernel_satp;   // kernel page table
  /*   8 */ word_t kernel_sp;     // top of process's kernel stack
  /*  16 */ word_t kernel_trap;   // usertrap()
  /*  24 */ word_t epc;           // saved user program counter
  /*  32 */ word_t kernel_hartid; // saved kernel tp
  /*  40 */ word_t ra;
  /*  48 */ word_t sp;
  /*  56 */ word_t gp;
  /*  64 */ word_t tp;
  /*  72 */ word_t t0;
  /*  80 */ word_t t1;
  /*  88 */ word_t t2;
  /*  96 */ word_t s0;
  /* 104 */ word_t s1;
  /* 112 */ word_t a0;
  /* 120 */ word_t a1;
  /* 128 */ word_t a2;
  /* 136 */ word_t a3;
  /* 144 */ word_t a4;
  /* 152 */ word_t a5;
  /* 160 */ word_t a6;
  /* 168 */ word_t a7;
  /* 176 */ word_t s2;
  /* 184 */ word_t s3;
  /* 192 */ word_t s4;
  /* 200 */ word_t s5;
  /* 208 */ word_t s6;
  /* 216 */ word_t s7;
  /* 224 */ word_t s8;
  /* 232 */ word_t s9;
  /* 240 */ word_t s10;
  /* 248 */ word_t s11;
  /* 256 */ word_t t3;
  /* 264 */ word_t t4;
  /* 272 */ word_t t5;
  /* 280 */ word_t t6;
};

enum procstate { UNUSED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// ccc: 32/64bits -- https://www.five-embeddev.com/riscv-isa-manual/latest/supervisor.html
// 4.3 SV32... 4.4 SV39
#ifdef __RV32__
#define MMAP_VSTART (1L << 30) // 這裡亂猜，可能有問題？
#else
#define MMAP_VSTART (1L << 37)
#endif

#define MMAP_SIZE   (1L << 20)
#define MMAP_NUM    32
#define MMAP_VEND   (MMAP_VSTART + MMAP_NUM * MMAP_SIZE)

// used for mmap
struct map_info {
  word_t vstart;
  word_t vend;
  int length;
  int prot;
  int flags;
  int offset;
  int used;
  struct file *file;
};

// Per-process state
struct proc {
  struct spinlock lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  struct proc *parent;         // Parent process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  // these are private to the process, so p->lock need not be held.
  word_t kstack;               // Virtual address of kernel stack
  word_t sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // Page table
  struct trapframe *tf;        // data page for trampoline.S
  struct context context;      // swtch() here to run process
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)

  // used for mmap
  struct map_info minfo[MMAP_NUM];
};
