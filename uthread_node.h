//
// Created by cabby333 on 4/19/17.
//

#ifndef EX2_UTHREAD_NODE_H
#define EX2_UTHREAD_NODE_H

#include <iostream>
#include <setjmp.h>
#include <signal.h>
#include "uthread_util.h"
#include <list>

class UthreadNode
{
public:

    /**
    * default Constructor - for Main thread
    * @return
    */
    UthreadNode();

    /**
    * Constructor - for spawning new threads
    * @param f - the function which thread runs
    * @return
    */
    UthreadNode(void (*f)(void));

    /**
    * Destructor
    */
    ~UthreadNode();

    /**
    * returns the num of quantums counted in this thread
    * @return the num of quantums (int)
    */
    int get_num_of_quantums();

    /**
    * returns the env of current thread
    * @return the env (sigjmp_buff*)
    */
    sigjmp_buf* get_env();

    /**
    * Increases the quantum count by 1 of this thread
    */
    void increment_quant();

    std::list<int> sync_waiters;
    bool is_blocked;
    bool is_synced;

private:
    int _quantums_counter;
    address_t _pc, _sp;
    sigjmp_buf _env;
    char* _sp_p;
    void (*_f)(void);
};

#endif //EX2_UTHREAD_NODE_H
