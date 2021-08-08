// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  uint *ref_count; // 注意： ref_count 是指標 (陣列)
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  kmem.ref_count = (uint*)end; // 設定 ref_count 指向核心結束點
  word_t rc_pages = ((((PHYSTOP - (word_t)end) >> 12) + 1) * sizeof(uint) >> 12) + 1; // 分配的頁數
  word_t rc_offset = (word_t)rc_pages << 12; // 分配的區域大小 (bytes)。 
  freerange(end + rc_offset, (void*)PHYSTOP);
  // 問：為何核心還沒啟動就得 free (xv6 也是如此，只是用 freerange(end, PHYSTOP))
  // 答：因為實體記憶體有多少，必須一開始就知道，這樣才能分配出去並管理。
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((word_t)pa_start); // 取得頁起始位址
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) // 一頁一頁釋放
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((word_t)pa % PGSIZE) != 0 || (char*)pa < end || (word_t)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  
  r = (struct run*)pa;

  word_t idx = ((word_t)pa - (word_t)end) >> 12;
  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.ref_count[idx] = 0;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
    word_t idx = ((word_t)r - (word_t)end) >> 12;
    kmem.ref_count[idx] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void
kref(void* pa) {
  word_t idx = ((word_t)pa - (word_t)end) >> 12;

  acquire(&kmem.lock);
  kmem.ref_count[idx]++;
  release(&kmem.lock);
}

void
kderef(void* pa) {
  word_t idx = ((word_t)pa - (word_t)end) >> 12;
  char shall_free = 0;

  acquire(&kmem.lock);
  kmem.ref_count[idx]--;
  if(kmem.ref_count[idx] == 0)
    shall_free = 1;
  release(&kmem.lock);

  if(shall_free)
    kfree(pa);
}
