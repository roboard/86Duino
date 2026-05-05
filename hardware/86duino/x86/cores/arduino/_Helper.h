#ifndef _HELPER_H
#define _HELPER_H

/*****************************************************************************/
/* SCOOP LIBRARY / AUTHOR FABRICE OUDERT / GNU GPL V3                        */
/* https://code.google.com/p/arduino-scoop-cooperative-scheduler-arm-avr/    */
/* VERSION 1.2  NEW YEAR PACK 10/01/2013                                     */
/* ENJOY AND USE AT YOUR OWN RISK  :)                                        */
/* SHOULD READ USER GUIDE FIRST (@!@)                                        */
/*****************************************************************************/

#include "io.h"
#include <setjmp.h>

/******** PREPROCESSING CONDITIONS ********/    
#define  HelperYIELDCYCLE    1        // if set to 1, yield will automatically launch all tasks in the list 
                                     // without coming back to main loop (like mySCoop.cycle() (faster when more than 1 task)
#define  HelperANDROIDMODE   2        // set to 1 if we want to have the possibility to use startLoop() 
                                     // set to 2 if we also want possibility to kill the task
#define  HelperOVERLOADYIELD 1        // set to 1 to provides a yield() global function which will overload standard arduino yield()

#define SCoopInstanceNickName1    myHelper   // could be changed for "Sch" or "SC" or whatever you prefer
#define ArduinoSchedulerNickName1 myHelperS // for compatibility with Arduino DUE library

#if (ARDUINO < 103)
#warning "V1.2 TESTED ONLY ON 1.0.3 with PanSTamp, Arduino UNO, Teensy++2.0, Teensy2.0 and Teensy3.0" 
#endif

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>                // not a valid approach for ARM
#endif

#define SCDelay_t           uint32_t  // type for all the virtual timer used in scoop library (period of timer, sleep function..)
#define SCoopTimerCount_t   int32_t  // define the type of the counter used in SCoopTimer. can be changed to int32_t instead

#define SCoopDefaultQuantum   200    // recomended before switching to next task. this provide a 1% overhead time used by scheduler, for 3 tasks+loop
#define SCoopDefaultStackSize (1024 * 1024)    // must be a multiple of 8
#define AndroidSchedulerDefaultStack (1024 * 1024) // a bit too much, just for backward compatibility reason

#define micros_t     uint32_t        // all low level micros second computation will be done in 32 bit too. possibility to change to int16
#define ptrInt       uint32_t        // used to typecast pointers to integer
typedef uint64_t     SCoopStack_t   __attribute__ ((aligned (8)));

// definition of the various state of an object or task. this was prefered to "enum" for using 8 bits instead of 16bits on AVR ...
#define SCoopTERMINATED  0
#define SCoopCONSTRUCTED B00001    // object state, compatible with Java library
#define SCoopNEW         B00010    // context ready
#define SCoopRUNNABLE    B00100    // bit 2 means the task is runnable or running : setup() has been launched and context ready
#define SCoopRUNNING     B00101    // inside run() or loop()
#define SCoopWAITING     B00110    // inside a sleep method
#define SCoopPAUSED      B01000    // bit 3 means the task is paused
#define SCoopTRIGGER     B10000    // force object to be launched when calling launch()
#define SCoopKILLING     B100000   // force object to be killed by Scheduler (or paused if static)

#define SCoopEventType   1         // used to provide a statical type information to the object in the list (polymorph)
#define SCoopTaskType    2         // only used by mySCoop.start() in the library code , as virtual call were prefered elsewhere
#define SCoopTimerType   3         // not used so far
#define SCoopDynamicTask 4         // 

/********* Objects Prototypes *******/

class _HelperTask;
class _Helper;

/********* GLOBAL VARIABLE *******/

extern _HelperTask*    _HelperFirstTask;      // point on the latest registered task
extern uint8_t        _HelperNumberTask;     // the number of tasks registered (main loop() not counted)

extern _Helper SCoopInstanceNickName1;       // one forced instance of the SCoop Scheduler
#if HelperANDROIDMODE >= 1
extern _Helper& ArduinoSchedulerNickName1;   // redundant declaration for compatibilit with the name of the Android/DUE "Scheduler"
#endif

extern void helperYield(void);         // used to overload the Arduino yield "weak"
extern void helperYield0(void);        // used to define our global yield(0)

#define SCoopClassOperatorEqual(name,type) name & operator=(const type rhs) { set(rhs); return *this; }; // magic statement tadah

/********* SCoopTASK CLASS *******/

class _HelperTask {
public:

    _HelperTask();                               // basic constructor
    _HelperTask(SCoopStack_t* stack, ptrInt size);
    
    ~_HelperTask();                              // just call terminate(). should be used only if a stack is made with malloc()
    
    void registerFunc(void (*userloop)(void));
    void (*loop)(void);                  // this is the call to user function. should be overloaded by a derived objects
    
    void yield();                              // this method is specific to the task. either return to scheduler, or switch to next task
    void yield0() { yield(0); }                // same but switch imediately without checking time
    void yield(micros_t quantum);              // same but force to check if the time passed is spent
    
    ptrInt stackLeft();                        // remaining stack space in this task
    void unregisterThis();
#if HelperANDROIDMODE >= 2
    void kill();                               // only works in conjunction with _Helper::startLoop for dynamic tasks
#endif  
    uint8_t *    pStack;                       // always point back and forth to the SP register for this task
    uint8_t *    pStackAddr;                   // keep a copy of the lowest stack adress. only used by stackleft()
    micros_t     quantumMicros;                // copy of the SCoopQuantum global definition, so the user can overload the value in setup()
    micros_t     prevMicros;                   // memorize the value of the micros() counter when entering the task. Works with quantumMicros
    jmp_buf      pEnv;

    uint8_t      itemType;
    volatile uint8_t state;
    _HelperTask  *pNext;
    
    virtual void start() __attribute__((optimize("O1"))); // initialize stack environement for calling run/loop. can be called by user if needed
    virtual bool launch() ;                    // launch the task from where it was stop, or just launch run/loop or user function the first time

    unsigned long long tempT;
    unsigned long long spentClocks;
    unsigned long prevRunTimes;
    unsigned long runTimes;
    unsigned long long averageClocks;
    
private:  // only internal methods used to optimize code size or readabilty
  
    void initBasic();                          // called by constructors. common code to each constructor variant
    void init(SCoopStack_t* stack, ptrInt size)// only called by constructor
    __attribute__((noinline));                 // optimize code instead of speed, as this is called only once...
    
    void  inline yieldInline(micros_t quantum)// potentially switch to pNext object, if time quantum given is reached
    __attribute__((always_inline));  
    
    void yieldSpent(micros_t spent)            // give control back to scheduler in order to switch to next task or come bacok in main loop()
    __attribute__((noinline));                 // speed optimization not that critical, as we already decided to switch
    
    void yieldSwitch()                         // just do it when you want to go to it
    __attribute__((noinline));
    
    inline void startFirstLoop()               // only used to simplify code reading. most likely the compiler will inline them
    __attribute__((always_inline));            // internal use only, to split cod into eementary function, facilitate inlining

                                             // total variable size = 12 on AVR and 22 on ARM if TIMEREPORT = 0
};                                           // total variable size = 16 on AVR and 30 on ARM if TIMEREPORT >=1

	
/******* MAIN SCoop CLASS ******/
class _Helper {                           // used only once for instanciating "mySCoop"
public:
    _Helper();                             // basic constructor
    
    void start(micros_t cycleTime);      // same as start but will compute a task quantum based on provided user expected cycle time. 
    void start(micros_t cycleTime, micros_t mainLoop);      // same as start but will compute a task quantum based on provided time. 
    void start();                        // start all registered objects in the list
#if HelperANDROIDMODE >= 1
    _HelperTask* startLoop(uint32_t stackSize = AndroidSchedulerDefaultStack); // dynamic task creation ... !
#endif  
    void yield();                        // can be called from where ever in order to Force the switch to next task
    void yield0();                       // can be called from where ever in order to Force the switch to next task
    
    jmp_buf     mainEnv;                 // used to store the main Stack register of the main loop()
    unsigned long long mainT;
    unsigned long long mainSpentClocks;
    unsigned long long scoopT;
    unsigned long long allSpentClocks;
    unsigned long cpuFreq;
    unsigned long prevMainRunTimes;
    unsigned long mainRunTimes;
    unsigned long long averageClocks;
    unsigned long long st;
    double prevBusyCPU;
    double getFreeCPU(void);
    double getBusyCPU(int TaskN = -2);
    unsigned long getTaskRunTimes(int TaskN = -2);
    
    _HelperTask * Current;                 // curent task in the yield cycle
    _HelperTask * Task;                    // task pointer
    volatile uint8_t Atomic;
    micros_t    startQuantum;            // initial value for each task time quantum. use default, otherwise calculated by start(x)
    micros_t    quantumMicros;           // initial value for the main loop time quantum. use default, otherwise calculated by start(x)
    micros_t    targetCycleMicros;       // this represent the target cycle time declared in the start(xxx), or the sum of all quantum
#if HelperYIELDCYCLE == 0
    micros_t    quantumMicrosReal;       // this variable is same as quantum micros but divided by number of tasks
#endif
};

#endif


