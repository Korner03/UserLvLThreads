#include <stdlib.h>
#include "uthread_node.h"
#include "uthreads.h"


/**
 * default Constructor - for Main thread
 * @return
 */
UthreadNode::UthreadNode() {
    this->_sp_p = nullptr;
    this->_sp = 0;
    this->_pc = 0;
    this->_quantums_counter = 1;
    this->is_blocked = false;
    this->is_synced = false;
    this->_f = nullptr;

    sigsetjmp(_env, 1);
    (_env->__jmpbuf)[JB_SP] = translate_address(_sp);
    (_env->__jmpbuf)[JB_PC] = translate_address(_pc);
    sigemptyset(&_env->__saved_mask);
}

/**
 * Constructor - for spawning new threads
 * @param f - the function which thread runs
 * @return
 */
UthreadNode::UthreadNode(void (*f)(void)) {
    try {
        this->_sp_p = new char[STACK_SIZE];
    } catch (std::bad_alloc& exc) {
        std::cerr << "system error: Bad alloc\n";
        exit(1);
    }
    this->_sp = (address_t) _sp_p + STACK_SIZE - sizeof(address_t);
    this->_pc = (address_t)f;
    this->_quantums_counter = 0;
    this->_f = f;

    sigsetjmp(_env, 1);
    (_env->__jmpbuf)[JB_SP] = translate_address(_sp);
    (_env->__jmpbuf)[JB_PC] = translate_address(_pc);
    sigemptyset(&_env->__saved_mask);
}


/**
 * Destructor
 */
UthreadNode::~UthreadNode() {
    delete[] this->_sp_p;
    this->_sp_p = nullptr;
}

/**
 * returns the num of quantums counted in this thread
 * @return the num of quantums (int)
 */
int UthreadNode::get_num_of_quantums() {
    return this->_quantums_counter;
}

/**
 * returns the env of current thread
 * @return the env (sigjmp_buff*)
 */
sigjmp_buf* UthreadNode::get_env() {
    return &_env;
}

/**
 * Increases the quantum count by 1 of this thread
 */
void UthreadNode::increment_quant() {
    this->_quantums_counter++;
}
