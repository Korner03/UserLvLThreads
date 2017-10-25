//
// Created by cabby333 on 4/19/17.
//

#ifndef EX2_UTHREAD_CORE_H
#define EX2_UTHREAD_CORE_H

#include <iostream>
#include <list>
#include <queue>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "uthread_node.h"
#include "uthreads.h"
#include "uthread_util.h"


// the class which handles a set of threads
class UthreadCore
{
public:

    /**
    * Constructor
    * @param quantum_usecs
    * @return
    */
    UthreadCore(int quantum_usecs);

    /**
    * Destructor
    */
    ~UthreadCore();

    /**
    * Creates a new Thread (node)
    * @param f - the new thread
    * @return id (int) if success, -1 otherwise
    */
    int spawn(void (*f)(void));

    /**
    * removes a thread and clears it's connected resources
    * @param tid - the id of thread to remove
    * @return 0 if success, -1 otherwise
    */
    int remove_node(int tid);

    /**
    * Switches a thread to blocking mode
    * @param tid - the id of the thread to block
    * @return 0 if success, -1 otherwise
    */
    int block_node(int tid);

    /**
    * Resumes a thread which was blocked, handles cases of synced by simply turning off the sync and
    * not moving it to ready
    * @param tid - the id of the thread
    * @return
    */
    int resume_node(int tid);

    /**
    * Synces the running thread to a given thread
    * @param tid - of the thread which we sync the running to
    * @return 0 if success, false otherwise
    */
    int sync_node(int tid);

    /**
    * @return the id of the running node
    */
    int get_running_node_id();

    /**
    * @return the total amount of quantums so far
    */
    int get_total_num_of_quantums();

    /**
    * returns the num a quantums of a given thread
    * @param tid of the thread
    * @return the num of quantums (int)
    */
    int get_node_num_of_quantums(int tid);

    /**
    * returns the enviroment of the current thread
    * @return  a pointer to the env of current thread
    */
    sigjmp_buf* get_current_env();

    /**
    * gets the enviroment of the new thread to context switch to, also handles synced threads of the
    * node which finished his quantum
    * @param is_blocking - flag if thread is blocked
    * @param should_handle_last - flag if need to handle the thread which just finished running
    * @return the enviroment of the thread we need to switch to
    */
    sigjmp_buf* get_new_env(bool is_blocking, bool should_handle_last);

    /**
    * increases the quantum counter by 1
    * @param tid - id of the thread to increase
    */
    void increment_tid_quant_counter(int tid);

    /**
    * checks if we still have a thread waiting to work
    * @return true if not empty, false otherwise
    */
    bool is_ready();


private:
    int _quantum_usecs;
    int _total_quantums_counter;
    UthreadNode* _id_array[MAX_THREAD_NUM];
    std::list<int> _ready;
    std::list<int> _blocking;
    int _running;
};

#endif //EX2_UTHREAD_CORE_H
