#include "hw_timer.h"

#include "lpc40xx.h"
#include "lpc_peripherals.h"

typedef struct {
  LPC_TIM_TypeDef *registers;
  lpc_peripheral_e peripheral_id;
} hw_timer_s;

static const hw_timer_s hw_timers[] = {
    {LPC_TIM0, LPC_PERIPHERAL__TIMER0},
    {LPC_TIM1, LPC_PERIPHERAL__TIMER1},
    {LPC_TIM2, LPC_PERIPHERAL__TIMER2},
    {LPC_TIM3, LPC_PERIPHERAL__TIMER3},
};

void hw_timer__enable(lpc_timer_e timer, const uint32_t prescalar_divider, function__void_f isr_callback) {
  // Peripheral must be turned on before accessing its registers
  lpc_peripheral__turn_on_power_to(hw_timers[timer].peripheral_id);

  hw_timers[timer].registers->TCR = 0;
  hw_timers[timer].registers->TC = 0;

  hw_timers[timer].registers->IR = 0x3F; // Clear all interrupts
  hw_timers[timer].registers->MCR = 0;   // Clear all match register settings
  hw_timers[timer].registers->MR0 = 0;
  hw_timers[timer].registers->MR1 = 0;
  hw_timers[timer].registers->MR2 = 0;
  hw_timers[timer].registers->MR3 = 0;

  /* Interrupt can be enabled because we can assume the timer is not setup
   * for interrupt yet through hw_timer__enable_match_isr()
   */
  lpc_peripheral__enable_interrupt(hw_timers[timer].peripheral_id, isr_callback);

  hw_timers[timer].registers->PR = prescalar_divider;
  hw_timers[timer].registers->TCR = 1; // Enable
}

void hw_timer__enable_match_isr(lpc_timer_e timer, lpc_timer__mr_e mr_type, const uint32_t mr_value) {
  const uint32_t interrupt_on_match = (uint32_t)mr_type * 3; // 3 bits per MR
  hw_timers[timer].registers->MCR |= (1 << interrupt_on_match);

  // Four MR registers are contiguous and they start from &MR0
  volatile uint32_t *mr_base = &(hw_timers[timer].registers->MR0);
  volatile uint32_t *mr_register = mr_base + mr_type;

  *mr_register = mr_value;
}

void hw_timer__acknowledge_interrupt(lpc_timer_e timer, lpc_timer__mr_e mr_type) {
  hw_timers[timer].registers->IR = (1 << (uint32_t)mr_type);
}

uint32_t hw_timer__get_value(lpc_timer_e timer) { return hw_timers[timer].registers->TC; }

void hw_timer__set_value(lpc_timer_e timer, uint32_t tc_value) { hw_timers[timer].registers->TC = tc_value; }
