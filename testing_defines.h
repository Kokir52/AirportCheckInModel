#ifndef A2_TESTING_DEFINES_H
#define A2_TESTING_DEFINES_H

// ------------------------------------------------------------------------------------
typedef enum {
    INACTIVE,
    ACTIVE,
    RANDOM
} Mode;

// ------------------------------------------------------------------------------------
typedef struct {
    Mode baggage_drop;
    Mode maintenance;
    Mode self_check_in;
} TestingOptions;

// ------------------------------------------------------------------------------------
void dummyStartRoutine() {}

TestingOptions options = {RANDOM, RANDOM, RANDOM};

/* ... etc. Feel free to modify definitions.h for testing purposes and to test modularly, 
 * e.g., by using testing_defines or any other method that seems intuitive to you. :)
 */

#endif //A2_TESTING_DEFINES_H