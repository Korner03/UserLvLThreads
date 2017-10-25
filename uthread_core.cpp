#include <stdlib.h>
#include "uthread_core.h"

/**
 * Constructor
 * @param quantum_usecs
 * @return
 */
UthreadCore::UthreadCore(int quantum_usecs) {
    this->_quantum_usecs = quantum_usecs;
    this->_running = 0;
    this->_total_quantums_counter = 1;
    this->_id_array[0] = new UthreadNode();

    for (int i = 1; i < MAX_THREAD_NUM; ++i) {
        this->_id_array[i] = nullptr;
    }
}

/**
 * Destructor
 */
UthreadCore::~UthreadCore() {
    for (int i = 0; i < MAX_THREAD_NUM; ++i) {
        if (this->_id_array[i] != nullptr) {
            delete (this->_id_array[i]);
            this->_id_array[i] = nullptr;
        }
    }
}

/**
 * Creates a new Thread (node)
 * @param f - the new thread
 * @return id (int) if success, -1 otherwise
 */
int UthreadCore::spawn(void (*f)(void)) {

    for (int i = 1; i < MAX_THREAD_NUM; ++i) {
        if (this->_id_array[i] == nullptr) {
            try {
                this->_id_array[i] = new UthreadNode(f);
            } catch (std::bad_alloc& exc) {
                std::cerr << "system error: Bad alloc\n";
                exit(1);
            }

            this->_ready.push_back(i);
            return i;
        }
    }
    return -1;
}

/**
 * removes a thread and clears it's connected resources
 * @param tid - the id of thread to remove
 * @return 0 if success, -1 otherwise
 */
int UthreadCore::remove_node(int tid) {
    if (tid <= 0 || tid >= MAX_THREAD_NUM || _id_array[tid] == nullptr) {
        return -1;
    }

    // moving the waiters from blocking to ready, without blocked threads.

    for (auto it = this->_id_array[tid]->sync_waiters.begin(); it != this->_id_array[tid]->sync_waiters.end(); ++it) {

        this->_id_array[*it]->is_synced = false;

        if (!this->_id_array[*it]->is_blocked) {
            this->_ready.push_front(*it);
            this->_blocking.remove(*it);
        }

    }


    // remove the thread from both ready and blocking.
    this->_ready.remove(tid);
    this->_blocking.remove(tid);

    // switch the running thread to blocking.
    if (_running == tid) {
        switch_context_with_blocking(true, false, false);
    } else {
        // remove thread.
        delete (this->_id_array[tid]);
        this->_id_array[tid] = nullptr;
    }
    return 0;
}

/**
 * Switches a thread to blocking mode
 * @param tid - the id of the thread to block
 * @return 0 if success, -1 otherwise
 */
int UthreadCore::block_node(int tid) {
    if (this->_id_array[tid] == nullptr) {
        return -1;
    }

    // handles case if blocking the node which is currently running
    if (this->_running == tid) {
        reset_timer();
        switch_context_with_blocking(true, true, true);
    } else {
        this->_ready.remove(tid);
    }

    _id_array[tid]->is_blocked = true;

    // checks that we arent blocking a blocked thread
    for (std::list<int>::iterator it = this->_blocking.begin(); it != this->_blocking.end(); ++it){
        if (*it == tid) {
            return 0;
        }
    }

    this->_blocking.push_back(tid);
    return 0;
}

/**
 * Resumes a thread which was blocked, handles cases of synced by simply turning off the sync and
 * not moving it to ready
 * @param tid - the id of the thread
 * @return
 */
int UthreadCore::resume_node(int tid) {
    if (this->_id_array[tid] == nullptr) {
        return -1;
    }

    // checks that the thread isnt running
    if (tid == _running) {
        return 0;
    }
    // check that the thread isn't alreayd ready
    for (std::list<int>::iterator it = this->_ready.begin(); it != this->_ready.end(); ++it) {
        if (*it == tid) {
            return 0;
        }
    }


    // if synced, just mark as not blocked anymore.
    if (_id_array[tid]->is_synced) {
        _id_array[tid]->is_blocked = false;
        return 0;
    }

    this->_blocking.remove(tid);
    this->_ready.push_back(tid);
    this->_id_array[tid]->is_blocked = false;

    return 0;
}

/**
 * @return the id of the running node
 */
int UthreadCore::get_running_node_id() {
    return this->_running;
}

/**
 * @return the total amount of quantums so far
 */
int UthreadCore::get_total_num_of_quantums() {
    return this->_total_quantums_counter;
}

/**
 * returns the num a quantums of a given thread
 * @param tid of the thread
 * @return the num of quantums (int)
 */
int UthreadCore::get_node_num_of_quantums(int tid) {
    if (tid < 0 || tid >= MAX_THREAD_NUM || this->_id_array[tid] == nullptr) {
        return -1;
    }
    return this->_id_array[tid]->get_num_of_quantums();
}

/**
 * returns the enviroment of the current thread
 * @return  a pointer to the env of current thread
 */
sigjmp_buf* UthreadCore::get_current_env()
{
    return this->_id_array[_running]->get_env();
}

/**
 * gets the enviroment of the new thread to context switch to, also handles synced threads of the
 * node which finished his quantum
 * @param is_blocking - flag if thread is blocked
 * @param should_handle_last - flag if need to handle the thread which just finished running
 * @return the enviroment of the thread we need to switch to
 */
sigjmp_buf* UthreadCore::get_new_env(bool is_blocking, bool should_handle_last=true)
{

    int traveler;
    // handles the synced list of the running thread
    while (!_id_array[_running]->sync_waiters.empty()) {
        traveler = _id_array[_running]->sync_waiters.front();
        _id_array[traveler]->is_synced = false;
        if (!_id_array[traveler]->is_blocked) {
            this->_ready.push_front(traveler);
            this->_blocking.remove(traveler);
        }
        _id_array[_running]->sync_waiters.pop_front();
    }
    

    // move the running thread to the right queue.
    if (should_handle_last) {
        if (is_blocking) {
            _blocking.push_back(_running);
        } else {
            _ready.push_back(_running);
        }
    } else {
        delete (this->_id_array[_running]);
        this->_id_array[_running] = nullptr;
    }

    // update the running.
    _running = _ready.front();
    _ready.pop_front();

    increment_tid_quant_counter(_running);

    return this->_id_array[_running]->get_env();
}

/**
 * checks if we still have a thread waiting to work
 * @return true if not empty, false otherwise
 */
bool UthreadCore::is_ready() {
    return _ready.size() > 0;
}

/**
 * Synces the running thread to a given thread
 * @param tid - of the thread which we sync the running to
 * @return 0 if success, false otherwise
 */
int UthreadCore::sync_node(int tid) {
    if (tid < 0 || tid >= MAX_THREAD_NUM || _running == 0 || _id_array[tid] == nullptr\
        || _running == tid) {
        return -1;
    }
    _id_array[_running]->is_synced = true;
    _id_array[tid]->sync_waiters.push_back(_running);
    switch_context_with_blocking(true, true, true);
    return 0;
}

/**
 * increases the quantum counter by 1
 * @param tid - id of the thread to increase
 */
void UthreadCore::increment_tid_quant_counter(int tid) {
    if (tid < 0 || tid >= MAX_THREAD_NUM || _id_array[tid] == nullptr) {
        return;
    }

    _id_array[tid]->increment_quant();
    this->_total_quantums_counter++;
}