#include "LightControlSpy.h" 
#include "unity.h" 

void testLightControlSpyReturnsLastStateChange(void) 
{ 
    LightControl_init(); 
    LightControl_on(42); 
    
    TEST_ASSERT_EQUAL( 42, LightControlSpy_getLastLightId() ); 
    TEST_ASSERT_EQUAL( LIGHT_ON, LightControlSpy_getLastState() ); 

    LightControl_destroy(); 
}

void testLightControIsInitialized(void){

    LightControl_init();

    TEST_ASSERT_EQUAL(LIGHT_ID_UNKNOWN, LightControlSpy_getLastLightId()); //ID == -1
    TEST_ASSERT_EQUAL(LIGHT_STATE_UNKNOWN, LightControlSpy_getLastState()); // STATE == -1
    
    LightControl_destroy();
}

void testLightControlIsDestroyed(void)
{
    LightControl_init();
    LightControl_on(42); 
    
    LightControl_destroy(); 
    
    TEST_ASSERT_EQUAL(LIGHT_ID_UNKNOWN, LightControlSpy_getLastLightId());
    TEST_ASSERT_EQUAL(LIGHT_STATE_UNKNOWN, LightControlSpy_getLastState());
}

void testLightControlSpyReturnsOnlyLastStateOFF(void){

    LightControl_init();

    LightControl_on(10);
    LightControl_off(21); // This should overwrite the 10

    TEST_ASSERT_EQUAL(21, LightControlSpy_getLastLightId());
    TEST_ASSERT_EQUAL(LIGHT_OFF, LightControlSpy_getLastState());

    LightControl_destroy();

}


