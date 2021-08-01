# os5 -- A UNIX like OS on RISC-V

os5 was forked from https://github.com/monkey2000/xv6-riscv-fall19. 

However, we try to extend xv6 to os5 for teach & use. 

os5 include the following API not supported by xv6.

1. net -- socket/tcp/ip.
    * https://blog.mky.moe/mit6828/10-lab10/
2. uthread -- user level thread.
    * https://blog.mky.moe/mit6828/6-lab06/
3. mmap -- memory mapped & sharing. (IPC)
    * https://blog.mky.moe/mit6828/9-lab09/
4. c4 -- A mini c compiler.
    * https://github.com/rswier/c4
5. kilo -- A mini editor.
    * https://github.com/antirez/kilo
6. telnet
    * add by ccc
