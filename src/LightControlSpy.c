#include "LightControlSpy.h"

//Variables to store the last changes
static int id = LIGHT_ID_UNKNOWN;
static int state = LIGHT_STATE_UNKNOWN;


//Spy functions - getters
int LightControlSpy_getLastLightId(){
    return id;
}

int LightControlSpy_getLastState(){
    return state;
}

//LightControl functions

void LightControl_on(int receivedId){
    id = receivedId;
    state = LIGHT_ON;
}

void LightControl_off(int receivedId) {
    id = receivedId;
    state = LIGHT_OFF;
}

void LightControl_init(void) { //reset for each test call
    id = LIGHT_ID_UNKNOWN;
    state = LIGHT_STATE_UNKNOWN;
}

void LightControl_destroy(void) {
    LightControl_init();
}

