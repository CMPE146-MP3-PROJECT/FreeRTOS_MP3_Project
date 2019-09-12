#include <stddef.h>
#include <stdint.h>

// These symbols are defined by the linker script
extern uint32_t _bheap;
extern uint32_t _eheap;

/**
 * malloc() reaches out here to grab memory
 * On a host OS (Linux, MacOS), the operating system provides the virtual memory to malloc
 */
void *_sbrk(size_t requested_byte_count) {
  static const void *max_heap_ptr = (void *)&_eheap;
  static void *current_heap_base_ptr = NULL;

  if (current_heap_base_ptr == NULL) {
    current_heap_base_ptr = (void *)&_bheap;
  }

  void *next_heap_ptr = (uint8_t *)current_heap_base_ptr + requested_byte_count;
  void *ret_heap_ptr = NULL;

  if (next_heap_ptr < max_heap_ptr) {
    ret_heap_ptr = current_heap_base_ptr;
    current_heap_base_ptr = next_heap_ptr;
  }

  return ret_heap_ptr;
}
