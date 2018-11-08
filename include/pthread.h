// (C) 2018 Srimanta Barua

#pragma once

#include <sys/types.h>
#include <time.h>
#include <sched.h>

// Used to statically initialize a pthread_once_t
#define PTHREAD_ONCE_INIT { 1, 0 }

// Used to statically initialize a pthread_cond_t
#define PTHREAD_COND_INITIALIZER ((pthread_cond_t) 0xffffffff)

// Used to statically initialize a pthread_mutex_t
#define PTHREAD_MUTEX_INITIALIZER ((pthread_mutex_t) 1)


// pthread_mutexattr_t types
#define PTHREAD_MUTEX_NORMAL    0
#define PTHREAD_MUTEX_RECURSIVE 1


// Create a new thread
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		   void* (*start_routine)(void*), void *arg);

// Join with a terminated thread
int pthread_join(pthread_t thread, void **retval);

// Compare thread IDs
int pthread_equal(pthread_t t1, pthread_t t2);

// Obtain ID of calling thread
pthread_t pthread_self(void);

// Detach a thread
int pthread_detach(pthread_t thread);

// Send a cancellation request to a thread
int pthread_cancel(pthread_t thread);

// Thread-specific-data create
int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));

// Thread-specific-data delete
int pthread_key_delete(pthread_key_t key);

// Get thread-specific data
void* pthread_getspecific(pthread_key_t key);

// Set thread-specific data
int pthread_setspecific(pthread_key_t key, const void *value);

// Dynamic package initialization
int pthread_once(pthread_once_t *once, void (*init_routine)(void));

// Lock a mutex
int pthread_mutex_lock(pthread_mutex_t *mutex);

// Try to lock a mutex. Do not block if already locked
int pthread_mutex_trylock(pthread_mutex_t *mutex);

// Unlock a mutex
int pthread_mutex_unlock(pthread_mutex_t *mutex);

// Initialize a mutex
int pthread_mutex_init(pthread_mutex_t *restrict mutex,
		       const pthread_mutexattr_t *attr);

// Destroy a mutex
int pthread_mutex_destroy(pthread_mutex_t *mutex);

// Initialize a mutex attributes object
int pthread_mutexattr_init(pthread_mutexattr_t *attr);

// Set the mutex type attribute
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);

// Destroy a mutex attributes object
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

// Initialize condition variable
int pthread_cond_init(pthread_cond_t *restrict cond,
		      const pthread_condattr_t *restrict attr);

// Destroy condition variable
int pthread_cond_destroy(pthread_cond_t *cond);

// Broadcast a condition
int pthread_cond_broadcast(pthread_cond_t *cond);

// Signal a condition
int pthread_cond_signal(pthread_cond_t *cond);

// Wait on a condition
int pthread_cond_wait(pthread_cond_t *restrict cond,
		      pthread_mutex_t *restrict mutex);

// Wait on a condition, except error is returned if abstime passes
int pthread_cond_timedwait(pthread_cond_t *restrict cont,
			   pthread_mutex_t *restrict mutex,
			   const struct timespec *restrict abstime);
