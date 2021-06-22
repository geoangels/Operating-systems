/* This file defines constants for use in message communication (mostly)
 * between system processes.
 *
 * A number of protocol message request and response types are defined. For
 * debugging purposes, each protocol is assigned its own unique number range.
 * The following such message type ranges have been allocated:
 *
 *        1 -   0xFF    POSIX requests (see callnr.h)
 *    0x200 -  0x2FF    Data link layer requests and responses
 *    0x300 -  0x3FF    Bus controller requests and responses
 *    0x400 -  0x4FF    Character device requests
 *    0x500 -  0x5FF    Character device responses
 *    0x600 -  0x6FF    Kernel calls to SYSTEM task
 *    0x700 -  0x7FF    Reincarnation Server (RS) requests
 *    0x800 -  0x8FF    Data Store (DS) requests
 *    0x900 -  0x9FF    Requests from PM to VFS, and responses
 *    0xA00 -  0xAFF    Requests from VFS to file systems (see vfsif.h)
 *    0xB00 -  0xBFF    Requests from VM to VFS
 *    0xC00 -  0xCFF    Virtual Memory (VM) requests
 *    0xD00 -  0xDFF    IPC server requests
 *    0xE00 -  0xEFF    Common system messages (e.g. system signals)
 *    0xF00 -  0xFFF    Scheduling messages
 *   0x1000 - 0x10FF    Notify messages
 *   0x1100 - 0x11FF    USB
 *   0x1200 - 0x12FF    Devman
 *   0x1300 - 0x13FF    TTY Input
 *   0x1400 - 0x14FF    VFS-FS transaction IDs
 *   0x1500 - 0x15FF    Block device requests and responses
 *
 * Zero and negative values are widely used for OK and error responses.
 */

#ifndef _MINIX_COM_H
#define _MINIX_COM_H

/*===========================================================================*
 *                              Magic process numbers                        *
 *===========================================================================*/

/* These may not be any valid endpoint (see <minix/endpoint.h>). */
#define ANY     ((endpoint_t) 0x7ace)   /* used to indicate 'any process' */
#define NONE    ((endpoint_t) 0x6ace)   /* used to indicate 'no process at all' */
#define SELF    ((endpoint_t) 0x8ace)   /* used to indicate 'own process' */
#define _MAX_MAGIC_PROC (SELF)  /* used by <minix/endpoint.h>
                                   to determine generation size */
/*===========================================================================*
 *              Process numbers of processes in the system image             *
 *===========================================================================*/

/* The values of several task numbers depend on whether they or other tasks
 * are enabled. They are defined as (PREVIOUS_TASK - ENABLE_TASK) in general.
 * ENABLE_TASK is either 0 or 1, so a task either gets a new number, or gets
 * the same number as the previous task and is further unused. Note that the
 * order should correspond to the order in the task table defined in table.c.
 */

/* Kernel tasks. These all run in the same address space. */
#define ASYNCM  ((endpoint_t) -5) /* notifies about finished async sends */
#define IDLE    ((endpoint_t) -4) /* runs when no one else can run */
#define CLOCK   ((endpoint_t) -3) /* alarms and other clock functions */
#define SYSTEM  ((endpoint_t) -2) /* request system functionality */
#define KERNEL  ((endpoint_t) -1) /* pseudo-process for IPC and scheduling */
#define HARDWARE     KERNEL     /* for hardware interrupt handlers */

/* Number of tasks. Note that NR_PROCS is defined in <minix/config.h>. */
#define MAX_NR_TASKS    1023
#define NR_TASKS          5

/* User-space processes, that is, device drivers, servers, and INIT. */
#define PM_PROC_NR   ((endpoint_t) 0)   /* process manager */
#define VFS_PROC_NR  ((endpoint_t) 1)   /* file system */
#define RS_PROC_NR   ((endpoint_t) 2)   /* reincarnation server */
#define MEM_PROC_NR  ((endpoint_t) 3)   /* memory driver (RAM disk, null, etc.) */
#define LOG_PROC_NR  ((endpoint_t) 4)   /* log device driver */
#define TTY_PROC_NR  ((endpoint_t) 5)   /* terminal (TTY) driver */
#define DS_PROC_NR   ((endpoint_t) 6)   /* data store server */
#define MFS_PROC_NR  ((endpoint_t) 7)   /* minix root filesystem */
#define VM_PROC_NR   ((endpoint_t) 8)   /* memory server */
#define PFS_PROC_NR  ((endpoint_t) 9)  /* pipe filesystem */
#define SCHED_PROC_NR ((endpoint_t) 10) /* scheduler */
#define LAST_SPECIAL_PROC_NR    11      /* An untyped version for
                                           computation in macros.*/
#define INIT_PROC_NR ((endpoint_t) LAST_SPECIAL_PROC_NR)  /* init
                                                        -- goes multiuser */

/* Root system process and root user process. */
#define ROOT_SYS_PROC_NR  RS_PROC_NR
#define ROOT_USR_PROC_NR  INIT_PROC_NR

/* Number of processes contained in the system image. */
#define NR_BOOT_PROCS   (NR_TASKS + LAST_SPECIAL_PROC_NR + 1)

/*===========================================================================*
 *                         Kernel notification types                         *
 *===========================================================================*/

/* Kernel notification types. In principle, these can be sent to any process,
 * so make sure that these types do not interfere with other message types.
 * Notifications are prioritized because of the way they are unhold() and
 * blocking notifications are delivered. The lowest numbers go first. The
 * offset are used for the per-process notification bit maps.
 */
#define NOTIFY_MESSAGE            0x1000
/* FIXME the old is_notify(a) should be replaced by is_ipc_notify(status). */
#define is_ipc_notify(ipc_status) (IPC_STATUS_CALL(ipc_status) == NOTIFY)
#define is_notify(a)              ((unsigned) ((a) - NOTIFY_MESSAGE) < 0x100)
#define is_ipc_asynch(ipc_status) \
    (is_ipc_notify(ipc_status) || IPC_STATUS_CALL(ipc_status) == SENDA)

/* Shorthands for message parameters passed with notifications. */
#define NOTIFY_ARG              m2_l1
#define NOTIFY_TIMESTAMP        m2_l2

/*===========================================================================*
 *                Messages for BUS controller drivers                        *
 *===========================================================================*/
#define BUSC_RQ_BASE    0x300   /* base for request types */
#define BUSC_RS_BASE    0x380   /* base for response types */

#define BUSC_PCI_INIT           (BUSC_RQ_BASE + 0)      /* First message to
                                                         * PCI driver
                                                         */
#define BUSC_PCI_FIRST_DEV      (BUSC_RQ_BASE + 1)      /* Get index (and
                                                         * vid/did) of the
                                                         * first PCI device
                                                         */
#define BUSC_PCI_NEXT_DEV       (BUSC_RQ_BASE + 2)      /* Get index (and
                                                         * vid/did) of the
                                                         * next PCI device
                                                         */
#define BUSC_PCI_FIND_DEV       (BUSC_RQ_BASE + 3)      /* Get index of a
                                                         * PCI device based on
                                                         * bus/dev/function
                                                         */
#define BUSC_PCI_IDS            (BUSC_RQ_BASE + 4)      /* Get vid/did from an
                                                         * index
#define BUSC_PCI_RESERVE        (BUSC_RQ_BASE + 7)      /* Reserve a PCI dev */
#define BUSC_PCI_ATTR_R8        (BUSC_RQ_BASE + 8)      /* Read 8-bit
                                                         * attribute value
                                                         */
#define BUSC_PCI_ATTR_R16       (BUSC_RQ_BASE + 9)      /* Read 16-bit
                                                         * attribute value
                                                         */
#define BUSC_PCI_ATTR_R32       (BUSC_RQ_BASE + 10)     /* Read 32-bit
                                                         * attribute value
                                                         */
#define BUSC_PCI_ATTR_W8        (BUSC_RQ_BASE + 11)     /* Write 8-bit
                                                         * attribute value
                                                         */
#define BUSC_PCI_ATTR_W16       (BUSC_RQ_BASE + 12)     /* Write 16-bit
                                                         * attribute value
                                                         */
#define BUSC_PCI_ATTR_W32       (BUSC_RQ_BASE + 13)     /* Write 32-bit
                                                         * attribute value
                                                         */
#define BUSC_PCI_RESCAN         (BUSC_RQ_BASE + 14)     /* Rescan bus */
#define BUSC_PCI_DEV_NAME_S     (BUSC_RQ_BASE + 15)     /* Get the name of a
                                                         * PCI device
                                                         * (safecopy)
                                                         */
#define BUSC_PCI_SLOT_NAME_S    (BUSC_RQ_BASE + 16)     /* Get the name of a
                                                         * PCI slot (safecopy)
                                                         */
#define BUSC_PCI_SET_ACL        (BUSC_RQ_BASE + 17)     /* Set the ACL for a
                                                         * driver (safecopy)
                                                         */
#define BUSC_PCI_DEL_ACL        (BUSC_RQ_BASE + 18)     /* Delete the ACL of a
                                                         * driver
                                                         */
#define BUSC_PCI_GET_BAR        (BUSC_RQ_BASE + 19)     /* Get Base Address
                                                         * Register properties
                                                         */
#define   BUSC_PGB_DEVIND       m2_i1                   /* device index */
#define   BUSC_PGB_PORT         m2_i2                   /* port (BAR offset) */
#define   BUSC_PGB_BASE         m2_l1                   /* BAR base address */
#define   BUSC_PGB_SIZE         m2_l2                   /* BAR size */
#define   BUSC_PGB_IOFLAG       m2_i1                   /* I/O space? */
#define IOMMU_MAP               (BUSC_RQ_BASE + 32)     /* Ask IOMMU to map
                                                         * a segment of memory
                                                         */

/*===========================================================================*
 *                Messages for CHARACTER device drivers                      *
 *===========================================================================*/

/* Message types for character device drivers. */
#define DEV_RQ_BASE   0x400     /* base for character device request types */
#define DEV_RS_BASE   0x500     /* base for character device response types */

#define CANCEL          (DEV_RQ_BASE +  0) /* force a task to cancel */
#define DEV_OPEN        (DEV_RQ_BASE +  6) /* open a minor device */
#define DEV_CLOSE       (DEV_RQ_BASE +  7) /* close a minor device */
#define DEV_SELECT      (DEV_RQ_BASE + 12) /* request select() attention */
#define DEV_STATUS      (DEV_RQ_BASE + 13) /* request driver status */
#define DEV_REOPEN      (DEV_RQ_BASE + 14) /* reopen a minor device */

#define DEV_READ_S      (DEV_RQ_BASE + 20) /* (safecopy) read from minor */
#define DEV_WRITE_S     (DEV_RQ_BASE + 21) /* (safecopy) write to minor */
#define DEV_SCATTER_S   (DEV_RQ_BASE + 22) /* (safecopy) write from a vector */
#define DEV_GATHER_S    (DEV_RQ_BASE + 23) /* (safecopy) read into a vector */
#define DEV_IOCTL_S     (DEV_RQ_BASE + 24) /* (safecopy) I/O control code */

#define IS_DEV_RQ(type) (((type) & ~0xff) == DEV_RQ_BASE)

#define DEV_REVIVE      (DEV_RS_BASE + 2) /* driver revives process */
#define DEV_IO_READY    (DEV_RS_BASE + 3) /* selected device ready */
#define DEV_NO_STATUS   (DEV_RS_BASE + 4) /* empty status reply */
#define DEV_REOPEN_REPL (DEV_RS_BASE + 5) /* reply to DEV_REOPEN */
#define DEV_CLOSE_REPL  (DEV_RS_BASE + 6) /* reply to DEV_CLOSE */
#define DEV_SEL_REPL1   (DEV_RS_BASE + 7) /* first reply to DEV_SELECT */
#define DEV_SEL_REPL2   (DEV_RS_BASE + 8) /* (opt) second reply to DEV_SELECT */
#define DEV_OPEN_REPL   (DEV_RS_BASE + 9) /* reply to DEV_OPEN */

#define IS_DEV_RS(type) (((type) & ~0xff) == DEV_RS_BASE)

/* Field names for messages to character device drivers. */
#define DEVICE          m2_i1   /* major-minor device */
#define USER_ENDPT      m2_i2   /* which endpoint initiated this call? */
#define COUNT           m2_i3   /* how many bytes to transfer */
#define REQUEST         m2_i3   /* ioctl request code */
#define POSITION        m2_l1   /* file offset (low 4 bytes) */
#define HIGHPOS         m2_l2   /* file offset (high 4 bytes) */
#define ADDRESS         m2_p1   /* core buffer address */
#define IO_GRANT        m2_p1   /* grant id (for DEV_*_S variants) */

/* Field names for DEV_SELECT messages to character device drivers. */
/* Field names for messages to character device drivers. */
#define DEVICE          m2_i1   /* major-minor device */
#define USER_ENDPT      m2_i2   /* which endpoint initiated this call? */
#define COUNT           m2_i3   /* how many bytes to transfer */
#define REQUEST         m2_i3   /* ioctl request code */
#define POSITION        m2_l1   /* file offset (low 4 bytes) */
#define HIGHPOS         m2_l2   /* file offset (high 4 bytes) */
#define ADDRESS         m2_p1   /* core buffer address */
#define IO_GRANT        m2_p1   /* grant id (for DEV_*_S variants) */

/* Field names for DEV_SELECT messages to character device drivers. */
#define DEV_MINOR       m2_i1   /* minor device */
#define DEV_SEL_OPS     m2_i2   /* which select operations are requested */

/* Field names used in reply messages from tasks. */
#define REP_ENDPT       m2_i1   /* # of proc on whose behalf I/O was done */
#define REP_STATUS      m2_i2   /* bytes transferred or error number */
#define REP_IO_GRANT    m2_i3   /* DEV_REVIVE: grant by which I/O was done */
#  define SUSPEND        -998   /* status to suspend caller, reply later */

/* Field names for messages to TTY driver. */
#define TTY_LINE        DEVICE  /* message parameter: terminal line */
#define TTY_REQUEST     COUNT   /* message parameter: ioctl request code */
#define TTY_SPEK        POSITION/* message parameter: ioctl speed, erasing */
#define TTY_PGRP        m2_i3   /* message parameter: process group */

/*===========================================================================*
 *                         Messages for networking layer                     *
 *===========================================================================*/

/* Base type for data link layer requests and responses. */
#define DL_RQ_BASE      0x200
#define DL_RS_BASE      0x280
/* Message types for data link layer requests. */
#define DL_CONF         (DL_RQ_BASE + 0)
#define DL_GETSTAT_S    (DL_RQ_BASE + 1)
#define DL_WRITEV_S     (DL_RQ_BASE + 2)
#define DL_READV_S      (DL_RQ_BASE + 3)

/* Message type for data link layer replies. */
#define DL_CONF_REPLY   (DL_RS_BASE + 0)
#define DL_STAT_REPLY   (DL_RS_BASE + 1)
#define DL_TASK_REPLY   (DL_RS_BASE + 2)

/* Field names for data link layer messages. */
#define DL_COUNT        m2_i3
#define DL_MODE         m2_l1
#define DL_FLAGS        m2_l1
#define DL_GRANT        m2_l2
#define DL_STAT         m3_i1
#define DL_HWADDR       m3_ca1

/* Bits in 'DL_FLAGS' field of DL replies. */
#  define DL_NOFLAGS            0x00
#  define DL_PACK_SEND          0x01
#  define DL_PACK_RECV          0x02

/* Bits in 'DL_MODE' field of DL requests. */
#  define DL_NOMODE             0x0
#  define DL_PROMISC_REQ        0x1
#  define DL_MULTI_REQ          0x2
#  define DL_BROAD_REQ          0x4

/*===========================================================================*
 *                  SYSTASK request types and field names                    *
 *===========================================================================*/

/* System library calls are dispatched via a call vector, so be careful when
 * modifying the system call numbers. The numbers here determine which call
 * is made from the call vector.
 */
#define KERNEL_CALL     0x600   /* base for kernel calls to SYSTEM */

#  define SYS_FORK       (KERNEL_CALL + 0)      /* sys_fork() */
#  define SYS_EXEC       (KERNEL_CALL + 1)      /* sys_exec() */
#  define SYS_CLEAR      (KERNEL_CALL + 2)      /* sys_clear() */
#  define SYS_SCHEDULE   (KERNEL_CALL + 3)      /* sys_schedule() */
#  define SYS_PRIVCTL    (KERNEL_CALL + 4)      /* sys_privctl() */
#  define SYS_TRACE      (KERNEL_CALL + 5)      /* sys_trace() */
#  define SYS_KILL       (KERNEL_CALL + 6)      /* sys_kill() */

#  define SYS_GETKSIG    (KERNEL_CALL + 7)      /* sys_getsig() */
#  define SYS_ENDKSIG    (KERNEL_CALL + 8)      /* sys_endsig() */
#  define SYS_SIGSEND    (KERNEL_CALL + 9)      /* sys_sigsend() */
#  define SYS_SIGRETURN  (KERNEL_CALL + 10)     /* sys_sigreturn() */

#  define SYS_NEWMAP     (KERNEL_CALL + 11)     /* sys_newmap() */
#  define SYS_MEMSET     (KERNEL_CALL + 13)     /* sys_memset() */

#  define SYS_UMAP       (KERNEL_CALL + 14)     /* sys_umap() */
#  define SYS_VIRCOPY    (KERNEL_CALL + 15)     /* sys_vircopy() */
#  define SYS_PHYSCOPY   (KERNEL_CALL + 16)     /* sys_physcopy() */
#  define SYS_UMAP_REMOTE (KERNEL_CALL + 17)    /* sys_umap_remote() */

#  define SYS_IRQCTL     (KERNEL_CALL + 19)     /* sys_irqctl() */
#  define SYS_INT86      (KERNEL_CALL + 20)     /* sys_int86() */
#  define SYS_DEVIO      (KERNEL_CALL + 21)     /* sys_devio() */
#  define SYS_SDEVIO     (KERNEL_CALL + 22)     /* sys_sdevio() */
#  define SYS_VDEVIO     (KERNEL_CALL + 23)     /* sys_vdevio() */

#  define SYS_SETALARM   (KERNEL_CALL + 24)     /* sys_setalarm() */
#  define SYS_TIMES      (KERNEL_CALL + 25)     /* sys_times() */
#  define SYS_GETINFO    (KERNEL_CALL + 26)     /* sys_getinfo() */
#  define SYS_ABORT      (KERNEL_CALL + 27)     /* sys_abort() */
#  define SYS_IOPENABLE  (KERNEL_CALL + 28)     /* sys_enable_iop() */
#  define SYS_SAFECOPYFROM (KERNEL_CALL + 31)   /* sys_safecopyfrom() */
#  define SYS_SAFECOPYTO   (KERNEL_CALL + 32)   /* sys_safecopyto() */
#  define SYS_VSAFECOPY  (KERNEL_CALL + 33)     /* sys_vsafecopy() */
#  define SYS_SETGRANT   (KERNEL_CALL + 34)     /* sys_setgrant() */
#  define SYS_READBIOS   (KERNEL_CALL + 35)     /* sys_readbios() */

#  define SYS_SPROF      (KERNEL_CALL + 36)     /* sys_sprof() */
#  define SYS_CPROF      (KERNEL_CALL + 37)     /* sys_cprof() */
#  define SYS_PROFBUF    (KERNEL_CALL + 38)     /* sys_profbuf() */

#  define SYS_STIME      (KERNEL_CALL + 39)     /* sys_stime() */

#  define SYS_VMCTL      (KERNEL_CALL + 43)     /* sys_vmctl() */
#  define SYS_SYSCTL     (KERNEL_CALL + 44)     /* sys_sysctl() */

#  define SYS_VTIMER     (KERNEL_CALL + 45)     /* sys_vtimer() */
#  define SYS_RUNCTL     (KERNEL_CALL + 46)     /* sys_runctl() */
#  define SYS_SAFEMAP    (KERNEL_CALL + 47)     /* sys_safemap() */
#  define SYS_SAFEREVMAP (KERNEL_CALL + 48)     /* sys_saferevmap() sys_saferevmap2() */
#  define SYS_SAFEUNMAP  (KERNEL_CALL + 49)     /* sys_safeunmap() */
#  define SYS_GETMCONTEXT (KERNEL_CALL + 50)    /* sys_getmcontext() */
#  define SYS_SETMCONTEXT (KERNEL_CALL + 51)    /* sys_setmcontext() */

#  define SYS_UPDATE     (KERNEL_CALL + 52)     /* sys_update() */
#  define SYS_EXIT       (KERNEL_CALL + 53)     /* sys_exit() */

#  define SYS_SCHEDCTL (KERNEL_CALL + 54)       /* sys_schedctl() */
#  define SYS_STATECTL (KERNEL_CALL + 55)       /* sys_statectl() */

/* Total */
#define NR_SYS_CALLS    56      /* number of kernel calls */

#define SYS_CALL_MASK_SIZE BITMAP_CHUNKS(NR_SYS_CALLS)

/* Basic kernel calls allowed to every system process. */
#define SYS_BASIC_CALLS \
    SYS_EXIT, SYS_SAFECOPYFROM, SYS_SAFECOPYTO, SYS_VSAFECOPY, SYS_GETINFO, \
    SYS_TIMES, SYS_SETALARM, SYS_SETGRANT, SYS_SAFEMAP, SYS_SAFEREVMAP, \
    SYS_SAFEUNMAP, SYS_PROFBUF, SYS_SYSCTL, SYS_STATECTL

/* Field names for SYS_MEMSET. */
#define MEM_PTR         m2_p1   /* base */
#define MEM_COUNT       m2_l1   /* count */
#define MEM_PATTERN     m2_l2   /* pattern to write */

/* Field names for SYS_DEVIO, SYS_VDEVIO, SYS_SDEVIO. */
#define DIO_REQUEST     m2_i3   /* device in or output */
#   define _DIO_INPUT           0x001
#   define _DIO_OUTPUT          0x002
#   define _DIO_DIRMASK         0x00f
#   define _DIO_BYTE            0x010
#   define _DIO_WORD            0x020
#   define _DIO_LONG            0x030
#   define _DIO_TYPEMASK        0x0f0
#   define _DIO_SAFE            0x100
#   define _DIO_SAFEMASK        0xf00
#   define DIO_INPUT_BYTE           (_DIO_INPUT|_DIO_BYTE)
#   define DIO_INPUT_WORD           (_DIO_INPUT|_DIO_WORD)
#   define DIO_INPUT_LONG           (_DIO_INPUT|_DIO_LONG)
#   define DIO_OUTPUT_BYTE          (_DIO_OUTPUT|_DIO_BYTE)
#   define DIO_OUTPUT_WORD          (_DIO_OUTPUT|_DIO_WORD)
#   define DIO_OUTPUT_LONG          (_DIO_OUTPUT|_DIO_LONG)
#   define DIO_SAFE_INPUT_BYTE      (_DIO_INPUT|_DIO_BYTE|_DIO_SAFE)
#   define DIO_SAFE_INPUT_WORD      (_DIO_INPUT|_DIO_WORD|_DIO_SAFE)
#   define DIO_SAFE_INPUT_LONG      (_DIO_INPUT|_DIO_LONG|_DIO_SAFE)
#   define DIO_SAFE_OUTPUT_BYTE     (_DIO_OUTPUT|_DIO_BYTE|_DIO_SAFE)
#   define DIO_SAFE_OUTPUT_WORD     (_DIO_OUTPUT|_DIO_WORD|_DIO_SAFE)
#   define DIO_SAFE_OUTPUT_LONG     (_DIO_OUTPUT|_DIO_LONG|_DIO_SAFE)
#define DIO_PORT        m2_l1   /* single port address */
#define DIO_VALUE       m2_l2   /* single I/O value */
#define DIO_VEC_ADDR    m2_p1   /* address of buffer or (p,v)-pairs */
#define DIO_VEC_SIZE    m2_l2   /* number of elements in vector */
#define DIO_VEC_ENDPT   m2_i2   /* number of process where vector is */
#define DIO_OFFSET      m2_i1   /* offset from grant */

/* Field names for SYS_SETALARM. */
#define ALRM_EXP_TIME   m2_l1   /* expire time for the alarm call */
#define ALRM_ABS_TIME   m2_i2   /* set to 1 to use absolute alarm time */
#define ALRM_TIME_LEFT  m2_l1   /* how many ticks were remaining */

/* Field names for SYS_IRQCTL. */
#define IRQ_REQUEST     m5_s1   /* what to do? */
#  define IRQ_SETPOLICY     1   /* manage a slot of the IRQ table */
#  define IRQ_RMPOLICY      2   /* remove a slot of the IRQ table */
#  define IRQ_ENABLE        3   /* enable interrupts */
#  define IRQ_DISABLE       4   /* disable interrupts */
#define IRQ_VECTOR      m5_s2   /* irq vector */
#define IRQ_POLICY      m5_i1   /* options for IRQCTL request */
#  define IRQ_REENABLE  0x001   /* reenable IRQ line after interrupt */
#  define IRQ_BYTE      0x100   /* byte values */
#  define IRQ_WORD      0x200   /* word values */
    #  define IRQ_LONG      0x400       /* long values */
#define IRQ_HOOK_ID     m5_l3   /* id of irq hook at kernel */

/* Field names for SYS_ABORT. */
#define ABRT_HOW        m1_i1   /* RBT_REBOOT, RBT_HALT, etc. */
#define ABRT_MON_ENDPT  m1_i2   /* process where monitor params are */
#define ABRT_MON_LEN    m1_i3   /* length of monitor params */
#define ABRT_MON_ADDR   m1_p1   /* virtual address of monitor params */

/* Field names for SYS_IOPENABLE. */
#define IOP_ENDPT       m2_l1   /* target endpoint */

/* Field names for _UMAP, _VIRCOPY, _PHYSCOPY. */
#define CP_SRC_SPACE    m5_s1   /* T or D space (stack is also D) */
#define CP_SRC_ENDPT    m5_i1   /* process to copy from */
#define CP_SRC_ADDR     m5_l1   /* address where data come from */
#define CP_DST_SPACE    m5_s2   /* T or D space (stack is also D) */
#define CP_DST_ENDPT    m5_i2   /* process to copy to */
#define CP_DST_ADDR     m5_l2   /* address where data go to */
#define CP_NR_BYTES     m5_l3   /* number of bytes to copy */

/* Field names for SYS_GETINFO. */
#define I_REQUEST      m7_i3    /* what info to get */
#   define GET_KINFO       0    /* get kernel information structure */
#   define GET_IMAGE       1    /* get system image table */
#   define GET_PROCTAB     2    /* get kernel process table */
#   define GET_RANDOMNESS  3    /* get randomness buffer */
#   define GET_MONPARAMS   4    /* get monitor parameters */
#   define GET_KENV        5    /* get kernel environment string */
#   define GET_IRQHOOKS    6    /* get the IRQ table */
#   define GET_KMESSAGES   7    /* get kernel messages */
#   define GET_PRIVTAB     8    /* get kernel privileges table */
#   define GET_KADDRESSES  9    /* get various kernel addresses */
#   define GET_SCHEDINFO  10    /* get scheduling queues */
#   define GET_PROC       11    /* get process slot if given process */
#   define GET_MACHINE    12    /* get machine information */
#   define GET_LOCKTIMING 13    /* get lock()/unlock() latency timing */
#   define GET_BIOSBUFFER 14    /* get a buffer for BIOS calls */
#   define GET_LOADINFO   15    /* get load average information */
#   define GET_IRQACTIDS  16    /* get the IRQ masks */
#   define GET_PRIV       17    /* get privilege structure */
#   define GET_HZ         18    /* get HZ value */
#   define GET_WHOAMI     19    /* get own name, endpoint, and privileges */
#   define GET_RANDOMNESS_BIN 20 /* get one randomness bin */

#   define GET_IDLETSC    21    /* get cumulative idle time stamp counter */
#if !defined(__ELF__)
#   define GET_AOUTHEADER 22    /* get a.out headers from the boot image */
#endif
#   define GET_CPUINFO    23    /* get information about cpus */
#   define GET_REGS       24    /* get general process registers */
#define I_ENDPT        m7_i4    /* calling process (may only be SELF) */
#define I_VAL_PTR      m7_p1    /* virtual address at caller */
#define I_VAL_LEN      m7_i1    /* max length of value */
#define I_VAL_PTR2     m7_p2    /* second virtual address */
#define I_VAL_LEN2_E   m7_i2    /* second length, or proc nr */

/* GET_WHOAMI fields. */
#define GIWHO_EP        m3_i1
#define GIWHO_NAME      m3_ca1
#define GIWHO_PRIVFLAGS m3_i2

/* Field names for SYS_TIMES. */
#define T_ENDPT         m4_l1   /* process to request time info for */
#define T_USER_TIME     m4_l1   /* user time consumed by process */
#define T_SYSTEM_TIME   m4_l2   /* system time consumed by process */
#define T_BOOTTIME      m4_l3   /* Boottime in seconds (also for SYS_STIME) */
#define T_BOOT_TICKS    m4_l5   /* number of clock ticks since boot time */
/* Field names for SYS_TRACE, SYS_PRIVCTL, SYS_STATECTL. */
#define CTL_ENDPT      m2_i1    /* process number of the caller */
#define CTL_REQUEST    m2_i2    /* server control request */
#define CTL_ARG_PTR    m2_p1    /* pointer to argument */
#define CTL_ADDRESS    m2_l1    /* address at traced process' space */
#define CTL_DATA       m2_l2    /* data field for tracing */

/* SYS_PRIVCTL with CTL_REQUEST == SYS_PRIV_QUERY_MEM */
#define CTL_PHYSSTART  m2_l1    /* physical memory start in bytes*/
#define CTL_PHYSLEN    m2_l2    /* length in bytes */

/* Subfunctions for SYS_PRIVCTL */
#define SYS_PRIV_ALLOW          1       /* Allow process to run */
#define SYS_PRIV_DISALLOW       2       /* Disallow process to run */
#define SYS_PRIV_SET_SYS        3       /* Set a system privilege structure */
#define SYS_PRIV_SET_USER       4       /* Set a user privilege structure */
#define SYS_PRIV_ADD_IO         5       /* Add I/O range (struct io_range) */
#define SYS_PRIV_ADD_MEM        6       /* Add memory range (struct mem_range)
                                         */
#define SYS_PRIV_ADD_IRQ        7       /* Add IRQ */
#define SYS_PRIV_QUERY_MEM      8       /* Verify memory privilege. */
#define SYS_PRIV_UPDATE_SYS     9       /* Update a sys privilege structure. */
#define SYS_PRIV_YIELD         10       /* Allow process to run and suspend */

/* Field names for SYS_SETGRANT */
#define SG_ADDR         m2_p1   /* address */
#define SG_SIZE         m2_i2   /* no. of entries */

/* Field names for SYS_GETKSIG, _ENDKSIG, _KILL, _SIGSEND, _SIGRETURN. */
#define SIG_ENDPT      m2_i1    /* process number for inform */
#define SIG_NUMBER     m2_i2    /* signal number to send */
#define SIG_FLAGS      m2_i3    /* signal flags field */
#define SIG_MAP        m2_l1    /* used by kernel to pass signal bit map */
#define SIG_CTXT_PTR   m2_p1    /* pointer to info to restore signal context */

/* Field names for SYS_FORK, _EXEC, _EXIT, _NEWMAP, GETMCONTEXT, SETMCONTEXT.*/
#define PR_ENDPT        m1_i1   /* indicates a process */
#define PR_PRIORITY     m1_i2   /* process priority */
#define PR_SLOT         m1_i2   /* indicates a process slot */
#define PR_STACK_PTR    m1_p1   /* used for stack ptr in sys_exec, sys_getsp */
#define PR_NAME_PTR     m1_p2   /* tells where program name is for dmp */
#define PR_IP_PTR       m1_p3   /* initial value for ip after exec */
#define PR_MEM_PTR      m1_p1   /* tells where memory map is for sys_newmap
                                 * and sys_fork
                                 */
/* Flags for PR_FORK_FLAGS. */
#define PFF_VMINHIBIT   0x01    /* Don't schedule until release by VM. */

/* Field names for SYS_INT86 */
#define INT86_REG86    m1_p1    /* pointer to registers */

/* Field names for SYS_SAFECOPY* */
#define SCP_FROM_TO     m2_i1   /* from/to whom? */
#define SCP_SEG         m2_i2   /* my own segment */
#define SCP_GID         m2_i3   /* grant id */
#define SCP_OFFSET      m2_l1   /* offset within grant */
#define SCP_ADDRESS     m2_p1   /* my own address */
#define SCP_BYTES       m2_l2   /* bytes from offset */

/* Field names for SYS_VSAFECOPY* */
#define VSCP_VEC_ADDR   m2_p1   /* start of vector */
#define VSCP_VEC_SIZE   m2_l2   /* elements in vector */

/* Field names for SYS_SAFEMAPs */
#define SMAP_EP         m2_i1
#define SMAP_GID        m2_i2
#define SMAP_OFFSET     m2_i3
#define SMAP_SEG        m2_p1
#define SMAP_ADDRESS    m2_l1
#define SMAP_BYTES      m2_l2
#define SMAP_FLAG       m2_s1

/* Field names for SYS_SPROF, _CPROF, _PROFBUF. */
#define PROF_ACTION    m7_i1    /* start/stop/reset/get */
#define PROF_MEM_SIZE  m7_i2    /* available memory for data */
#define PROF_FREQ      m7_i3    /* sample frequency */
#define PROF_ENDPT     m7_i4    /* endpoint of caller */
#define PROF_INTR_TYPE m7_i5    /* interrupt type */
#define PROF_CTL_PTR   m7_p1    /* location of info struct */
#define PROF_MEM_PTR   m7_p2    /* location of profiling data */

/* Field names for SYS_READBIOS. */
#define RDB_SIZE        m2_i1
#define RDB_ADDR        m2_l1
#define RDB_BUF         m2_p1

/* Field names for SYS_VMCTL. */
#define SVMCTL_WHO      m1_i1
#define SVMCTL_PARAM    m1_i2   /* All SYS_VMCTL requests. */
#define SVMCTL_VALUE    m1_i3
#define SVMCTL_MRG_TARGET       m2_i1   /* MEMREQ_GET reply: target process */
#define SVMCTL_MRG_ADDR         m2_i2   /* MEMREQ_GET reply: address */
#define SVMCTL_MRG_LENGTH       m2_i3   /* MEMREQ_GET reply: length */
#define SVMCTL_MRG_FLAG         m2_s1   /* MEMREQ_GET reply: flag */
#define SVMCTL_MRG_EP2          m2_l1   /* MEMREQ_GET reply: source process */
#define SVMCTL_MRG_ADDR2        m2_l2   /* MEMREQ_GET reply: source address */
#define SVMCTL_MRG_REQUESTOR    m2_p1   /* MEMREQ_GET reply: requestor */
#define SVMCTL_MAP_VIR_ADDR     m1_p1
#define SVMCTL_PTROOT           m1_i3
#define SVMCTL_PTROOT_V         m1_p1

/* Reply message for VMCTL_KERN_PHYSMAP */
#define SVMCTL_MAP_FLAGS        m2_i1   /* VMMF_* */
#define SVMCTL_MAP_PHYS_ADDR    m2_l1
#define SVMCTL_MAP_PHYS_LEN     m2_l2

#define VMMF_UNCACHED           (1L << 0)

/* Values for SVMCTL_PARAM. */
#define VMCTL_CLEAR_PAGEFAULT   12













