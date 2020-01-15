#include "sensors.h"

#include "acceleration.h"

bool sensors__init(void) {
  bool status = true;

  status &= acceleration__init();
  // status &= apds__init();

  return status;
}