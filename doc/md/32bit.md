https://github.com/joehattori/xv6-rv32im/commit/de7444f3b6aa78de2197d7049bdb512bb3dcd444?fbclid=IwAR3WDSlA73E2PWhCRIPd4zY4yKVz0cWx8ohcZ74a36eB1ijXL9Z_TRvdrYU


kernel/riscv.h

```cpp
// extract the three 9-bit page table indices from a virtual address.
#define PXMASK          0x1FF // 9 bits
#define PXSHIFT(level)  (PGSHIFT+(9*(level)))
#define PX(level, va) ((((uint64) (va)) >> PXSHIFT(level)) & PXMASK)
// extract the two 10-bit page table indices from a virtual address.
#define PXMASK          0x3FF // 10 bits
#define PXSHIFT(level)  (PGSHIFT+(10*(level)))
#define PX(level, va) ((((uint32) (va)) >> PXSHIFT(level)) & PXMASK)

// one beyond the highest possible virtual address.
// MAXVA is actually one bit less than the max allowed by
// Sv39, to avoid having to sign-extend virtual addresses
// Sv32, to avoid having to sign-extend virtual addresses
// that have the high bit set.
#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))
#define MAXVA 0xFFFFFFFF

typedef uint64 pte_t;
typedef uint64 *pagetable_t; // 512 PTEs
typedef uint32 pte_t;
typedef uint32 *pagetable_t; // 1024 PTEs
```

kernel/spinlock.c 

```cpp
@@ -8,6 +8,12 @@
#include "proc.h"
#include "defs.h"

uint test_and_set(uint *locked) {
  uint old = *locked;
  *locked = 1;
  return old;
}

void
initlock(struct spinlock *lk, char *name)
{
@@ -29,8 +35,8 @@ acquire(struct spinlock *lk)
  //   a5 = 1
  //   s1 = &lk->locked
  //   amoswap.w.aq a5, a5, (s1)
  while(__sync_lock_test_and_set(&lk->locked, 1) != 0)
    ;
  // while(__sync_lock_test_and_set(&lk->locked, 1) != 0);
  while(test_and_set(&lk->locked) != 0);

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
@@ -66,7 +72,8 @@ release(struct spinlock *lk)
  // On RISC-V, sync_lock_release turns into an atomic swap:
  //   s1 = &lk->locked
  //   amoswap.w zero, zero, (s1)
  __sync_lock_release(&lk->locked);
  // __sync_lock_release(&lk->locked);
  lk->locked = 0;

  pop_off();
}
```

 kernel/types.h 
```cpp
@@ -5,6 +5,6 @@ typedef unsigned char  uchar;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
typedef unsigned long uint64;
typedef unsigned long long  uint64;

typedef uint64 pde_t;
typedef uint32 pde_t;
```

kernel/vm.c 

```cpp
// create any required page-table pages.
//
// The risc-v Sv39 scheme has three levels of page-table
// pages. A page-table page contains 512 64-bit PTEs.
// A 64-bit virtual address is split into five fields:
//   39..63 -- must be zero.
//   30..38 -- 9 bits of level-2 index.
//   21..29 -- 9 bits of level-1 index.
//   12..20 -- 9 bits of level-0 index.
// pages. A page-table page contains 1024 32-bit PTEs.
// A 32-bit virtual address is split into three fields:
//   22..31 -- 10 bits of level-1 index.
//   12..21 -- 10 bits of level-0 index.
//    0..11 -- 12 bits of byte offset within the page.
pte_t *
walk(pagetable_t pagetable, uint64 va, int alloc)
walk(pagetable_t pagetable, uint32 va, int alloc)
{
  if(va >= MAXVA)
    panic("walk");

  for(int level = 2; level > 0; level--) {
  for(int level = 1; level > 0; level--) {
    pte_t *pte = &pagetable[PX(level, va)];
    if(*pte & PTE_V) {
      pagetable = (pagetable_t)PTE2PA(*pte);
@@ -100,11 +98,11 @@ walk(pagetable_t pagetable, uint64 va, int alloc)
// Look up a virtual address, return the physical address,
// or 0 if not mapped.
// Can only be used to look up user pages.
uint64
walkaddr(pagetable_t pagetable, uint64 va)
uint32
walkaddr(pagetable_t pagetable, uint32 va)
{
  pte_t *pte;
  uint64 pa;
  uint32 pa;

  if(va >= MAXVA)
    return 0;
@@ -124,7 +122,7 @@ walkaddr(pagetable_t pagetable, uint64 va)
// only used when booting.
// does not flush TLB or enable paging.
void
kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
kvmmap(pagetable_t kpgtbl, uint32 va, uint32 pa, uint32 sz, int perm)
{
  if(mappages(kpgtbl, va, sz, pa, perm) != 0)
    panic("kvmmap");
@@ -135,9 +133,9 @@ kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
// be page-aligned. Returns 0 on success, -1 if walk() couldn't
// allocate a needed page-table page.
int
mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
mappages(pagetable_t pagetable, uint32 va, uint32 size, uint32 pa, int perm)
{
  uint64 a, last;
  uint32 a, last;
  pte_t *pte;

  a = PGROUNDDOWN(va);
@@ -160,23 +158,23 @@ mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
// page-aligned. The mappings must exist.
// Optionally free the physical memory.
void
uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
uvmunmap(pagetable_t pagetable, uint32 va, uint32 npages, int do_free)
{
  uint64 a;
  pte_t *pte;

  if((va % PGSIZE) != 0)
    panic("uvmunmap: not aligned");

  for(a = va; a < va + npages*PGSIZE; a += PGSIZE){
  for (int i = 0; i < npages; i++) {
    uint32 a = va + PGSIZE * i;
    if((pte = walk(pagetable, a, 0)) == 0)
      panic("uvmunmap: walk");
    if((*pte & PTE_V) == 0)
      panic("uvmunmap: not mapped");
    if(PTE_FLAGS(*pte) == PTE_V)
      panic("uvmunmap: not a leaf");
    if(do_free){
      uint64 pa = PTE2PA(*pte);
      uint32 pa = PTE2PA(*pte);
      kfree((void*)pa);
    }
    *pte = 0;
@@ -208,17 +206,17 @@ uvminit(pagetable_t pagetable, uchar *src, uint sz)
    panic("inituvm: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  mappages(pagetable, 0, PGSIZE, (uint64)mem, PTE_W|PTE_R|PTE_X|PTE_U);
  mappages(pagetable, 0, PGSIZE, (uint32)mem, PTE_W|PTE_R|PTE_X|PTE_U);
  memmove(mem, src, sz);
}

// Allocate PTEs and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
uint64
uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
uint32
uvmalloc(pagetable_t pagetable, uint32 oldsz, uint32 newsz)
{
  char *mem;
  uint64 a;
  uint32 a;

  if(newsz < oldsz)
    return oldsz;
@@ -231,7 +229,7 @@ uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
      return 0;
    }
    memset(mem, 0, PGSIZE);
    if(mappages(pagetable, a, PGSIZE, (uint64)mem, PTE_W|PTE_X|PTE_R|PTE_U) != 0){
    if(mappages(pagetable, a, PGSIZE, (uint32)mem, PTE_W|PTE_X|PTE_R|PTE_U) != 0){
      kfree(mem);
      uvmdealloc(pagetable, a, oldsz);
      return 0;
@@ -244,8 +242,8 @@ uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
uint64
uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
uint32
uvmdealloc(pagetable_t pagetable, uint32 oldsz, uint32 newsz)
{
  if(newsz >= oldsz)
    return oldsz;
@@ -263,15 +261,16 @@ uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
void
freewalk(pagetable_t pagetable)
{
  // there are 2^9 = 512 PTEs in a page table.
  for(int i = 0; i < 512; i++){
  // there are 2^10 = 1024 PTEs in a page table.
  for(int i = 0; i < 1024; i++){
    pte_t pte = pagetable[i];
    if((pte & PTE_V) && (pte & (PTE_R|PTE_W|PTE_X)) == 0){
      // this PTE points to a lower-level page table.
      uint64 child = PTE2PA(pte);
      uint32 child = PTE2PA(pte);
      freewalk((pagetable_t)child);
      pagetable[i] = 0;
    } else if(pte & PTE_V){
      printf("%d\n", i);
      panic("freewalk: leaf");
    }
  }
```

user/initcode.S 
```cpp
@@ -22,7 +22,7 @@ init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
.p2align 5
argv:
  .long init
  .long 0
```


user/usertests.c 
@@ -26,10 +26,10 @@ char buf[BUFSZ];
void
copyin(char *s)
{
  uint64 addrs[] = { 0x80000000LL, 0xffffffffffffffff };
  uint32 addrs[] = { 0x80000000LL, 0xffffffff };

  for(int ai = 0; ai < 2; ai++){
    uint64 addr = addrs[ai];
    uint32 addr = addrs[ai];

    int fd = open("copyin1", O_CREATE|O_WRONLY);
    if(fd < 0){
@@ -70,10 +70,10 @@ copyin(char *s)
void
copyout(char *s)
{
  uint64 addrs[] = { 0x80000000LL, 0xffffffffffffffff };
  uint32 addrs[] = { 0x80000000LL, 0xffffffff };

  for(int ai = 0; ai < 2; ai++){
    uint64 addr = addrs[ai];
    uint32 addr = addrs[ai];

    int fd = open("README", 0);
    if(fd < 0){
@@ -111,10 +111,10 @@ copyout(char *s)
void
copyinstr1(char *s)
{
  uint64 addrs[] = { 0x80000000LL, 0xffffffffffffffff };
  uint32 addrs[] = { 0x80000000LL, 0xffffffff };

  for(int ai = 0; ai < 2; ai++){
    uint64 addr = addrs[ai];
    uint32 addr = addrs[ai];