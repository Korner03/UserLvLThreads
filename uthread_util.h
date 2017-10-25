
#ifndef EX2_UTHREAD_UTIL_H
#define EX2_UTHREAD_UTIL_H

#ifdef __x86_64__ /* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr);

#else /* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr);

#endif

#define MASK_ALARMS sigprocmask(SIG_BLOCK, &set, NULL);\

#define UNMASK_ALARMS sigprocmask(SIG_UNBLOCK, &set, NULL);

/**
 * Initializes the timer (value and intervals), also set's the sigction obj handler to
 * our switch context function
 * @param quantum_usecs - the quantum in usecs to apply
 */
void init_timer(int quantum_usecs);

/**
 * Resets the timer
 * @return - 0 if success, -1 otherwise
 */
int reset_timer();

/**
 * Switches context in case quanta was finished
 * @param sig - the id of signal
 */
void switch_context(int sig);

/**
 * Switches context in case its because quantum ran out, or because a thread was blocked
 * @param is_blocking - true if context switching due to blocking
 * @param need_reset - true if the quanta timer for running thread needs to be reset
 * @param should_handle_last - flag if need to handle the thread which just finished running
 */
void switch_context_with_blocking(bool is_blocking, bool need_reset, bool should_handle_last);

#endif
