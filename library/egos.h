#pragma once

struct earth {
    /* CPU interface */
    int (*intr_enable)();
    int (*intr_register)(void (*handler)(int));
    int (*excp_register)(void (*handler)(int));

    int (*mmu_alloc)(int* frame_no, void** cached_addr);
    int (*mmu_free)(int pid);
    int (*mmu_map)(int pid, int page_no, int frame_no);
    int (*mmu_switch)(int pid);

    /* Devices interface */
    int (*disk_read)(int block_no, int nblocks, char* dst);
    int (*disk_write)(int block_no, int nblocks, char* src);

    int (*tty_intr)();
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(char* buf, int len);

    int (*tty_printf)(const char *format, ...);
    int (*tty_info)(const char *format, ...);
    int (*tty_fatal)(const char *format, ...);
    int (*tty_success)(const char *format, ...);
    int (*tty_critical)(const char *format, ...);

    /* QEMU or Arty, detected in mmu_init() */
    enum { QEMU, ARTY } platform;
};

struct grass {
    /* Process control interface */
    int  (*proc_alloc)();
    void (*proc_free)(int pid);
    void (*proc_set_ready)(int pid);

    /* System call interface */
    void (*sys_exit)(int status);
    int  (*sys_send)(int pid, char* msg, int size);
    int  (*sys_recv)(int* pid, char* buf, int size);

    /* Shell environment variables */
    int workdir_ino;
    char workdir[128];
};

extern struct earth *earth;
extern struct grass *grass;
#define MAX_NPROCESS     16

#ifndef LIBC_STDIO
#define printf             earth->tty_printf
#define INFO               earth->tty_info
#define FATAL              earth->tty_fatal
#define SUCCESS            earth->tty_success
#define CRITICAL           earth->tty_critical
#endif

/* Memory layout */
#define PAGE_SIZE          4096
#define FRAME_CACHE_END    0x80020000
#define FRAME_CACHE_START  0x80004000  /* 112KB  frame cache           */
                                       /*        earth interface       */
#define GRASS_STACK_TOP    0x80003f80  /* 7KB    earth/grass stack     */
#define SYSCALL_ARG        0x80002400  /* 1KB    system call args      */
                                       /*        grass interface       */
#define APPS_STACK_TOP     0x80002000  /* 6KB    app stack             */
#define SAVED_INTR_STACK   0x80000400  /* 1KB    saved interrupt stack */
#define APPS_ARG           0x80000000  /* 1KB    app main() argc, argv */
#define APPS_SIZE          0x00003000  
#define APPS_ENTRY         0x08005000  /* 12KB   app code+data         */
#define GRASS_SIZE         0x00002800
#define GRASS_ENTRY        0x08002800  /* 8KB    grass code+data       */
                                       /* 12KB   earth data            */
                                       /* earth code is in QSPI flash  */

/* Memory-mapped I/O register access macros */
#define ACCESS(x) (*(__typeof__(*x) volatile *)(x))
#define REGW(base, offset) (ACCESS((unsigned int*)(base + offset)))
#define REGB(base, offset) (ACCESS((unsigned char*)(base + offset)))
