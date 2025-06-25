#pragma once

#define __IRMC_TKNCAT(_a, _b) _a ## _b
#define IRMC_TKNCAT(_a, _b) __IRMC_TKNCAT(_a, _b)
#define IRMC_UNIQUE(_a) IRMC_TKNCAT(_a, __COUNTER__)
#define IRMC_UNREACHABLE __builtin_unreachable()
#define IRMC_UNIMPLEMENTED { do { IRMC_MSG(FATAL, "NOT IMPLEMENTED - %s:%d in func: %s", __FILE__, __LINE__, __func__); IRMC_UNREACHABLE; } while(0); }
#define IRMC_ARRSTRIDE(_arr) sizeof((_arr)[0])
#define IRMC_ARRLEN(_arr) (sizeof(_arr) / IRMC_ARRSTRIDE(_arr))
#define IRMC_RETURN(_val) { return _val; }