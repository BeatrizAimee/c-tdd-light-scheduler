#include "LightScheduler.h"
#include <stdlib.h>

typedef struct {
    int id;
    WeekDay day;
    int minute;
    int state;
} ScheduledEvent;

static ScheduledEvent events[256];
static int handler = -1;


// AUX FUNCTIONS

static void wipeMemory(void) {
    // Wipe the memory
    for (int i = 0; i < 256; i++) {
        events[i].id = -1;
    }
}

static int checkBounds(int scheduledId, WeekDay scheduledDay, int scheduledMinute) {
    if (scheduledMinute < 0 || scheduledMinute > 1439) return ERR_INVALID_TIME;
    if (scheduledDay != EVERYDAY && scheduledDay != WEEKEND && scheduledDay != WEEKDAY && (scheduledDay < MONDAY || scheduledDay > SUNDAY)) return ERR_INVALID_DAY;
    if (scheduledId < 0 || scheduledId > 255) return ERR_INVALID_ID;
    return OK; 
}

static int scheduleEvent(int scheduledId, WeekDay scheduledDay, int scheduledMinute, int state) {
    int check = checkBounds(scheduledId, scheduledDay, scheduledMinute);
    if (check != OK) return check;

    for (int i = 0; i < 256; i++) {

        if (events[i].id == scheduledId && events[i].day == scheduledDay && events[i].minute == scheduledMinute) {
            events[i].id = -1;
            return ERR_CONFLICT;
        }

        if (events[i].id == -1) {
            events[i].id = scheduledId;
            events[i].day = scheduledDay;
            events[i].minute = scheduledMinute;
            events[i].state = state; // Dynamically set 1 or 0 here
            return OK;
        }
    }
    return ERR_FULL;
}

static int randomize(int scheduledMinute){ 
    
    int offset = (rand() % 31) - 15; //random offset from -15 to +15
    int randomizedMinute = scheduledMinute + offset;
    
    if (randomizedMinute < 0) randomizedMinute = 0;
    if (randomizedMinute > 1439) randomizedMinute = 1439;

    return randomizedMinute;
}

//Called Functions


void LightScheduler_init(void) {
    TimeService_init();
    wipeMemory();
    handler = TimeService_startPeriodicAlarm(60, LightScheduler_wakeUp);
}

void LightScheduler_destroy(void) {
    if (handler != -1) {
        TimeService_stopPeriodicAlarm(handler);
        handler = -1;
        wipeMemory();
    }
    TimeService_destroy();
}

void LightScheduler_wakeUp(void) {
    Time currentTime;
    TimeService_getTime(&currentTime);

    for (int i = 0; i < 256; i++) {
        if (events[i].id != -1) {
            int isWeekend = (events[i].day == WEEKEND && (currentTime.dayOfWeek == SATURDAY || currentTime.dayOfWeek == SUNDAY));
            int isWeekday = (events[i].day == WEEKDAY && (currentTime.dayOfWeek >= MONDAY && currentTime.dayOfWeek <= FRIDAY));

            if ((currentTime.dayOfWeek == events[i].day || events[i].day == EVERYDAY || isWeekend || isWeekday) && currentTime.minuteOfDay == events[i].minute) {

                if (events[i].state == 1) {
                    LightControl_on(events[i].id);
                } else if (events[i].state == 0) {
                    LightControl_off(events[i].id);
                }
            }
        }
    }
}

int LightScheduler_TurnOn(int scheduledId, WeekDay scheduledDay, int scheduledMinute) {
    return scheduleEvent(scheduledId, scheduledDay, scheduledMinute, 1);
}

int LightScheduler_TurnOff(int scheduledId, WeekDay scheduledDay, int scheduledMinute) {
    return scheduleEvent(scheduledId, scheduledDay, scheduledMinute, 0);
}

int LightScheduler_RemoveEvent(int scheduledId, WeekDay scheduledDay, int scheduledMinute) {
    //We dont need this to pass the tests as we have a -1 in the end, but its an architetural decision to manage errors
    int check = checkBounds(scheduledId, scheduledDay, scheduledMinute);
    if (check != OK) return check;


    for (int i = 0; i < 256; i++) {
        if (events[i].id == scheduledId && events[i].day == scheduledDay && events[i].minute == scheduledMinute) {
            events[i].id = -1;
            return 0;
        }
    }
    // If the loop finishes without finding a match
    return ERR_NOT_FOUND;
}

int LightScheduler_GetScheduledEventsCount(void) {
    int count = 0;
    for (int i = 0; i < 256; i++) {
        if (events[i].id != -1) count++;
    }
    return count;
}

int LightScheduler_TurnOnRandom(int scheduledId, WeekDay scheduledDay, int scheduledMinute) {

    int check = checkBounds(scheduledId, scheduledDay, scheduledMinute);
    if (check != OK) return check;

    int randomizedMinute = randomize(scheduledMinute);
    return scheduleEvent(scheduledId, scheduledDay, randomizedMinute, 1);
}

int LightScheduler_TurnOffRandom(int scheduledId, WeekDay scheduledDay, int scheduledMinute) {

    int check = checkBounds(scheduledId, scheduledDay, scheduledMinute);
    if (check != OK) return check;

    int randomizedMinute = randomize(scheduledMinute);
    return scheduleEvent(scheduledId, scheduledDay, randomizedMinute, 0);
}