#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/* Possible states of a thread: */
#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2

#define STACK_SIZE  8192
#define MAX_THREAD  4

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

struct thread {
  char       stack[STACK_SIZE]; /* the thread's stack */
  struct context ctx;           /* saved context */
  int        state;             /* FREE, RUNNING, RUNNABLE */
};
struct thread all_thread[MAX_THREAD];
struct thread *current_thread;
extern void thread_switch(word_t, word_t);

void 
thread_init(void)
{
  // main() is thread 0, which will make the first invocation to
  // thread_schedule().  it needs a stack so that the first thread_switch() can
  // save thread 0's state.  thread_schedule() won't run the main thread ever
  // again, because its state is set to RUNNING, and thread_schedule() selects
  // a RUNNABLE thread.
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
}

void 
thread_schedule(void)
{
  struct thread *t, *next_thread;

  /* Find another runnable thread. */
  next_thread = 0;
  t = current_thread + 1;
  for(int i = 0; i < MAX_THREAD; i++){
    if(t >= all_thread + MAX_THREAD)
      t = all_thread;
    if(t->state == RUNNABLE) {
      next_thread = t;
      break;
    }
    t = t + 1;
  }

  if (next_thread == 0) {
    printf("thread_schedule: no runnable threads\n");
    exit(-1);
  }

  if (current_thread != next_thread) {         /* switch threads?  */
    next_thread->state = RUNNING;
    t = current_thread;
    current_thread = next_thread;
    /* YOUR CODE HERE
     * Invoke thread_switch to switch from t to next_thread:
     * thread_switch(??, ??);
     */
    thread_switch((word_t) &t->ctx, (word_t) &next_thread->ctx);
  } else
    next_thread = 0;
}

void 
thread_create(void (*func)())
{
  struct thread *t;

  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;
  }
  t->state = RUNNABLE;

  t->ctx.ra = (word_t) func;
  t->ctx.sp = (word_t) (&t->stack) + STACK_SIZE;
}

void 
thread_yield(void)
{
  current_thread->state = RUNNABLE;
  thread_schedule();
}

volatile int a_started, b_started, c_started;
volatile int a_n, b_n, c_n;

void 
thread_a(void)
{
  int i;
  printf("thread_a started\n");
  a_started = 1;
  while(b_started == 0 || c_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_a %d\n", i);
    a_n += 1;
    thread_yield();
  }
  printf("thread_a: exit after %d\n", a_n);

  current_thread->state = FREE;
  thread_schedule();
}

void 
thread_b(void)
{
  int i;
  printf("thread_b started\n");
  b_started = 1;
  while(a_started == 0 || c_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_b %d\n", i);
    b_n += 1;
    thread_yield();
  }
  printf("thread_b: exit after %d\n", b_n);

  current_thread->state = FREE;
  thread_schedule();
}

void 
thread_c(void)
{
  int i;
  printf("thread_c started\n");
  c_started = 1;
  while(a_started == 0 || b_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_c %d\n", i);
    c_n += 1;
    thread_yield();
  }
  printf("thread_c: exit after %d\n", c_n);

  current_thread->state = FREE;
  thread_schedule();
}

int 
main(int argc, char *argv[]) 
{
  a_started = b_started = c_started = 0;
  a_n = b_n = c_n = 0;
  thread_init();
  thread_create(thread_a);
  thread_create(thread_b);
  thread_create(thread_c);
  thread_schedule();
  exit(0);
}
