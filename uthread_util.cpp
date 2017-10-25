#include <stdlib.h>
#include "uthread_util.h"
#include "uthread_core.h"

extern UthreadCore* g_threads;
extern sigset_t set;
extern sigset_t waitingSet;

// start timer
struct itimerval timer;
struct sigaction sa;

// resets the timer


/**
 * Initializes the timer (value and intervals), also set's the sigction obj handler to
 * our switch context function
 * @param quantum_usecs - the quantum in usecs to apply
 */
void init_timer(int quantum_usecs) {

    sa.sa_handler = &switch_context;

    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        std::cerr << "system error: Sigaction error\n";
        exit(1);
    }

    int sec = quantum_usecs / 1000000;
    int usec = quantum_usecs % 1000000;

    // Configure the timer to expire after 1 sec... */
    timer.it_value.tv_sec = sec;		// first time interval, seconds part
    timer.it_value.tv_usec = usec;		// first time interval, microseconds part

    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = sec;	// following time intervals, seconds part
    timer.it_interval.tv_usec = usec;	// following time intervals, microseconds part
}

/**
 * Resets the timer
 * @return - 0 if success, -1 otherwise
 */
int reset_timer() {
    MASK_ALARMS

    if (setitimer(ITIMER_VIRTUAL, &timer, NULL)) {
        std::cerr << "system error: Setitimer error\n";
        exit(1);
    }

    UNMASK_ALARMS
    return 0;
}

/**
 * Switches context in case quanta was finished
 * @param sig - the id of signal
 */
void switch_context(int sig) {
    if (sig != SIGVTALRM) {
        return;
    }
    switch_context_with_blocking(false, false, true);
}

/**
 * Switches context in case its because quantum ran out, or because a thread was blocked
 * @param is_blocking - true if context switching due to blocking
 * @param need_reset - true if the quanta timer for running thread needs to be reset
 * @param should_handle_last - flag if need to handle the thread which just finished running
 */
void switch_context_with_blocking(bool is_blocking, bool need_reset, bool should_handle_last=true) {
    sigjmp_buf* buff = g_threads->get_current_env();
    int ret_val = sigsetjmp(*buff, 1);

    if (ret_val == 1) {
        UNMASK_ALARMS
        return;
    }

    int sig;

    buff = g_threads->get_new_env(is_blocking, should_handle_last);
    if (need_reset) {
        reset_timer();
        // clears pending SIGVALRM
        sigemptyset(&waitingSet);
        if (sigpending(&waitingSet)) {
            std::cerr << "system error: sig pending error\n";
            exit(1);
        }
        int member = sigismember(&waitingSet, SIGVTALRM);
        if (member < 0) {
            std::cerr << "system error: sig member error\n";
            exit(1);
        } else if (member) {
            if (sigwait(&waitingSet, &sig)) {
                std::cerr << "system error: sig member error\n";
                exit(1);
            }
        }
    }

    UNMASK_ALARMS
    siglongjmp(*buff, 1);
}

#ifdef __x86_64__ /* code for 64 bit Intel arch */

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
            "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else /* code for 32 bit Intel arch */

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif