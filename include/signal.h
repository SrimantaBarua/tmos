// (C) 2018 Srimanta Barua

#pragma once

#include <sys/types.h>
#include <sys/machine.h>


// Represents sets of signals
typedef unsigned long sigset_t;

// Atomic entity type
typedef int sig_atomic_t;


// For use with sigprocmask()
#define SIG_SETMASK 1  // Set the current mask to the arg
#define SIG_BLOCK   2  // OR the arg to the current mask
#define SIG_UNBLOCK 3  // AND the (NOT of the arg) to the current mask


// sigev_motify values
#define SIGEV_NONE   1 // No async notif. shall be delivered
#define SIGEV_SIGNAL 2 // Queued signal, with app defined value
#define SIGEV_THREAD 3 // Notification fn. shall be called


// sa_flags bitmask (struct sigaction)
#define SA_NOCLDSTOP  1  // Do not generate SIGCHLD
#define SA_ONSTACK    2  // Causes signal delivery on an alternate stack.
#define SA_RESETHAND  4  // Causes signal dispositions to be set to SIG_DFL on entry to signal handlers.
#define SA_RESTART    8  // Causes certain functions to become restartable.
#define SA_SIGINFO    16 // Causes extra information to be passed to signal handlers at the time of receipt of a signal.
#define SA_NOCLDWAIT  32 // Causes implementations not to create zombie processes on child death.
#define SA_NODEFER    64 // Causes signal not to be automatically blocked on entry to signal handler.


// stack_t.ss_flags
#define SS_ONSTACK   1 // Process is executing on an alternate signal stack.
#define SS_DISABLE   2 // Alternate signal stack is disabled.


// Signal stack size
#define MINSIGSTKSZ  PAGE_SIZE  // Minimum stack size for a signal handler.
#define SIGSTKSZ     PAGE_SIZE  // Default size in bytes for the alternate signal stack.


// Signal value type
union sigval {
	int sigval_int;   // Integer value
	void *sigval_ptr; // Pointer value
};


// Signal event
struct sigevent {
	int            sigev_notify;                  // Notification type
	int            sigev_signo;                   // Signal number
	union sigval   sigev_value;                   // Signal value
	pthread_attr_t *sigev_notify_attributes;      // Notification attribs
	void (*sigev_notify_function) (union sigval); // Notification function
};


typedef struct {
	int          si_signo;  // Signal number
	int          si_code;   // Signal code
	int          si_errno;  // If non-zero, ref. errno.h
	pid_t        si_pid;    // Sending process ID
	uid_t        si_uid;    // Real user ID of sending process
	void         *si_addr;  // Address of faulting instruction
	int          si_status; // Exit value or signal
	long         si_band;   // Band event for SIGPOLL
	union sigval si_value;  // Signal value
} siginfo_t;


// Denote the action taken by a process on receipt of a signal
struct sigaction {
	sigset_t sa_mask;      // Set of signals to be blocked during handler
	int      sa_flags;     // Special flags
	union {
		// Pointer to signal handling fn., or one of SIG_IGN/SIG_DFL
		void (*_handler) (int);
		// Pointer to a signal handling function
		void (*_sigaction) (int, siginfo_t*, void*);
	} _signal_handler;
};
#define sa_handler   _signal_handler._handler
#define sa_sigaction _signal_handler._sigaction


// Reasons for signal generation (si_code)
// SIGILL
#define ILL_ILLOPC 6  // Illegal opcode
#define ILL_ILLOPN 7  // Illegal operand
#define ILL_ILLADR 8  // Illegal addressing mode
#define ILL_ILLTRP 9  // Illegal trap
#define ILL_PRVOPC 10 // Privileged opcode
#define ILL_PRVREG 11 // Privileged register
#define ILL_COPROC 12 // Coprocessor error
#define ILL_BADSTK 13 // Internal stack error
// SIGFPE
#define FPE_INTDIV 14 // Integer divide by zero
#define FPE_INTOVF 15 // Integer overflow
#define FPE_FLTDIV 16 // Floating point divide by zero
#define FPE_FLTOVF 17 // Floating point overflow
#define FPE_FLTUND 18 // Floating point underflow
#define FPE_FLTRES 19 // Floating point inexact result
#define FPE_FLTINV 20 // Invalid floating point operation
#define FPE_FLTSUB 21 // Subscript out of range
// SIGSEGV
#define SEGV_MAPERR 22 // Address not mapped to object
#define SEGV_ACCERR 23 // Invalid permissions for mapped object
// SIGBUS
#define BUS_ADRALN 24 // Invalid address alignment
#define BUS_ADRERR 25 // Nonexistant physical address
#define BUS_OBJERR 26 // Object-specific hardware error
// SIGTRAP
#define TRAP_BRKPT 27 // Process breakpoint
#define TRAP_TRACE 28 // Process trace point
// SIGCHLD
#define CLD_EXITED    29 // Child has exited
#define CLD_KILLED    30 // Child has terminated abnormally. No core file
#define CLD_DUMPED    31 // Child has terminated abnormally. Core file generated
#define CLD_TRAPPED   32 // Traced child has trapped
#define CLD_STOPPED   33 // Child has stopped
#define CLD_CONTINUED 34 // Stopped child has continued
// SIGPOLL
#define POLL_IN  35 // Data input available
#define POLL_OUT 36 // Output buffers available
#define POLL_MSG 37 // Input message available
#define POLL_ERR 38 // I/O error
#define POLL_PRI 39 // High priority input available
#define POLL_HUP 40 // Device disconnected
// Any
#define SI_USER    1 // Signal sent by kill()
#define SI_QUEUE   2 // Signal sent by sigqueue()
#define SI_TIMER   3 // Timer expiration (timer_setttime())
#define SI_ASYNCIO 4 // Completion of async I/O
#define SI_MESGQ   5 // Arrival of message on empty message queue


// Functions

// Send a signal to a process or a group of processes
int kill(pid_t pid, int sig);

// Examine and change a signal action
int sigaction(int sig, const struct sigaction *act, struct sigaction *oldact);

// Add sig to set
int sigaddset(sigset_t *set, int sig);

// Remove sig from set
int sigdelset(sigset_t *set, int sig);

// Remove all signals from set
int sigemptyset(sigset_t *set);

// Add all signals to set
int sigfillset(sigset_t *set);

// Fetch/change the signal mask of the calling thread
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
