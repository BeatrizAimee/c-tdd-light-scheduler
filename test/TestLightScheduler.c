#include "LightScheduler.h"
#include "unity.h"
#include "LightControlSpy.h"
#include "MockTimeService.h"
#include <stdlib.h>

//Ended up making the tests for off functions bc if felt more complete/like i was testing every possibility

void setUp(void)
{   
    LightControl_init();
}

void tearDown(void)
{   
    TimeService_destroy_Expect();
    LightScheduler_destroy();
    LightControl_destroy();
}

// --- AUX FUNCTIONS ---

static Time time; 

static void setupScheduler(void) {
    
    TimeService_init_Expect();
    TimeService_startPeriodicAlarm_IgnoreAndReturn(42);
    TimeService_stopPeriodicAlarm_Ignore();
    LightScheduler_init();
}

static void setTimeAndWakeUp(WeekDay day, int minute) {
    time.dayOfWeek = day;
    time.minuteOfDay = minute;
    TimeService_getTime_ExpectAnyArgs();
    TimeService_getTime_ReturnThruPtr_time(&time);
    LightScheduler_wakeUp();
}

static void checkLightState(int expectedId, int expectedState) {
    TEST_ASSERT_EQUAL(expectedId, LightControlSpy_getLastLightId());
    TEST_ASSERT_EQUAL(expectedState, LightControlSpy_getLastState());
}

// -----------------------------------------------------

// ----------------------------------Test 0: Initialize the scheduler
void test_InitializeAndDestructPeriodicAlarm(void) {
    TimeService_init_Expect();
    TimeService_startPeriodicAlarm_ExpectAndReturn(60, NULL, 42);
    TimeService_startPeriodicAlarm_IgnoreArg_callback(); 
    TimeService_stopPeriodicAlarm_Expect(42);
    
    LightScheduler_init(); 
}

// Aditional Tests for initialization/Destruction

//Test to see if the id in the destructor is hardcoded
void test_InitializeAndDestructPeriodicAlarmWithADifferentID(void) {
    TimeService_init_Expect();
    TimeService_startPeriodicAlarm_ExpectAndReturn(60, NULL, 99);
    TimeService_startPeriodicAlarm_IgnoreArg_callback();
    TimeService_stopPeriodicAlarm_Expect(99);

    LightScheduler_init();
}

void testCallDestroyBeforeInit(void) {
    //Dont call Init, tearDown will call the destroyer.
}

void testDoubleDestroy(void) {
    TimeService_init_Expect();
    TimeService_startPeriodicAlarm_ExpectAndReturn(60, NULL, 123);
    TimeService_startPeriodicAlarm_IgnoreArg_callback();
    TimeService_stopPeriodicAlarm_Expect(123);

    TimeService_destroy_Expect();

    LightScheduler_init();
    LightScheduler_destroy();
    //TearDown destroys a second time
}

// --------------------------------------------------------------------Test 1: No events scheduled
void test_WakeupWithNoEventsScheduled(void) { 
    setupScheduler();
    TimeService_getTime_Ignore();

    LightScheduler_wakeUp(); 

    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN); 
}

void test_WakeUpDoesNothingIfNothingScheduled(void) {
    setupScheduler();

    setTimeAndWakeUp((WeekDay)NONE, -1);

    TEST_ASSERT_EQUAL(LIGHT_ID_UNKNOWN, LightControlSpy_getLastLightId());
}

// Aditional Tests for whakeUp Function
void test_WakeUpAsksCurrentTime(void) {
    setupScheduler();
    TimeService_getTime_ExpectAnyArgs(); //The scheduler MUST ask for the time, cant have an empty wakeUp anymore

    LightScheduler_wakeUp();
}

void test_DestroyWipesMemory(void) {
    setupScheduler();
    LightScheduler_TurnOn(3, MONDAY, 600);
    TimeService_destroy_Expect();
    LightScheduler_destroy();

    // Call at the time it was scheduled
    setTimeAndWakeUp(MONDAY, 600);

    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_InitResetsAllScheduledEvents(void) {
    setupScheduler();
    LightScheduler_TurnOn(3, MONDAY, 600);

    //End of a test and the beginning of a new one
    TimeService_destroy_Expect();
    LightScheduler_destroy();
    setupScheduler(); //This should wipe prev test

    setTimeAndWakeUp(MONDAY, 600);

    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

// ----------------------------------------------------------------------Test 2: Schedule an event, trigger it, advance exactly one week

void test_TurnOnRepeatsWeekly(void) {
    setupScheduler();
    LightScheduler_TurnOn(3, MONDAY, 600);

    // First Week
    setTimeAndWakeUp(MONDAY, 600);
    checkLightState(3, LIGHT_ON);

    LightControl_init(); 

    // Advance One Week
    setTimeAndWakeUp(MONDAY, 600);
    checkLightState(3, LIGHT_ON);
}

// Aditional test - mirror for turn off
void test_TurnOffRepeatsWeekly(void) {
    setupScheduler();
    LightScheduler_TurnOff(3, MONDAY, 600);

    // First Week
    setTimeAndWakeUp(MONDAY, 600);
    checkLightState(3, LIGHT_OFF);

    LightControl_init(); 

    // Advance One Week
    setTimeAndWakeUp(MONDAY, 600);
    checkLightState(3, LIGHT_OFF);
}
// ---------------------------------------------------------------------------Test 3: Boundary check for the IDs (between 0 and 255)

void test_TurnOnRejectsIdTooHigh(void) {
    setupScheduler();
    int result = LightScheduler_TurnOn(256, MONDAY, 600);
    TEST_ASSERT_EQUAL(ERR_INVALID_ID, result);
}

void test_TurnOnRejectsIdTooLow(void) {
    setupScheduler();
    int result = LightScheduler_TurnOn(-3, MONDAY, 600);
    TEST_ASSERT_EQUAL(ERR_INVALID_ID, result);
}

// Aditional Boundary tests

void test_TurnoffRejectsIdTooHigh(void) {
    setupScheduler();
    int result = LightScheduler_TurnOff(256, MONDAY, 600);
    TEST_ASSERT_EQUAL(ERR_INVALID_ID, result);

    setTimeAndWakeUp(MONDAY, 600);

    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_TurnOffRejectsIdTooLow(void) {
    setupScheduler();
    int result = LightScheduler_TurnOff(-3, MONDAY, 600);
    TEST_ASSERT_EQUAL(ERR_INVALID_ID, result);

    setTimeAndWakeUp(MONDAY, 600);

    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_TurnOnRejectsInvalidMinuteHigh(void) {
    setupScheduler();

    // Try to schedule a light with an impossible time
    int result = LightScheduler_TurnOn(3, SUNDAY, 1500);
    TEST_ASSERT_EQUAL(ERR_INVALID_TIME, result);

    setTimeAndWakeUp(SUNDAY, 1500);

    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_TurnOnRejectsInvalidMinuteLow(void) {
    setupScheduler();
    int result = LightScheduler_TurnOn(3, SUNDAY, -3);
    TEST_ASSERT_EQUAL(ERR_INVALID_TIME, result);

    setTimeAndWakeUp(SUNDAY, -3);

    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_TurnOffRejectsInvalidMinuteHigh(void) {
    setupScheduler();
    int result = LightScheduler_TurnOff(3, SUNDAY, 1500);
    TEST_ASSERT_EQUAL(ERR_INVALID_TIME, result);

    setTimeAndWakeUp(SUNDAY, 1500);

    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_TurnOffRejectsInvalidMinuteLow(void) {
    setupScheduler();
    int result = LightScheduler_TurnOff(3, SUNDAY, -3);
    TEST_ASSERT_EQUAL(ERR_INVALID_TIME, result);

    setTimeAndWakeUp(SUNDAY, -3);

    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void testTurnOnRejectsInvalidDayHigh(void) {
    setupScheduler();

    //Schedule for an invalid day 42
    int result = LightScheduler_TurnOn(3, (WeekDay)42, 1200);
    TEST_ASSERT_EQUAL(ERR_INVALID_DAY, result);

    setTimeAndWakeUp((WeekDay)42, 1200);

    TEST_ASSERT_EQUAL(LIGHT_ID_UNKNOWN, LightControlSpy_getLastLightId());
}

void testTurnOnRejectsInvalidDayLow(void) {
    setupScheduler();

    //Schedule for an invalid day -3
    int result = LightScheduler_TurnOn(3, (WeekDay)-3, 1200);
    TEST_ASSERT_EQUAL(ERR_INVALID_DAY, result);

    setTimeAndWakeUp((WeekDay)-3, 1200);

    TEST_ASSERT_EQUAL(LIGHT_ID_UNKNOWN, LightControlSpy_getLastLightId());
}

void testTurnOffRejectsInvalidDayHigh(void) {
    setupScheduler();

    //Schedule for an invalid day 42
    int result = LightScheduler_TurnOff(3, (WeekDay)42, 1200);
    TEST_ASSERT_EQUAL(ERR_INVALID_DAY, result);

    setTimeAndWakeUp((WeekDay)42, 1200);

    TEST_ASSERT_EQUAL(LIGHT_ID_UNKNOWN, LightControlSpy_getLastLightId());
}

void testTurnOffRejectsInvalidDayLow(void) {
    setupScheduler();

    //Schedule for an invalid day -2
    int result = LightScheduler_TurnOff(3, (WeekDay)-2, 1200);
    TEST_ASSERT_EQUAL(ERR_INVALID_DAY, result);

    setTimeAndWakeUp((WeekDay)-2, 1200);

    TEST_ASSERT_EQUAL(LIGHT_ID_UNKNOWN, LightControlSpy_getLastLightId());
}

void test_TurnOnAcceptsValidMinuteLowerBound(void) {
    setupScheduler();
    // Schedule for exactly Midnight
    int result = LightScheduler_TurnOn(3, MONDAY, 0);
    TEST_ASSERT_EQUAL(OK, result);

    setTimeAndWakeUp(MONDAY, 0);
    checkLightState(3, LIGHT_ON);
}

void test_TurnOnAcceptsValidMinuteUpperBound(void) {
    setupScheduler();

    // Schedule for exactly 11:59 PM
    int result = LightScheduler_TurnOn(3, MONDAY, 1439);
    TEST_ASSERT_EQUAL(OK, result);

    setTimeAndWakeUp(MONDAY, 1439);
    checkLightState(3, LIGHT_ON);
}

void test_TurnOffAcceptsValidMinuteLowerBound(void) {
    setupScheduler();

    // Schedule for exactly Midnight
    int result = LightScheduler_TurnOff(3, MONDAY, 0);
    TEST_ASSERT_EQUAL(OK, result);

    setTimeAndWakeUp(MONDAY, 0);
    checkLightState(3, LIGHT_OFF);
}

void test_TurnOffAcceptsValidMinuteUpperBound(void) {
    setupScheduler();

    // Schedule for exactly 11:59 PM
    int result = LightScheduler_TurnOff(3, MONDAY, 1439);
    TEST_ASSERT_EQUAL(OK, result);

    setTimeAndWakeUp(MONDAY, 1439);
    checkLightState(3, LIGHT_OFF);
}

void test_RemoveEventRejectsInvalidMinuteHigh(void) {
    setupScheduler();

    int result = LightScheduler_RemoveEvent(3, SUNDAY, 1500);
    TEST_ASSERT_EQUAL(ERR_INVALID_TIME, result);

    setTimeAndWakeUp(SUNDAY, 1500);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_RemoveEventRejectsInvalidMinuteLow(void) {
    setupScheduler();

    int result = LightScheduler_RemoveEvent(3, SUNDAY, -3);
    TEST_ASSERT_EQUAL(ERR_INVALID_TIME, result);

    setTimeAndWakeUp(SUNDAY, -3);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_RemoveEventRejectsInvalidDayHigh(void) {
    setupScheduler();

    int result = LightScheduler_RemoveEvent(3, (WeekDay)42, 1200);
    TEST_ASSERT_EQUAL(ERR_INVALID_DAY, result);

    setTimeAndWakeUp((WeekDay)42, 1200);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_RemoveEventRejectsInvalidDayLow(void) {
    setupScheduler();

    int result = LightScheduler_RemoveEvent(3, (WeekDay)-3, 1200);
    TEST_ASSERT_EQUAL(ERR_INVALID_DAY, result);

    setTimeAndWakeUp((WeekDay)-3, 1200);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_RemoveEventRejectsIdTooHigh(void) {
    setupScheduler();

    int result = LightScheduler_RemoveEvent(256, MONDAY, 600);
    TEST_ASSERT_EQUAL(ERR_INVALID_ID, result);

    setTimeAndWakeUp(MONDAY, 600);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_RemoveEventRejectsIdTooLow(void) {
    setupScheduler();

    int result = LightScheduler_RemoveEvent(-3, MONDAY, 600);
    TEST_ASSERT_EQUAL(ERR_INVALID_ID, result);

    setTimeAndWakeUp(MONDAY, 600);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

// -----------------------------------------------------------------------------Test 4: Triggered only on specified day and within one minute resolution
void test_TriggersOnTimeAndLightStatePersists(void) {
    setupScheduler();
    LightScheduler_TurnOn(3, FRIDAY, 600);

    // ONE MINUTE BEFORE (09:59)
    setTimeAndWakeUp(FRIDAY, 599); 
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);

    // ON TIME (10:00)
    setTimeAndWakeUp(FRIDAY, 600); 
    checkLightState(3, LIGHT_ON);

    // ONE MINUTE AFTER (10:01)
    setTimeAndWakeUp(FRIDAY, 601); 
    checkLightState(3, LIGHT_ON);
}

// Aditional test for time-trigger

void test_DoesNotRetriggerAfterScheduledMinutePasses(void) {
    setupScheduler();
    LightScheduler_TurnOn(3, FRIDAY, 600);

    setTimeAndWakeUp(FRIDAY, 600);

    // The light turned on. Now we wipe the Spy to see if it sent the ON command more than once
    LightControl_init();

    setTimeAndWakeUp(FRIDAY, 601);

    // If it sent, the ID would be 3. If it stayed quiet, it's UNKNOWN.
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}


// ----------------------------------------------------------------------------------Test 5 & Test 7: Verify it doesn't trigger on the wrong day 
void test_LightStaysUnknownAtWrongDay(void) {
    setupScheduler();
    LightScheduler_TurnOn(3, FRIDAY, 600);

    // Check THURSDAY 10:00 (or any following day)
    setTimeAndWakeUp(THURDSDAY, 600);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

// ----------------------------------------------------------------------------------Test 6: Verify it triggers on a defined day
void test_TurnLightOnAtATime(void) {
    setupScheduler();
    LightScheduler_TurnOn(3, FRIDAY, 600);

    setTimeAndWakeUp(FRIDAY, 600);
    checkLightState(3, LIGHT_ON);
}

// Aditional Tests for turn off/on in a specific time

void test_TurnLightOffAtATime(void) {
    setupScheduler();
    LightScheduler_TurnOff(3, SUNDAY, 1200);

    setTimeAndWakeUp(SUNDAY, 1200);

    checkLightState(3, LIGHT_OFF);
}

void test_TurnLightOnAtATimeDifferentId(void) { //Check if id is hardcoded
    setupScheduler();
    LightScheduler_TurnOn(99, SUNDAY, 1200);

    setTimeAndWakeUp(SUNDAY, 1200);

    checkLightState(99, LIGHT_ON);
}

void test_TurnLightOffAtATimeDifferentId(void) { //Check if id is hardcoded
    setupScheduler();
    LightScheduler_TurnOff(99, SUNDAY, 1200);

    setTimeAndWakeUp(SUNDAY, 1200);

    checkLightState(99, LIGHT_OFF);
}

// -------------------------------------------------------------------------------------------Test 8: Schedule EVERYDAY at 08:00

void test_ScheduleTurnOnEverydayTrigger(void) {
    setupScheduler();
    LightScheduler_TurnOn(3, EVERYDAY, 480);

    setTimeAndWakeUp(MONDAY, 480);
    checkLightState(3, LIGHT_ON);

    LightControl_init(); 

    setTimeAndWakeUp(TUESDAY, 480);
    checkLightState(3, LIGHT_ON);
}

// Aditional test - mirror for turn off

void test_ScheduleTurnOffEverydayTrigger(void) {
    setupScheduler();

    LightScheduler_TurnOff(3, EVERYDAY, 480);

    setTimeAndWakeUp(MONDAY, 480);
    checkLightState(3, LIGHT_OFF);

    LightControl_init();

    setTimeAndWakeUp(TUESDAY, 480);
    checkLightState(3, LIGHT_OFF);
}

// -------------------------------------------------------------------------------------Test 9: Schedule WEEKEND at 10:00

void test_ScheduleTurnOnWeekendTrigger(void) {
    setupScheduler();
    LightScheduler_TurnOn(3, WEEKEND, 600);

    setTimeAndWakeUp(SATURDAY, 600);
    checkLightState(3, LIGHT_ON);

    LightControl_init();

    setTimeAndWakeUp(SUNDAY, 600);
    checkLightState(3, LIGHT_ON);

    LightControl_init();

    // Verify it ignores Monday
    setTimeAndWakeUp(MONDAY, 600);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

// Aditional test - mirror for turn off

void test_ScheduleTurnOffWeekendTrigger(void) {
    setupScheduler();
    LightScheduler_TurnOff(3, WEEKEND, 600);

    setTimeAndWakeUp(SATURDAY, 600);
    checkLightState(3, LIGHT_OFF);

    LightControl_init();

    setTimeAndWakeUp(SUNDAY, 600);
    checkLightState(3, LIGHT_OFF);

    LightControl_init();

    setTimeAndWakeUp(MONDAY, 600);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

// ------------------------------------------------------------------------Test 10: Schedule WEEKDAY at 10:00

void test_ScheduleTurnOnWeekdayTrigger(void) {
    setupScheduler();
    LightScheduler_TurnOn(1, WEEKDAY, 600);

    // Check FRIDAY 10:00
    setTimeAndWakeUp(FRIDAY, 600);
    checkLightState(1, LIGHT_ON);

    LightControl_init();

    // Check WEDNESDAY 10:00
    setTimeAndWakeUp(WEDNESDAY, 600);
    checkLightState(1, LIGHT_ON);

    // Reset the Spy memory
    LightControl_init();

    // Verify it ignores Sunday
    setTimeAndWakeUp(SUNDAY, 600);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

// Aditional test - mirror for turn off

void test_ScheduleTurnOffWeekdayTrigger(void) {
    setupScheduler();
    LightScheduler_TurnOff(1, WEEKDAY, 600);

    // Check FRIDAY 10:00
    setTimeAndWakeUp(FRIDAY, 600);
    checkLightState(1, LIGHT_OFF);

    LightControl_init();

    // Check WEDNESDAY 10:00
    setTimeAndWakeUp(WEDNESDAY, 600);
    checkLightState(1, LIGHT_OFF);
    
    // Reset the Spy memory
    LightControl_init();

    // Verify it ignores Sunday
    setTimeAndWakeUp(SUNDAY, 600);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}


// ---------------------------------------------------------------------------Test 11: Schedule ON at 10:00 and OFF at 11:00

void test_ScheduleMultipleEventsForTheSameLight(void) {
    setupScheduler();
    LightScheduler_TurnOn(3, MONDAY, 600);
    LightScheduler_TurnOff(3, MONDAY, 660);

    setTimeAndWakeUp(MONDAY, 600);
    checkLightState(3, LIGHT_ON);

    setTimeAndWakeUp(MONDAY, 660);
    checkLightState(3, LIGHT_OFF);
}

// Aditional test for multiple access

void test_AcceptsMultipleDifferentLightsAtTheExactSameTime(void) {
    setupScheduler();

    // Schedule Light 1 ON at 10:00
    int result1 = LightScheduler_TurnOn(1, MONDAY, 600);
    TEST_ASSERT_EQUAL(OK, result1);

    // Schedule Light 2 ON at 10:00
    int result2 = LightScheduler_TurnOn(2, MONDAY, 600);

    // Didnt trigger an error
    TEST_ASSERT_EQUAL(OK, result2);
}

// ---------------------------------------------------------------------------Test 12: Duplicate events for same light at exact same time

void test_RejectsDuplicateScheduleForSameLightAndSameTime(void) {
    setupScheduler();

    int result1 = LightScheduler_TurnOn(3, MONDAY, 600);
    TEST_ASSERT_EQUAL(OK, result1);
    
    int result2 = LightScheduler_TurnOff(3, MONDAY, 600);
    TEST_ASSERT_EQUAL(ERR_CONFLICT, result2);

    setTimeAndWakeUp(MONDAY, 600);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

// ----------------------------------------------------------------------------Test 13: Remove an event, verify no changes happen

void test_RemoveScheduledEvent(void) {
    setupScheduler();
    LightScheduler_TurnOn(5, MONDAY, 720);

    int removeResult = LightScheduler_RemoveEvent(5, MONDAY, 720);
    TEST_ASSERT_EQUAL(OK, removeResult); 

    setTimeAndWakeUp(MONDAY, 720);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

// Aditional Removing test

void test_RemovingAnEventAllowsToScheduleAnother(void) {
    setupScheduler();

    for (int i = 0; i < 256; i++) {
        LightScheduler_TurnOn(1, MONDAY, i);
    }

    int failedResult = LightScheduler_TurnOn(1, MONDAY, 257);
    TEST_ASSERT_EQUAL(ERR_FULL, failedResult);

    // Remove the first event to free up a slot
    int removeResult = LightScheduler_RemoveEvent(1, MONDAY, 0);
    TEST_ASSERT_EQUAL(OK, removeResult);

    int successResult = LightScheduler_TurnOn(99, TUESDAY, 100);
    TEST_ASSERT_EQUAL(OK, successResult);
}

// -------------------------------------------------------------------------------Test 14: Delete an event that does not exist

void test_RemoveEventReturnsErrorIfEventDoesNotExist(void) {
    setupScheduler();
    int removeResult = LightScheduler_RemoveEvent(99, MONDAY, 720);
    TEST_ASSERT_EQUAL(ERR_NOT_FOUND, removeResult);
}

// --------------------------------------------------------------------------------Test 15: Reject 257th event

void test_ScheduleMoreThan256EventsReturnsError(void) {
    setupScheduler();
    
    for (int i = 0; i < 256; i++) {
        int result = LightScheduler_TurnOn(1, MONDAY, i);
        TEST_ASSERT_EQUAL(OK, result); 
    }

    int result257 = LightScheduler_TurnOn(1, MONDAY, 256);
    TEST_ASSERT_EQUAL(ERR_FULL, result257); 
}

// --------------------------------------------------------------------------------Test 16: Last event (256th) triggers correctly

void test_LastEventTriggersCorrectly(void) {
    setupScheduler();

    for (int i = 0; i < 256; i++) {
        LightScheduler_TurnOn(1, MONDAY, i);
    }

    setTimeAndWakeUp(MONDAY, 255);
    checkLightState(1, LIGHT_ON);
}


// ------------------------------------------------------------------------------------ Test 17: Count 

void test_ScheduledEventsCount(void) {
    setupScheduler();

    // Should start at 0
    TEST_ASSERT_EQUAL(0, LightScheduler_GetScheduledEventsCount());

    LightScheduler_TurnOn(1, MONDAY, 600);
    LightScheduler_TurnOff(2, TUESDAY, 700);
    LightScheduler_TurnOn(3, WEDNESDAY, 800);

    TEST_ASSERT_EQUAL(3, LightScheduler_GetScheduledEventsCount());
}

//----------------------------------------------------------------------------------------- Test 18: Random

void test_TurnOnWithRandomization(void) {

    setupScheduler();
    
    srand(42); // Force the random generator to a known starting point
    int Offset = (rand() % 31) - 15; //seed
    
    srand(42);  //reset seed for scheduler
    int result = LightScheduler_TurnOnRandom(1, MONDAY, 600);
    TEST_ASSERT_EQUAL(OK, result);

    if (Offset != 0) {
        setTimeAndWakeUp(MONDAY, 600);
        checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
    }

    //Verify at reandomized time
    setTimeAndWakeUp(MONDAY, 600 + Offset);
    checkLightState(1, LIGHT_ON);
}
void test_TurnOffWithRandomization(void) {

    setupScheduler();
    
    srand(42); // Force the random generator to a known starting point
    int Offset = (rand() % 31) - 15; //seed
    
    srand(42);  //reset seed for scheduler
    int result = LightScheduler_TurnOffRandom(1, MONDAY, 600);
    TEST_ASSERT_EQUAL(OK, result);

    if (Offset != 0) {
        setTimeAndWakeUp(MONDAY, 600);
        checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
    }

    //Verify at reandomized time
    setTimeAndWakeUp(MONDAY, 600 + Offset);
    checkLightState(1, LIGHT_OFF);
}

void test_TurnOnRandomClampsAtMidnightforValidINputs(void) {
    setupScheduler();

    int seed = 0;
    int offset;
    do {
        srand(seed);
        offset = (rand() % 31) - 15;
        if (offset >= 0) seed++;
    } while (offset >= 0); //search a good seed

    srand(seed);
    
    int result = LightScheduler_TurnOnRandom(1, MONDAY, 0); //negative offset will put below 0
    TEST_ASSERT_EQUAL(OK, result);

    setTimeAndWakeUp(MONDAY, 0);
    checkLightState(1, LIGHT_ON);
}

void test_TurnOffRandomClampsAtMidnightforValidINputs(void) {
    setupScheduler();

    int seed = 0;
    int offset;
    do {
        srand(seed);
        offset = (rand() % 31) - 15;
        if (offset >= 0) seed++;
    } while (offset >= 0);

    srand(seed);

    int result = LightScheduler_TurnOffRandom(1, MONDAY, 0);
    TEST_ASSERT_EQUAL(OK, result);

    setTimeAndWakeUp(MONDAY, 0);
    checkLightState(1, LIGHT_OFF);
}

void test_TurnOnRandomClampsAtEndOfDayforValidINputs(void) {
    setupScheduler();

    int seed = 0;
    int offset;
    do {
        srand(seed);
        offset = (rand() % 31) - 15;
        if (offset <= 0) seed++;
    } while (offset <= 0);

    srand(seed);
    
    int result = LightScheduler_TurnOnRandom(1, MONDAY, 1439); //positive offset puts above 1439
    TEST_ASSERT_EQUAL(OK, result);

    setTimeAndWakeUp(MONDAY, 1439);
    checkLightState(1, LIGHT_ON);
}

void test_TurnOffRandomClampsAtEndOfDayforValidINputs(void) {
    setupScheduler();

    int seed = 0;
    int offset;
    do {
        srand(seed);
        offset = (rand() % 31) - 15;
        if (offset <= 0) seed++;
    } while (offset <= 0);

    srand(seed);
    
    int result = LightScheduler_TurnOffRandom(1, MONDAY, 1439); //positive offset puts above 1439
    TEST_ASSERT_EQUAL(OK, result);

    setTimeAndWakeUp(MONDAY, 1439);
    checkLightState(1, LIGHT_OFF);
}

void test_TurnOnRandomRejectsInvalidMinuteInputHigh(void) {
    setupScheduler();

    int result = LightScheduler_TurnOnRandom(3, SUNDAY, 1500);
    TEST_ASSERT_EQUAL(ERR_INVALID_TIME, result);

    setTimeAndWakeUp(SUNDAY, 1500);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_TurnOffRandomRejectsInvalidMinuteInputHigh(void) {
    setupScheduler();

    int result = LightScheduler_TurnOffRandom(3, SUNDAY, 1500);
    TEST_ASSERT_EQUAL(ERR_INVALID_TIME, result);

    setTimeAndWakeUp(SUNDAY, 1500);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_TurnOnRandomRejectsInvalidMinuteInputLow(void) {
    setupScheduler();

    int result = LightScheduler_TurnOnRandom(3, SUNDAY, -50);
    TEST_ASSERT_EQUAL(ERR_INVALID_TIME, result);

    setTimeAndWakeUp(SUNDAY, -50);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_TurnOffRandomRejectsInvalidMinuteInputLow(void) {
    setupScheduler();


    int result = LightScheduler_TurnOffRandom(3, SUNDAY, -50);
    TEST_ASSERT_EQUAL(ERR_INVALID_TIME, result);

    setTimeAndWakeUp(SUNDAY, -50);
    checkLightState(LIGHT_ID_UNKNOWN, LIGHT_STATE_UNKNOWN);
}

void test_TurnOnRandomReturnsConflictIfRandomizedTimeOverlaps(void) {
    setupScheduler();

    int normalResult = LightScheduler_TurnOn(1, MONDAY, 610);
    TEST_ASSERT_EQUAL(OK, normalResult);

    int seed = 0;
    int offset;
    do {
        srand(seed);
        offset = (rand() % 31) - 15;
        if (offset != 10) seed++;
    } while (offset != 10); //seeds that produce an offset of 10

    srand(seed);
    
    int randomResult = LightScheduler_TurnOnRandom(1, MONDAY, 600); //overlaps with previous schedule
    TEST_ASSERT_EQUAL(ERR_CONFLICT, randomResult);
}