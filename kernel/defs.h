struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct sleeplock;
struct stat;
struct superblock;
struct mbuf;
struct sock;
struct map_info; // ccc:mmap

// bio.c
void            binit(void);
struct buf*     bread(uint, uint);
void            brelse(struct buf*);
void            bwrite(struct buf*);
void            bpin(struct buf*);
void            bunpin(struct buf*);

// console.c
void            consoleinit(void);
void            consoleintr(int);
void            consputc(int);

// pci.c
void            pci_init();

// e1000.c
void            e1000_init(uint32 *);
void            e1000_intr(void);
int             e1000_transmit(struct mbuf*);

// exec.c
int             exec(char*, char**);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, word_t, int n);
int             filestat(struct file*, word_t addr);
int             filewrite(struct file*, word_t, int n);

// fs.c
void            fsinit(int);
int             dirlink(struct inode*, char*, uint);
struct inode*   dirlookup(struct inode*, char*, uint*);
struct inode*   ialloc(uint, short);
struct inode*   idup(struct inode*);
void            iinit();
void            ilock(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
int             readi(struct inode*, int, word_t, uint, uint);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, int, word_t, uint, uint);

// net.c
void            net_rx(struct mbuf*);
void            net_tx_udp(struct mbuf*, uint32, uint16, uint16);

// sysnet.c
void            sockinit(void);
int             sockalloc(struct file **, uint32, uint16, uint16);
void            sockrecvudp(struct mbuf*, uint32, uint16, uint16);
void            sockclose(struct sock *);
int             sockwrite(struct sock *, word_t, int);
int             sockread(struct sock *, word_t, int);

// ramdisk.c
void            ramdiskinit(void);
void            ramdiskintr(void);
void            ramdiskrw(struct buf*);

// kalloc.c
void*           kalloc(void);
void            kfree(void *);
void            kinit();
void            kref(void*);   // ccc:mmap
void            kderef(void*); // ccc:mmap

// log.c
void            initlog(int, struct superblock*);
void            log_write(struct buf*);
void            begin_op(int);
void            end_op(int);
void            crash_op(int,int);

// pipe.c
int             pipealloc(struct file**, struct file**);
void            pipeclose(struct pipe*, int);
int             piperead(struct pipe*, word_t, int);
int             pipewrite(struct pipe*, word_t, int);

// printf.c
void            printf(char*, ...);
void            panic(char*) __attribute__((noreturn));
void            printfinit(void);

// proc.c
int             cpuid(void);
void            exit(int);
int             fork(void);
int             growproc(int);
pagetable_t     proc_pagetable(struct proc *);
void            proc_freepagetable(pagetable_t, word_t);
int             kill(int);
struct cpu*     mycpu(void);
struct cpu*     getmycpu(void);
struct proc*    myproc();
void            procinit(void);
void            scheduler(void) __attribute__((noreturn));
void            sched(void);
void            setproc(struct proc*);
void            sleep(void*, struct spinlock*);
void            userinit(void);
int             wait(word_t);
void            wakeup(void*);
void            yield(void);
int             either_copyout(int user_dst, word_t dst, void *src, word_t len);
int             either_copyin(void *dst, int user_src, word_t src, word_t len);
void            procdump(void);

// swtch.S
void            swtch(struct context*, struct context*);

// spinlock.c
void            acquire(struct spinlock*);
int             holding(struct spinlock*);
void            initlock(struct spinlock*, char*);
void            release(struct spinlock*);
void            push_off(void);
void            pop_off(void);
word_t          sys_ntas(void);

// sleeplock.c
void            acquiresleep(struct sleeplock*);
void            releasesleep(struct sleeplock*);
int             holdingsleep(struct sleeplock*);
void            initsleeplock(struct sleeplock*, char*);

// string.c
int             memcmp(const void*, const void*, uint);
void*           memmove(void*, const void*, uint);
void*           memset(void*, int, uint);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);

// syscall.c
int             argint(int, int*);
int             argstr(int, char*, int);
int             argaddr(int, word_t *);
int             fetchstr(word_t, char*, int);
int             fetchaddr(word_t, word_t*);
void            syscall();

// trap.c
extern uint     ticks;
void            trapinit(void);
void            trapinithart(void);
extern struct spinlock tickslock;
void            usertrapret(void);

// uart.c
void            uartinit(void);
void            uartintr(void);
void            uartputc(int);
int             uartgetc(void);

// vm.c
void            kvminit(void);
void            kvminithart(void);
word_t          kvmpa(word_t);
void            kvmmap(word_t, word_t, word_t, int);
int             mappages(pagetable_t, word_t, word_t, word_t, int);
pagetable_t     uvmcreate(void);
void            uvminit(pagetable_t, uchar *, uint);
word_t          uvmalloc(pagetable_t, word_t, word_t);
word_t          uvmdealloc(pagetable_t, word_t, word_t);
int             uvmcopy(pagetable_t, pagetable_t, word_t);
void            uvmfree(pagetable_t, word_t);
void            uvmunmap(pagetable_t, word_t, word_t, int);
void            uvmclear(pagetable_t, word_t);
word_t          walkaddr(pagetable_t, word_t);
int             copyout(pagetable_t, word_t, char *, word_t);
int             copyin(pagetable_t, char *, word_t, word_t);
int             copyinstr(pagetable_t, char *, word_t, word_t);

// ccc:mmap
word_t          sys_mmap(void);
word_t          sys_munmap(void);
void            mmap_dup(pagetable_t, struct map_info*);
void            mmap_dedup(pagetable_t, struct map_info*);

// plic.c
void            plicinit(void);
void            plicinithart(void);
int             plic_claim(void);
void            plic_complete(int);

// virtio_disk.c
void            virtio_disk_init(int);
void            virtio_disk_rw(int, struct buf *, int);
void            virtio_disk_intr(int);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

// Extra files for allocator lab


// buddy.c
void           bd_init(void*,void*);
void           bd_free(void*);
void           *bd_malloc(word_t);

struct list {
  struct list *next;
  struct list *prev;
};

// list.c
void lst_init(struct list*);
void lst_remove(struct list*);
void lst_push(struct list*, void *);
void *lst_pop(struct list*);
void lst_print(struct list*);
int lst_empty(struct list*);
