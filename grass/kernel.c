/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: Kernel ≈ 2 handlers
 *     intr_entry() handles timer, keyboard, and other interrupts
 *     excp_entry() handles faults (e.g., unauthorized memory access) and system
 * calls
 */

 #include "egos.h"
 #include "syscall.h"
 #include "process.h"
 #include <string.h>
 
 uint core_in_kernel;
 uint core_to_proc_idx[NCORES + 1];
 /* QEMU has cores with ID #1 .. #NCORES */
 /* Arty has cores with ID #0 .. #NCORES-1 */
 struct process proc_set[MAX_NPROCESS + 1];
 /* proc_set[0..MAX_NPROCESS-1] are actual processes */
 /* proc_set[MAX_NPROCESS] is a place holder for idle cores */
 
 #define curr_proc_idx core_to_proc_idx[core_in_kernel]
 #define curr_pid      proc_set[curr_proc_idx].pid
 #define curr_status   proc_set[curr_proc_idx].status
 #define CORE_IDLE     (curr_proc_idx == MAX_NPROCESS)
 void core_set_idle(uint core) { core_to_proc_idx[core] = MAX_NPROCESS; }
 
 static void intr_entry(uint);
 static void excp_entry(uint);
 
 void kernel_entry(uint mcause) {
     /* With the kernel lock, only one core can enter this point at any time */
     asm("csrr %0, mhartid" : "=r"(core_in_kernel));
 
     /* Save process context */
     asm("csrr %0, mepc" : "=r"(proc_set[curr_proc_idx].mepc));
     memcpy(proc_set[curr_proc_idx].saved_register, SAVED_REGISTER_ADDR,
            SAVED_REGISTER_SIZE);
 
     (mcause & (1 << 31)) ? intr_entry(mcause & 0x3FF) : excp_entry(mcause);
 
     /* Restore process context */
     asm("csrw mepc, %0" ::"r"(proc_set[curr_proc_idx].mepc));
     memcpy(SAVED_REGISTER_ADDR, proc_set[curr_proc_idx].saved_register,
            SAVED_REGISTER_SIZE);
 }
 
 #define INTR_ID_CTRL_C  2
 #define INTR_ID_TIMER   7
 #define EXCP_ID_ECALL_U 8
 #define EXCP_ID_ECALL_M 11
 static void proc_yield(uint id);
 static void proc_try_syscall(struct process* proc);
 
 static void excp_entry(uint id) {
     if (id >= EXCP_ID_ECALL_U && id <= EXCP_ID_ECALL_M) {
         /* Copy the system call arguments from user space to the kernel */
         uint syscall_paddr = earth->mmu_translate(curr_pid, SYSCALL_ARG);
         memcpy(&proc_set[curr_proc_idx].syscall, (void*)syscall_paddr,
                sizeof(struct syscall));
 
         proc_set[curr_proc_idx].mepc += 4;
         proc_set[curr_proc_idx].syscall.status = PENDING;
         proc_try_syscall(&proc_set[curr_proc_idx]);
         proc_yield(id);
         return;
     }
     /* Student's code goes here (system call and memory exception).
      * Kill the process if curr_pid is a user application */
    if (curr_pid >= GPID_USER_START) {
        proc_set[curr_proc_idx].status = PROC_UNUSED;
        INFO("process %d terminated with exception %d", curr_pid, id);
        exit(1);
    }
 
     /* Student's code ends here. */
     FATAL("excp_entry: kernel got exception %d", id);
 }
 
 static void intr_entry(uint id) {
     if (id == INTR_ID_TIMER) {
         proc_yield(id);
         return;
     }
 
     /* Student's code goes here (ethernet & tcp/ip).
      * Handle Ethernet device interrupt */
 
     /* Student's code ends here. */
 
     FATAL("excp_entry: kernel got interrupt %d", id);
 }


static void wait_for_interrupt() {
    #define MSTATUS_MIE_POS 3
    uint mstatus;
    asm("csrr %0, mstatus":"=r"(mstatus));

    mstatus |= (1 << MSTATUS_MIE_POS);
    asm("csrw mstatus, %0"::"r"(mstatus));
    asm("wfi");
}
 
 static void proc_yield(uint id) {
     /* Set the current process status to RUNNABLE if it was RUNNING */
     if (!CORE_IDLE && curr_status == PROC_RUNNING) proc_set_runnable(curr_pid);
 
     /* Student's code goes here (preemptive scheduler)
      * Replace the loop to find the next process with your sheduler logic. */
     /* Decrement sleep counters */
     for (uint i = 0; i < MAX_NPROCESS; i++) {
         if (proc_set[i].time > 0) {
             proc_set[i].time-= 1;
             if(proc_set[i].time < 0) {
                 proc_set[i].time = 0;
             }
         } else if (proc_set[i].status == PROC_SLEEPING && proc_set[i].time == 0) {
             proc_set[i].status = PROC_RUNNABLE;
         }
     }
 
     /* Find next process that is not sleeping */
     int next_idx = MAX_NPROCESS;
     for (uint i = 1; i <= MAX_NPROCESS; i++) {
         struct process* p = &proc_set[(curr_proc_idx + i) % MAX_NPROCESS];
 
         if (p->time == 0 && 
             (p->status == PROC_READY || p->status == PROC_RUNNABLE)) {
             next_idx = (curr_proc_idx + i) % MAX_NPROCESS;
             break;
         }
     }
     /* Find the next process to run */
     for (uint i = 1; i <= MAX_NPROCESS; i++) {
         struct process* p = &proc_set[(curr_proc_idx + i) % MAX_NPROCESS];
         if (p->status == PROC_PENDING_SYSCALL) proc_try_syscall(p);
 
         if (p->status == PROC_READY || p->status == PROC_RUNNABLE) {
             next_idx = (curr_proc_idx + i) % MAX_NPROCESS;
             break;
         }
     }
 
     /* Measure and record scheduling metrics for the current process before it
      * yields; Measure and record scheduling metrics for the next process. */
 
     /* Student's code ends here*/
 
     /* Context switch */
    curr_proc_idx = next_idx;
    earth->timer_reset(core_in_kernel);
    if (CORE_IDLE) {
        /* Student's code goes here (multi-core and atomic instruction) */

        /* Release the kernel lock; Enable interrupts by modifying mstatus; */
        /* Wait for a timer interrupt with the wfi instruction. */
        wait_for_interrupt();

        /* Student's code ends here. */
        FATAL("proc_yield: no process to run on core %d", core_in_kernel);
    }
    earth->mmu_switch(curr_pid);
    earth->mmu_flush_cache();

    /* Student's code goes here (protection, virtual memory, and multi-core).
     */

    /* Modify mstatus.MPP to enter machine, supervisor, or user mode
     * after mret depending on whether curr_pid is a kernel process. */
    uint mstatus;
    asm("csrr %0, mstatus" : "=r"(mstatus));
    if (curr_pid < GPID_USER_START) {
        mstatus = (mstatus & ~(0b11 << 11)) | (0b11 << 11);
    } else {
        mstatus = (mstatus & ~(0b11 << 11)) | (0b00 << 11);
    }
    asm("csrw mstatus, %0" ::"r"(mstatus));

    /* Student's code ends here. */

    /* Setup the entry point for a newly created process */
    if (curr_status == PROC_READY) {
        /* Set argc, argv and initial program counter */
        proc_set[curr_proc_idx].saved_register[0] = APPS_ARG;
        proc_set[curr_proc_idx].saved_register[1] = APPS_ARG + 4;
        proc_set[curr_proc_idx].mepc              = APPS_ENTRY;
    }
    proc_set_running(curr_pid);
 }
 
 void proc_sleep(int pid, int nticks) {
 
     for (uint i = 0; i < MAX_NPROCESS; i++) {
         if (proc_set[i].pid == pid && proc_set[i].status != PROC_UNUSED) {
             proc_set[i].time = nticks;
             proc_set[i].status = PROC_SLEEPING;
             return;
         }
     }
     FATAL("proc_sleep: process %d not found or invalid", pid);
 }
 
 static int proc_try_send(struct process* sender) {
     for (uint i = 0; i < MAX_NPROCESS; i++) {
         struct process* dst = &proc_set[i];
         if (dst->pid == sender->syscall.receiver &&
             dst->status != PROC_UNUSED) {
             /* Return -1 if dst is not receiving, or will not take msg from
              * sender */
             if (!(dst->syscall.type == SYS_RECV &&
                   dst->syscall.status == PENDING))
                 return -1;
             if (!(dst->syscall.sender == GPID_ALL ||
                   dst->syscall.sender == sender->pid))
                 return -1;
 
             dst->syscall.status = DONE;
             dst->syscall.sender = sender->pid;
             /* Copy the system call arguments within the kernel PCB */
             memcpy(dst->syscall.content, sender->syscall.content,
                    SYSCALL_MSG_LEN);
             return 0;
         }
     }
     FATAL("proc_try_send: process %d sending to unknown process %d",
           sender->pid, sender->syscall.receiver);
 }
 
 static int proc_try_recv(struct process* receiver) {
     if (receiver->syscall.status == PENDING) return -1;
 
     /* Copy the system call results from the kernel back to user space */
     earth->mmu_switch(receiver->pid);
     earth->mmu_flush_cache();
     uint syscall_paddr = earth->mmu_translate(receiver->pid, SYSCALL_ARG);
     memcpy((void*)syscall_paddr, &receiver->syscall, sizeof(struct syscall));
     return 0;
 }
 
 static void proc_try_syscall(struct process* proc) {
     int rc;
 
     switch (proc->syscall.type) {
     case SYS_RECV:
         rc = proc_try_recv(proc);
         break;
     case SYS_SEND:
         rc = proc_try_send(proc);
         break;
     default:
         FATAL("proc_try_syscall: unknown syscall type=%d", proc->syscall.type);
     }
 
     (rc == 0) ? proc_set_runnable(proc->pid) : proc_set_pending(proc->pid);
 }
 
 // void proc_coresinfo() {
 //     /* Student's code goes here (multi-core and atomic instruction) */
 //     /* Print out the pid of the process running on each core;
 //      * Add this function into the grass interface so that shell can invoke it */
 
 //     /* Student's code ends here. */
 // }