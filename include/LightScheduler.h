#ifndef LIGHT_SCHEDULER_H
#define LIGHT_SCHEDULER_H

#include "LightControl.h"
#include "TimeService.h"

// Error Handling
#define OK 0
#define ERR_INVALID_TIME -1
#define ERR_INVALID_DAY -2
#define ERR_INVALID_ID -3
#define ERR_CONFLICT -4
#define ERR_FULL -5
#define ERR_NOT_FOUND -6

int LightScheduler_TurnOnRandom(int, WeekDay, int);
int LightScheduler_TurnOffRandom(int, WeekDay, int);
int LightScheduler_GetScheduledEventsCount(void);
void LightScheduler_init(void);
void LightScheduler_destroy(void);
void LightScheduler_wakeUp(void);
int LightScheduler_TurnOn(int,WeekDay,int);
int LightScheduler_TurnOff(int,WeekDay,int );
int LightScheduler_RemoveEvent(int, WeekDay, int);

#endif
