#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define BATTERY_VOLTAGE 48
#define DRV_CURRENT_SENSE_AMPLIFICATION 40
#define PHASE_RESISTANCE_OHMS 0.0005

_Static_assert(DRV_CURRENT_SENSE_AMPLIFICATION == 5
              || DRV_CURRENT_SENSE_AMPLIFICATION == 10
              || DRV_CURRENT_SENSE_AMPLIFICATION == 20
              || DRV_CURRENT_SENSE_AMPLIFICATION == 40,
              "invalid current sense amplification amount");

#endif
