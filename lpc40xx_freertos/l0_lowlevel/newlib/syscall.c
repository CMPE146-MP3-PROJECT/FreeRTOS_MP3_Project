#include <stddef.h>
#include <stdint.h>

static void *sbrk_ram1(size_t requested_byte_count);
static void *sbrk_ram2(size_t requested_byte_count);

/**
 * malloc() reaches out here to grab memory
 * On a host OS (Linux, MacOS), the operating system provides the virtual memory to malloc
 */
void *_sbrk(size_t requested_byte_count) {
  void *memory_to_return = NULL;

  if (NULL == memory_to_return) {
    memory_to_return = sbrk_ram1(requested_byte_count);
  }

  if (NULL == memory_to_return) {
    memory_to_return = sbrk_ram2(requested_byte_count);
  }

  return memory_to_return;
}

static void *sbrk_ram1(size_t requested_byte_count) {
  // These symbols are defined by the linker script
  extern uint32_t _heap_start;
  extern uint32_t _heap_end;

  static void *next_free_heap = (void *)&_heap_start;
  void *memory_to_return = next_free_heap;
  next_free_heap += requested_byte_count;

  /**
   * If our next pointer is outside our valid RAM region, then we cannot return memory back
   * In addition, we need to subtract next_free_heap because of a couple of reasons:
   *   1. Maybe we still have a small chunk we could still serve in the future
   *   2. We do not want to increment next_free_heap and go out of bounds to wrap
   */
  if (!((next_free_heap >= (void *)&_heap_start) && (next_free_heap < (void *)&_heap_end))) {
    memory_to_return = NULL;
    next_free_heap -= requested_byte_count;
  }

  return memory_to_return;
}

static void *sbrk_ram2(size_t requested_byte_count) {
  extern uint32_t __start_of_unused_ram64;
  extern uint32_t __end_of_unused_ram64;

  static void *next_free_heap = (void *)&__start_of_unused_ram64;
  void *memory_to_return = next_free_heap;
  next_free_heap += requested_byte_count;

  if (!((next_free_heap >= (void *)&__start_of_unused_ram64) && (next_free_heap < (void *)&__end_of_unused_ram64))) {
    memory_to_return = NULL;
    next_free_heap -= requested_byte_count;
  }

  return memory_to_return;
}
