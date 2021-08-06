#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

word_t
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

word_t
sys_getpid(void)
{
  return myproc()->pid;
}

word_t
sys_fork(void)
{
  return fork();
}

word_t
sys_wait(void)
{
  word_t p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

word_t
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  // ccc:lazy sbrk delete
  if(growproc(n) < 0)
    return -1;
  // ccc:lazy sbrk add
  // myproc()->sz += n;
  // if(n < 0)
  //  uvmdealloc(myproc()->pagetable, addr, myproc()->sz);
  return addr;
}

word_t
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

word_t
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
word_t
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
