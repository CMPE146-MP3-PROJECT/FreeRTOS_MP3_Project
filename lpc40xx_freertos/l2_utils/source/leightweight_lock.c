#include "leightweight_lock.h"

void leightweight_lock__initialize(leightweight_lock_s *lock) {
  lock->signal = xSemaphoreCreateCountingStatic(UINT32_MAX, 0, &(lock->signal_struct));
  lock->count = 0;
}

void leightweight_lock__acquire(leightweight_lock_s *lock) {
  /**
   * The first caller of this function will add 1, but the count would have been zero before, and hence ' > 0' would
   * not trigger, and thus would avoid invoking xSemaphoreTake() API. The second caller will fall through ' > 0' case
   * and would have to sleep on the semaphore. Since the count is now 2, during atomic_fetch_sub_explicit(), the
   * original caller would subtract and fall through ' > 1' to give semaphore and unblock our xSemaphoreTake()
   */
  if (atomic_fetch_add_explicit(&(lock->count), 1, memory_order_acquire) > 0) {
    xSemaphoreTake(lock->signal, portMAX_DELAY);
  }
}

void leightweight_lock__release(leightweight_lock_s *lock) {
  /**
   * If there are callers blocked on the signal, then the atomic_fetch_sub_explicit() will return ' > 1' because
   * the count of the lock is prior to the atomic_fetch_sub_explicit() function subtracting value of 1, in which
   * case, we 'give' the signal for someone blocked on xSemaphoreTake() at leightweight_lock__acquire()
   */
  if (atomic_fetch_sub_explicit(&(lock->count), 1, memory_order_release) > 1) {
    xSemaphoreGive(lock->signal);
  }
}
