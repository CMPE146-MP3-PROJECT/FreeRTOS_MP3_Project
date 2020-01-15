#pragma once

#include <stdatomic.h>

#include "FreeRTOS.h"
#include "semphr.h"

/**
 * This is an optional module that can act like a 'leightweight' or faster lock
 *
 * It uses a counting semaphore but the optimization is that we may not ever use the semaphore API
 * if there are not simultaneous users of the lock
 *
 * @warning this is not a mutex, and will not handle priority inversion
 *
 * Takes inspirations from:
 * - https://preshing.com/20150316/semaphores-are-surprisingly-versatile/
 * - https://lwn.net/Articles/509102/
 */
typedef struct {
  StaticSemaphore_t signal_struct; ///< struct memory of 'signal'
  SemaphoreHandle_t signal;        ///< counting semaphore used as a 'signal'
  atomic_int count;                ///< Count that tracks number of users waiting for leightweight_lock__acquire()
} leightweight_lock_s;

/// Lock must be initialized before it is used
void leightweight_lock__initialize(leightweight_lock_s *lock);

/// Blocks forever until the lock is acquired
void leightweight_lock__acquire(leightweight_lock_s *lock);

/// Releases the lock and unblocks potential users blocked on leightweight_lock__acquire()
void leightweight_lock__release(leightweight_lock_s *lock);
