/*****************************************************************************/
/* SCOOP LIBRARY / AUTHOR FABRICE OUDERT / GNU GPL V3                        */
/* https://code.google.com/p/arduino-scoop-cooperative-scheduler-arm-avr/    */
/* VERSION 1.2  NEW YEAR PACK 10/1/2013                                      */
/* ENJOY AND USE AT YOUR OWN RISK  :)                                        */
/* SHOULD READ USER GUIDE FIRST (@\_/@)                                      */
/*****************************************************************************/

#include "io.h"
#include "SCoop.h"
#define SCINM SCoopInstanceNickName

/********* GLOBAL VARIABLE *******/

SCoopEvent*   SCoopFirstItem = NULL;           // has to be initialized here. hold a pointer on the whole list of task/event/timer...
SCoopEvent*   SCoopFirstTaskItem = NULL;       // has to be initialized here. points to the first of all tasks registered in the list
uint16_t      SCoopNumberTask = 0;             // hold the number of task registered. used to calculate quantum in start(xxx)


SCoop  SCoopInstanceNickName;                  // then we can use the library in the main sketch directly
#define SCINM SCoopInstanceNickName            // just a local nickname...
#if SCoopANDROIDMODE >= 1
SCoop& ArduinoSchedulerNickName = SCINM;       // this will create another identifier for the same object instance
#endif

/********* ASSEMBLY / LETS GET STARTED WITH THE COMPLEX THINGS **********/

// static void SCoopSwitch(jmp_buf* newP, jmp_buf* oldP) __attribute__((naked,noinline)) ;
static __inline__ void SCoopSwitch(jmp_buf* newP, jmp_buf* oldP) {
    if (setjmp(*oldP))
    {
        // do nothing
        return;
    }
    else  // jump to next thread
    {
        longjmp(*newP, 1);
        printf("error!!! should not return here...\n");
        while(1);
    }
};


#define ARM_ATOMIC ASM_ATOMIC 
#define AVR_ATOMIC 

#define SCoopMicros()   ((micros_t)((timer_GetClocks64()/CLOCKS_PER_MICROSEC)))     // overloading the standard micros()
#define SCoopDelayMillis()  ((SCDelay_t)timer_NowTime())  // overloading and typecasting the standard millis()

/********* SCOOPEVENT METHODS *******/

SCoopEvent::SCoopEvent() {
    init(NULL);                                  // initialize specific object variable
    registerThis(SCoopEventType);               // add to list and set state to Constructed
}

void SCoopEvent::registerThis(uint8_t type) {
    if (SCoopFirstItem == NULL) {
        pNext = NULL;                            // memorize the latest item registered
        SCoopFirstItem = this;                    // point the latest item to this one
    } else {
        register SCoopEvent* temp = SCoopFirstItem;
        while (temp->pNext != NULL) temp = temp->pNext;
        temp->pNext = this;
        pNext = NULL;
    }
    itemType = type;                             // just to memorize the object type, as we use polymorphism 
    scstate = SCoopCONSTRUCTED;                    // we are in the list and ready for a formal "init", either in the skecth or as a constructor extension
}

SCoopEvent::SCoopEvent(SCoopFunc_t func) {
    init(func); 
    registerThis(SCoopEventType);
}
  
SCoopEvent::~SCoopEvent() { // destructor : remove item from the list
    unregisterThis(); 
    if (SCoopFirstTaskItem == this) SCoopFirstTaskItem = pNext; // we do not need to change this if this is not the first task 
    // below section should be in Task Destructor, but didnt work there, probleme with chaining... so I put it here...
    if ((itemType == SCoopDynamicTask) || (itemType == SCoopTaskType)) {
        SCINM.targetCycleMicros -= reinterpret_cast<SCoopTask*>(this)->quantumMicros; // reduce target cycle time
        SCoopNumberTask--;	
#if SCoopYIELDCYCLE == 0	
        if (SCoopNumberTask>0) SCINM.quantumMicrosReal = quantumMicros / SCoopNumberTask;
#endif	
#if SCoopANDROIDMODE >=2
        if (itemType == SCoopDynamicTask)
            free(reinterpret_cast<SCoopTask*>(this)->pStackAddr);
#endif
	}
}
  
void SCoopEvent::unregisterThis() {              // remove item from SCoop list (needed for local objects)
    SCoopEvent * ptr = SCoopFirstItem;            // lets try first one
    if (ptr == this)
        SCoopFirstItem = ptr->pNext;// if this item is the last one registered
    else {
        do { 
            if (ptr->pNext == this) {                  // if the next is the one to remove
                ptr->pNext = ptr->pNext->pNext;      // skip it
                break;
            }                                      // we are done
        } while ((ptr=ptr->pNext));                  // try next item until we find the end of the list (NULL)                      
    }

    scstate = SCoopTERMINATED; 
}

void SCoopEvent::init(SCoopFunc_t func) {        // called by constructor, after registration
    userFunc = func;                           // hook call for the "run", if not overriden by a virtual void in child object
    if (func != NULL) scstate = SCoopNEW;         // this object is ready to get started or even launched (as launch() also call start())
}

void SCoopEvent::start() {                     // launched by scheduler when calling SCINM.start()
    if (scstate >= SCoopNEW) {
        setup();                                  // call the setup function, and do something only if a derived object has been created with this method.
        scstate = SCoopRUNNABLE;
    }                       
}

bool SCoopEvent::launch() {                    // launch or switch into this item or derived
    if (scstate & SCoopPAUSED) return false;       // check if item is suspended or not
    if (!(scstate & SCoopTRIGGER)) return false;
    SCoopATOMIC {
        scstate = SCoopRUNNING;                        // this also clear the trigger flag at the same time :)
        run();
        scstate = SCoopRUNNABLE;                     // an event shouldnt pause itself so lets go with RUNNABLE
    }
    return true;                                 // has been launched
}

void SCoopEvent::pause() {                      // pausing an event just set the state to PAUSED
    if (scstate >= SCoopRUNNABLE) scstate |= SCoopPAUSED;
}

void SCoopEvent::resume() {                     // resuming an event just clear the flag PAUSED ... might be not enough for user code
    if (scstate & SCoopPAUSED) scstate &= ~SCoopPAUSED;
}

bool SCoopEvent::paused() {                     // just return the pause flag status
    if (scstate & SCoopPAUSED)
        return true;
    else
        return false;
}    


/********* SCoopDelay CLASS *******/               // a basic virtual timer implementation. sort of "timerdown"
                                               // very small size code in each method. compiler will probably inline and clone everything
SCoopDelay::SCoopDelay() {             
    reset();
}
  
SCoopDelay::SCoopDelay(SCDelay_t reload) {
    set(setReload(reload));
}

SCDelay_t  SCoopDelay::setReload(SCDelay_t reload) {
    return (reloadValue = reload);
}

SCDelay_t SCoopDelay::getReload() {
    return reloadValue;
}

void SCoopDelay::initReload() {
    set(reloadValue);
}

void SCoopDelay::reload() {  
    timeValue += reloadValue;
}

bool SCoopDelay::reloaded() {
    if (elapsed()) {
        reload();
        return true;
    }
    return false;
}

void SCoopDelay::reset() {
    set(0);
}

SCDelay_t SCoopDelay::set(SCDelay_t time) {
    return (timeValue = time + SCoopDelayMillis());
}

SCDelay_t SCoopDelay::get() {
    register SCDelay_t temp =(timeValue - SCoopDelayMillis()); 
    if (temp < 0) return 0; else return temp;
}

SCDelay_t SCoopDelay::add(SCDelay_t time) {
    timeValue += time; return time;
}

SCDelay_t SCoopDelay::sub(SCDelay_t time) {
    timeValue -= time; return time;
}

bool SCoopDelay::elapsed() {
    return (get() == 0);
}


/********* SCoopDelayUS CLASS *******/               // a basic virtual timer implementation. sort of "timerdown"
                                               // very small size code in each method. compiler will probably inline and clone everything
SCoopDelayus::SCoopDelayus() {               
    reset();
}
  
SCoopDelayus::SCoopDelayus(micros_t reload) {
    set(setReload(reload));
}

micros_t  SCoopDelayus::setReload(micros_t reload) {
    return (reloadValue = reload);
}

micros_t SCoopDelayus::getReload() {  
    return reloadValue;
}

void SCoopDelayus::initReload() {
    set(reloadValue);
}

void SCoopDelayus::reload() { 
    timeValue += reloadValue;
}

bool SCoopDelayus::reloaded() {
    if (elapsed()) {
        reload();
        return true;
    }
    return false;
}

void SCoopDelayus::reset() {
    set(0);
}

micros_t SCoopDelayus::set(micros_t time) {
    return (timeValue = time + SCoopMicros());
}

micros_t SCoopDelayus::get() {
    register micros_t temp =(timeValue - SCoopMicros()); 
    if (temp <0) return 0; else return temp;
}

micros_t SCoopDelayus::add(micros_t time) {
    timeValue += time; return time;
}

micros_t SCoopDelayus::sub(micros_t time) {
    timeValue -= time; return time;
}

bool SCoopDelayus::elapsed() {
    return (get() == 0);
}



/********* SCoopTimer METHODS *******/

SCoopTimer::SCoopTimer() : SCoopEvent() {
    initBasic();
    init(0,NULL);
}

SCoopTimer::SCoopTimer(SCDelay_t period) : SCoopEvent() {
    initBasic();
    init(period, NULL);
}

SCoopTimer::SCoopTimer(SCDelay_t period, SCoopFunc_t func) : SCoopEvent() {
    initBasic();
    init( period, func);
}

void SCoopTimer::initBasic() {
    counter    = -1; 
    userFunc   = NULL;
    itemType   = SCoopTimerType;
}

void SCoopTimer::init(SCDelay_t period, SCoopFunc_t func) {
    timer.setReload(period); timer.reset(); 
    userFunc = func; 
    if (func != NULL) 
        scstate = SCoopNEW;  // we can use this NEW state as the user function is now defined
}

void SCoopTimer::start() {
    SCoopEvent::start(); 
    timer.initReload();   // make sure the timer is starting with the reload period value
}

bool SCoopTimer::launch() {
    if ((counter == 0) || (timer.getReload() == 0)) return false;
    if (timer.reloaded()) {
        scstate |= SCoopTRIGGER;
        register bool launched = SCoopEvent::launch();
        if ((launched ) && (counter > 0)) counter--;
            return launched; // rearm next run time so timers are NOT desynchronized by pause(). (my default choice)       
    }
    return false;
}


SCDelay_t SCoopTimer::getTimeToRun() {
    if ((counter == 0) || (timer.getReload() == 0)) return -1;
    return timer.get();
}

void SCoopTimer::setTimeToRun(SCDelay_t time) {
    timer.set(time);
}

void SCoopTimer::schedule(SCDelay_t time, SCoopTimerCount_t count) {
    timer.set(timer.setReload(time));
    counter = count;
}

void SCoopTimer::schedule(SCDelay_t time) {
    schedule(time, -1);
}


/********* SOME BASIC FUNCTIONS *******/

void SCoopMemFill(uint8_t *startp, uint8_t *endp, uint8_t v) {
    if (startp) while (startp < endp) *startp++ = v;
}  
    
ptrInt SCoopMemSearch(uint8_t *startp, uint8_t *endp, uint8_t v) {
    uint8_t *ptr = startp;
    while (ptr < endp) if (*ptr++ != v) break;
    return ((ptrInt)ptr - (ptrInt)startp - 1); 
}

/********* SCoopTASK METHODS *******/

// CONSTRUCTORS

SCoopTask::SCoopTask() : SCoopEvent() {
    initBasic();
}           

SCoopTask::SCoopTask(SCoopStack_t* stack, ptrInt size) : SCoopEvent() {
    initBasic(); init(stack,size);
}     

SCoopTask::SCoopTask(SCoopStack_t* stack, ptrInt size, SCoopFunc_t func) : SCoopEvent() {
    initBasic(); init(stack,size,func);
}     

void SCoopTask::initBasic() {
    SCoopNumberTask++;
    pStackAddr = NULL;
    pStack     = NULL;
    userFunc   = NULL;

    if (SCoopFirstTaskItem == NULL) {
        if (SCoopFirstItem != NULL) {
            register SCoopEvent* temp = SCoopFirstItem;
            while (temp->pNext != NULL) temp = temp->pNext;
            SCoopFirstTaskItem = temp;
        }
    }
    
    itemType = SCoopTaskType;
}


void SCoopTask::init(SCoopStack_t* stack, ptrInt size) {      
    pStackAddr = (uint8_t*)stack; 
    pStack = (uint8_t*)stack + ((size-sizeof(SCoopStack_t)) & ~15); // align 16 bytes
    SCoopMemFill((uint8_t*)stack, pStack, 0x55); // fill with 0x55 patern in order to calculate StackLeft later
};

	
void SCoopTask::init(SCoopStack_t* stack, ptrInt size, SCoopFunc_t func) { // only this one can be called by user
    init(stack,size); 
    userFunc = func; 
    if (func != NULL) 
        scstate = SCoopNEW;                       // we have a stack and a user function so we can "start" later.
}

SCoopTask::~SCoopTask(){ }                     // destructor to remove task from list .. doesnt really work with "delete"

/******** START and LAUNCH SECTION ****************/
SCoopTask** _global_thisP; // 86Duino
int thisPIndex0 = 0, thisPIndex1 = 0; // 86Duino
void SCoopTask::start() {
    if (pStack) {                                       // sanity check if stack has been allocated by user or constructor ...
        if ((scstate & SCoopNEW)) {                       // if the task context is not yet set
            _global_thisP[thisPIndex0++] = this; // 86Duino
            if (setjmp(pEnv)) {  // simulate switching but with current context : back to same place !  
                SCoopTask* tmp = _global_thisP[thisPIndex1++]; // 86Duino
                if (tmp->scstate & SCoopRUNNABLE) {                 // this will be executed only when we comeback here with a backToTask the very first time
                    tempT = timer_GetClocks64();
                    spentClocks = prevRunTimes = runTimes = averageClocks = 0;
                    tmp->startFirstLoop();                       // quite equivalent to a "setjmp" mechanism
                }
            } else {
                pEnv->__esp = (unsigned long) (pStack - 4); // that will pop a pEvn pointer (4 bytes) form stack array when jump to setjmp() form longjmp(), so we minus  4 let it to pop.
                *(uint32_t*)pStack = (uint32_t) (pEnv); // In fact, this value is not used by CPU, the SP just pop 4 Bytes and do nothing
            }                                           // we can restore interrupts as we are finished with critical stack handling
		}                                           // continue forward to launch setup
        
        SCoopEvent::start();                            // call the user setup function (if defined in derived object) and set object RUNNABLE     
        quantumMicros = SCINM.startQuantum;             // initialize quantum time provided by start (xx) or by user or by default
        SCINM.targetCycleMicros += quantumMicros;       // cumulate time to calculate target cycle time
        prevMicros = SCoopMicros();                     // memorize time , to calculate time spent in the task and in the cycle
        timer = 0;                                    // this will enable imediate user call to sleepSync to work properly 
    }
} // end start()

void SCoopTask::startFirstLoop() {                   // will execute this function the first call to backToTask() made by yield() 
    scstate = SCoopRUNNING;                   
    while (true)  {                                    // a SCoop task will never end ...
        if (!(scstate & SCoopPAUSED)) {
            loop();                                   // call user function (derived virtual loop or user adress)
            runTimes++; // 真的做完loop一次才加一，不是每次呼叫 yield() 加一
            yieldInline(quantumMicros);                // try to switch task if we reach the end of the user loop
        } else 
            yield(0);                                  // switch imediately to next task (or scheduler) if we are paused
    }
}

bool SCoopTask::launch() {
	if (scstate & SCoopRUNNABLE) {                   // make sure the task context is setup first and start() has been called already 
        if (!(scstate & (SCoopPAUSED | SCoopKILLING))) {
            coroutineState = IN_SCOOP;       // 86Duino
            SCINM.Task = this;                     // we always can find a pointer to the current task in which we are running
            SCoopSwitch(&pEnv, &SCINM.mainEnv);
            return true;
        } else                    // return to scheduler / yield() or cycle()
            prevMicros = SCoopMicros();            // just to avoid jeopardizing the cycleMicros in fact
	} else if (scstate & SCoopNEW)
        start();              // initialize context if not done in the main arduino setup() section ...
    return false;
}                               

#if SCoopANDROIDMODE >= 2  
void SCoopTask::kill() {                          
    if (itemType == SCoopDynamicTask) 
    scstate |= (SCoopKILLING );
    if (mySCoop.Task == this) yieldSwitch();      // quick return to main scheduler for treating the situation
}
#endif
  
  
/******** YIELD SECTION ****************/

void SCoopTask::yield() {                           // check if the quantum time alowed is elapsed, the switch to scheduler
    yieldInline(quantumMicros);
}

void SCoopTask::yield(micros_t quantum) {
    yieldInline(quantum);
}


void SCoopTask::yieldInline(micros_t quantum) {
    if (quantum) {
        register micros_t spent = SCoopMicros() - prevMicros;
        if (spent >= quantum) yieldSpent(spent);   // switch makes sense	   
    } else if (!SCINM.Atomic) {
        if (SCINM.Task != NULL) { // 結算現在的
            SCINM.Task->spentClocks += (timer_GetClocks64() - SCINM.Task->tempT);
        } // 沒有 else，因為設定 SCINM.Task = NULL 的地方在 516 行
        
        yieldSwitch(); // we ve been called from a task context lets yield from there
        
        if (SCINM.Task != NULL) // 重置新 Task time
            SCINM.Task->tempT = timer_GetClocks64();
        else
            SCINM.mainT = timer_GetClocks64();
    }
}
  
  
void SCoopTask::yieldSpent(micros_t spent) {       // immediate jump back to scheduler
    if (SCINM.Atomic) return;                      // do nothing if we are in a atomic section
    
    if (SCINM.Task != NULL) { // 結算現在的
        SCINM.Task->spentClocks += (timer_GetClocks64() - SCINM.Task->tempT);
    } // 沒有 else，因為設定 SCINM.Task = NULL 的地方在 516 行

    yieldSwitch(); // we ve been called from a task context lets yield from there
    
    if (SCINM.Task != NULL) // 重置新 Task time
        SCINM.Task->tempT = timer_GetClocks64();
    else
        SCINM.mainT = timer_GetClocks64();
}

void SCoopTask::yieldSwitch() { 
	register SCoopEvent* temp;
    
	if ((SCoopYIELDCYCLE == 1) &&                    // optimize speed by directly switching next adjacent task
	   ((temp=pNext) != NULL) &&                     // only if possible, otherwise back to main loop
       ((temp->scstate & (SCoopRUNNABLE | SCoopPAUSED | SCoopKILLING)) == SCoopRUNNABLE))	{
           SCINM.Current = temp;
           SCINM.Task = reinterpret_cast<SCoopTask*>(temp);          // lets go next
           SCoopSwitch(&(reinterpret_cast<SCoopTask*>(temp)->pEnv), &pEnv);    // save our context and use next one
    } else {                                           // systematically return to main loop or scheduler if using cycle()
        coroutineState = IN_MAIN_LOOP;
        SCINM.Task = NULL;
        SCoopSwitch(&SCINM.mainEnv, &pEnv);        // save context and return to main scheduler
    }												 // will return here by launch() from scheduler yield() or cycle()
    prevMicros = SCoopMicros();
}                                               // come back into the task HERE / NOW

/******** SLEEP SECTION ****************/

   
void SCoopTask::sleep(SCDelay_t ms) {                // will replace your usual arduino "delay" function
    sleepMs(ms,false);
}

void SCoopTask::sleepSync(SCDelay_t ms) {            // same as Sleep but delays are not absolute but relative to previous call to SleepSync
    sleepMs(ms, true);
}


void SCoopTask::sleepMs(SCDelay_t ms, bool sync) { 
    if (ms < 1) {
        timer.reset();
        return;
    }
    
    if (sync) timer.add(ms); else timer.set(ms);
    scstate = SCoopWAITING;
    while (timer) yield(0); 
    scstate = SCoopRUNNING;
}

	
bool SCoopTask::sleepUntil(vbool& var, SCDelay_t timeOut) { // just wait for an "external" variable to become true, with a timeout
    register bool temp = false;
    if (timeOut) {
        timer.set(timeOut);
        temp = true;
    }
    return sleepUntilBool(var, temp);
}

void SCoopTask::sleepUntil(vbool& var) {      // just wait for an "external" variable to become true
    sleepUntilBool(var, false);
}

bool SCoopTask::sleepUntilBool(vbool& var, bool checkTime) {             // just wait for an "external" variable to become true
    scstate = SCoopWAITING; 
    while(!var) {
        yield(0);
        if (checkTime) 
            if (timer.elapsed()) { scstate = SCoopRUNNING; return false; }
    }
    
    scstate = SCoopRUNNING;
    var = false;
    return true;
}


ptrInt SCoopTask::stackLeft() {
    if (pStackAddr)                          // sanity check if stack has been initialized
        return SCoopMemSearch(pStackAddr, pStack, 0x55);
    else
        return 0;
}


/********* SCoop METHODS *******/
  
SCoop::SCoop() {       // constructor
    startQuantum      = SCoopDefaultQuantum;     // every task will get the default value unless changed in their setup or with sart()
    quantumMicros     = SCoopDefaultQuantum;     // the main loop also
#if SCoopYIELDCYCLE == 0
    quantumMicrosReal = 0;                       // we do nt know yet the number of task registered
#endif
	targetCycleMicros = 0;                       //  we do nt know yet the total cycle time

    Current = NULL; 
	Task    = NULL; 
	Atomic  = 1;                               // ensure yield is not activated
}

void SCoop::start(micros_t cycleTime, micros_t mainLoop) {  // define the total length of the cycle and the main loop quantum
    if (cycleTime < 0) cycleTime = 0;
    if (mainLoop < 0) mainLoop = 0;
    if (cycleTime < mainLoop) mainLoop = cycleTime;
    startQuantum = (cycleTime - mainLoop) / (SCoopNumberTask); 
    quantumMicros = mainLoop;                                // specific time for main loop
    coroutineState = IN_MAIN_LOOP; // 86Duino
    start();
}


void SCoop::start(micros_t cycleTime) {          // define the total length of the cycle for all tasks+1
    if (cycleTime < 0) cycleTime = 0;
    startQuantum = cycleTime / (SCoopNumberTask+1); // consider the main loop as a task
    quantumMicros = startQuantum;                 // time for the main loop will be the same then
    start();
}
  

void SCoop::start() {                          // start all objects in the list
    if (SCoopNumberTask == 0) return;
    
    targetCycleMicros = quantumMicros;            // initialization
#if SCoopYIELDCYCLE == 0
    quantumMicrosReal = quantumMicros / SCoopNumberTask;  // divide time in slice, as we come back here at each task switch...
#endif
    _global_thisP = new SCoopTask*[SCoopNumberTask]; // 86Duino
    
    Current = SCoopFirstItem;                     // take the first
	while (Current) {
	  Current->start();                           // start this task (initialize environement and call setup())
      Current = Current->pNext;
    }                // take next ptr
    
	Atomic = 0;                          // ready for switchiching task with "yield"
    cpuFreq = vx86_CpuCLK();
    mainT = scoopT = timer_GetClocks64(); // start to run loop() after this line
    mainSpentClocks = allSpentClocks = prevMainRunTimes = mainRunTimes = 0;
    prevBusyCPU = 0.0;
}

  // this is the main code for the Scheduler, relying on yield() method as a state machine
  
void SCoop::yield0() {                        // can be called from where ever in order to Force the switch to next task
    if (Task) Task->yield(0); else SCoop::yield();
}

void SCoop::yield() {                         // can be called from where ever in order to Force the switch to next task
    if (SCoopNumberTask) {
        
        coroutineState = IN_SCOOP; // 86Duino
        
        st = timer_GetClocks64() - scoopT;
        
        if (st > (cpuFreq * 1000000LL)) {
            SCoopEvent* temp = NULL;
            temp = SCoopFirstTaskItem;
            // int i = 0;
            while (temp != NULL) {
                if (temp->runTimes > 0) {
                    temp->prevRunTimes = temp->runTimes;
                    temp->averageClocks = temp->spentClocks / temp->runTimes;
                    allSpentClocks += temp->averageClocks;
                }
                // printf("%d>%llu ", i++, temp->spentClocks);
                temp->spentClocks = temp->runTimes = 0;
                temp = temp->pNext;
            }
            if (mainRunTimes > 0) {
                prevMainRunTimes = mainRunTimes;
                averageClocks = mainSpentClocks / mainRunTimes;
                allSpentClocks += averageClocks;
            }
            // printf("%llu \n", mainRunTimes);
            mainSpentClocks = mainRunTimes = 0;
            prevBusyCPU = ((double)allSpentClocks / (double)st) * 100.0;
            allSpentClocks = 0;
            scoopT = timer_GetClocks64();
        }
    }
    
    if (Task) {
        Task->yield();                  // we ve been called from a task context lets yield from there
    } else {
        if (Atomic) return;                      // self explaining
    
        register SCoopEvent* temp = SCoopFirstItem;
        // while (temp != SCoopFirstTaskItem) { temp->launch(); temp = temp->pNext; }  // launch all events
        
        register micros_t time;
        if (Current == NULL) {                   // a cycle is completed
            coroutineState = IN_MAIN_LOOP;
            temp = SCoopFirstTaskItem;
            if (temp == NULL) return;             // no tasks in the list !

            // check overall target cycle time before launching first task
    
            if (quantumMicros) {                   // check if we are supposed to spend some time in the main loop or not
                time = SCoopMicros() - reinterpret_cast<SCoopTask*>(temp)->prevMicros; // mesure whole cycle length based on first task information
                if (time < targetCycleMicros) return;   // back in main loop() until we reach the expected target cycle time
            }
            Current = temp;
		} else { // lets check intertask timing before launching the next 
#if SCoopYIELDCYCLE == 0
            if (quantumMicrosReal) {              // check if we should spend quite some time in the main loop between 2 tasks
                time = SCoopMicros() 
                    - reinterpret_cast<SCoopTask*>(Current)->prevMicros // give the time since last task executed
                    - reinterpret_cast<SCoopTask*>(Current)->quantumMicros; // mesure whole cycle length based on first task information
                if (time < quantumMicrosReal) return;   // back in main loop() until we reach the expected target cycle time           
            }
#endif
		}
        
        coroutineState = IN_SCOOP; // 86Duino
        // mainSpentClocks += (timer_GetClocks64() - mainT);
        temp->tempT = timer_GetClocks64(); // 先重置在前
        temp->launch();
        Current = NULL;
        mainT = timer_GetClocks64();
        /*
        do { temp = Current; 
            temp->launch(); // now launch tasks from the list, and may be all in a single launch()
            Current = temp->pNext;   
#if SCoopANDROIDMODE >= 2                    // check if we autorize the killme
            if ((temp->scstate & SCoopKILLING) && (temp->itemType == SCoopDynamicTask))			     	     				 
                delete temp;	// killing done		 
#endif
#if SCoopYIELDCYCLE == 0	                   // back to main task if we are not in yield cycle mode
            return; 
#endif	                                       // otherwise we just go back into the main loop      
	    } while (Current);
        */
	}
}

  
void SCoop::cycle() {                         // execute a complete cycle across all tasks & events 
    do { yield(); } while (Current);
}
  
void SCoop::sleep(SCDelay_t time) {  
    SCoopDelay SCoopSleepTimer;
    SCoopSleepTimer = time;
    while (SCoopSleepTimer) yield();
}

   
void SCoop::delay(uint32_t ms) {
    uint32_t nowT = SCoopDelayMillis();
    while ((SCoopDelayMillis() - nowT) < ms) yield();
}


#if SCoopANDROIDMODE >= 1
SCoopTask* SCoop::startLoop(SCoopFunc_t func, uint32_t stackSize) {
	uint8_t *stack = (uint8_t*)malloc(stackSize);
	if (!stack) return NULL;

	SCoopTask *task = new SCoopTask((SCoopStack_t*)stack,stackSize,func);
	if (!task) {
		free(stack);
		return NULL;
    }

    // object now exist and is registered in the list!
	task->itemType = SCoopDynamicTask;
	return task;
}
#endif

double SCoop::getFreeCPU() {
    
    if (prevBusyCPU > 0.0) {
        if (prevBusyCPU <= 100.0)
            return 100.0 - prevBusyCPU;
        else
            return 0.0;
    }
    
    return -1;
}

double SCoop::getBusyCPU(int TaskN) {
    SCoopEvent* temp = NULL;
    int i = 1; // TaskN 從 1 開始, -1 是 main loop
    
    if (TaskN != -2) {
        
        if (TaskN == -1) return ((double)averageClocks / (cpuFreq * 1000000.0)) * 100.0;
        
        temp = SCoopFirstTaskItem;
        while (temp != NULL) {
            if (i == TaskN) return ((double)temp->averageClocks / (cpuFreq * 1000000.0)) * 100.0;
            temp = temp->pNext;
            i++;
        }
        return -2; // Not found
    } else if (prevBusyCPU > 0.0) {
        if (prevBusyCPU <= 100.0)
            return prevBusyCPU;
        else
            return 100.0;
    }
    
    return -1;
}

unsigned long SCoop::getTaskRunTimes(int TaskN) {
    SCoopEvent* temp = NULL;
    int i = 1; // TaskN 從 1 開始, -1 是 main loop
    unsigned long total = 0;
    
    if (TaskN != -2) {
        
        if (TaskN == -1) return prevMainRunTimes;
        
        temp = SCoopFirstTaskItem;
        while (temp != NULL) {
            if (i == TaskN) return temp->prevRunTimes;
            temp = temp->pNext;
            i++;
        }
        return -2; // Not found
    }

    return round((1.0 / prevBusyCPU) * 100.0);
}

/*************** OVERLOAD STANDARD YIELD *****************/


#if SCoopOVERLOADYIELD == 1
void yield(void)  { SCINM.yield(); };              // overload standard arduino yield
void yield0(void) { SCINM.yield0(); };             // overload standard arduino yield
void mainLoopRunOneTime(void) { SCINM.mainRunTimes++; };
void stopMainLoopTimerInSCoop(void) { SCINM.mainSpentClocks += (timer_GetClocks64() - SCINM.mainT); };
#endif

void __attribute__((weak)) sleep (SCDelay_t time) {
    SCINM.sleep(time);
}

/*************** FIFO *****************/

SCoopFifo::SCoopFifo(void * fifo, const uint8_t itemSize, const uint16_t itemNumber) {
    this->itemSize = itemSize;
    ptrMin = (uint8_t*)fifo;   
    ptrIn  = (uint8_t*)fifo;   
    ptrOut = (uint8_t*)fifo;   
    ptrMax = (uint8_t*)fifo + (itemNumber * itemSize);
}


uint16_t SCoopFifo::count() {                      // return the number of item currently in the fifo
    register int16_t temp;
    
    AVR_ATOMIC { temp = (ptrIn - ptrOut); }
    return (temp < 0 ? (temp + ptrMax-ptrMin) : temp);
}


bool SCoopFifo::put(void* var) {                    // put a record in the pifo and return true if ok or false if fifo is full
    register uint8_t N = itemSize;
    register uint8_t* dest;
    register uint8_t* source;
    
    AVR_ATOMIC { dest = ptrIn; }
    register uint8_t* post = dest + N;
    if (post >= ptrMax) { post = ptrMin; }
    
    AVR_ATOMIC { source = ptrOut; }
    if (post != source) { // no overload
        source = (uint8_t*)var;        
        do { *dest++ = *source++; } while (--N);
        AVR_ATOMIC { ptrIn = post; }
        return true;   // ok
    } else
        return false;  // fifo was full
}

bool SCoopFifo::putChar(const uint8_t value) {
   uint8_t X = value;  return put(&X);
}

bool SCoopFifo::putInt(const uint16_t value) {
   uint16_t X = value;  return put(&X);
}

bool SCoopFifo::putLong(const uint32_t value) {
   uint32_t X = value;  return put(&X);
}


bool SCoopFifo::get(void* var) {                    // retreive one record from the fifo, if not empty otherwise return false
    register uint8_t* In;
        register uint8_t* source;
        
        AVR_ATOMIC { In = ptrIn; source = ptrOut; }
        if (In != source) {
            register uint8_t N = itemSize;
            register uint8_t* dest = (uint8_t*)var;      
            do { *dest++ = *source++; } while (--N) ;
            if (source >= ptrMax) { source = ptrMin; }
            AVR_ATOMIC { ptrOut = source; }
            return true;         // ok
        } else 
            return false; // fifo was empty
    } 


void SCoopFifo::getYield(void* var) {               // same as get(var) but wait until fifo is not empty, and calls yield()
    register uint8_t* In;
    register uint8_t* Out;
    
    while (true) { 
        AVR_ATOMIC { In = ptrIn; Out = ptrOut; }
        if (In == Out) yield(); else break;
    }
    get(var);
}


uint8_t SCoopFifo::getChar() {
    uint8_t result8; 
    getYield(&result8); return result8;
}

uint16_t SCoopFifo::getInt() {
    uint16_t result16; 
    getYield(&result16); return result16;
}

uint32_t SCoopFifo::getLong() {
    uint32_t result32; 
    getYield(&result32); return result32;
}

uint16_t SCoopFifo::flush() {                      // empty the fifo
    ASM_ATOMIC {                                     // this will de activate interrupts
        ptrIn  = ptrMin;
        ptrOut = ptrMin;                            // this will ACTIVATE interrupts                            
    }
    return (ptrMax - ptrMin);
}

uint16_t SCoopFifo::flushNonAtomic() {             // empty the fifo
    ptrIn  = ptrMin;
    ptrOut = ptrMin;
    return (ptrMax - ptrMin);
}

  
