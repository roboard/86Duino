/*****************************************************************************/
/* SCOOP LIBRARY / AUTHOR FABRICE OUDERT / GNU GPL V3                        */
/* https://code.google.com/p/arduino-scoop-cooperative-scheduler-arm-avr/    */
/* VERSION 1.2  NEW YEAR PACK 10/1/2013                                      */
/* ENJOY AND USE AT YOUR OWN RISK  :)                                        */
/* SHOULD READ USER GUIDE FIRST (@\_/@)                                      */
/*****************************************************************************/

#include "_Helper.h"

/********* GLOBAL VARIABLE *******/
_HelperTask*  _HelperFirstTaskItem = NULL;       // has to be initialized here. points to the first of all tasks registered in the list
uint8_t       _HelperNumberTask = 0;             // hold the number of task registered. used to calculate quantum in start(xxx)

_Helper  SCoopInstanceNickName1;                  // then we can use the library in the main sketch directly
#define SCINM SCoopInstanceNickName1            // just a local nickname...
#if HelperANDROIDMODE >= 1
_Helper& ArduinoSchedulerNickName1 = SCINM;       // this will create another identifier for the same object instance
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

/********* SOME BASIC FUNCTIONS *******/

static void SCoopMemFill(uint8_t *startp, uint8_t *endp, uint8_t v) {
    if (startp) while (startp < endp) *startp++ = v;
}  
    
static ptrInt SCoopMemSearch(uint8_t *startp, uint8_t *endp, uint8_t v) {
    uint8_t *ptr = startp;
    while (ptr < endp) if (*ptr++ != v) break;
    return ((ptrInt)ptr - (ptrInt)startp - 1); 
}

/********* _HelperTASK METHODS *******/

// CONSTRUCTORS
_HelperTask::_HelperTask() {
}           

_HelperTask::_HelperTask(SCoopStack_t* stack, ptrInt size) {
    init(stack,size);
}

void _HelperTask::registerFunc(void (*userloop)(void)) {
    _HelperNumberTask++;
    loop = userloop;
    pNext = _HelperFirstTaskItem;                 // register in the task list     
    _HelperFirstTaskItem = this;
    state = SCoopNEW;
}


void _HelperTask::init(SCoopStack_t* stack, ptrInt size) {      
    pStackAddr = (uint8_t*)stack; 
    pStack = (uint8_t*)stack + ((size-sizeof(SCoopStack_t)) & ~15); // align 16 bytes
    SCoopMemFill((uint8_t*)stack, pStack, 0x55); // fill with 0x55 patern in order to calculate StackLeft later
};

void _HelperTask::unregisterThis() {              // remove item from SCoop list (needed for local objects)
    _HelperTask * ptr = _HelperFirstTaskItem;            // lets try first one
    if (ptr == this)
        _HelperFirstTaskItem = ptr->pNext;// if this item is the last one registered
    else {
        do { 
            if (ptr->pNext == this) {                  // if the next is the one to remove
                ptr->pNext = ptr->pNext->pNext;      // skip it
                break;
            }                                      // we are done
        } while ((ptr=ptr->pNext));                  // try next item until we find the end of the list (NULL)                      
    }

    state = SCoopTERMINATED; 
}

_HelperTask::~_HelperTask(){
    unregisterThis();
    if (_HelperFirstTaskItem == this) _HelperFirstTaskItem = pNext; // we do not need to change this if this is not the first task 
    // below section should be in Task Destructor, but didnt work there, probleme with chaining... so I put it here...
    if ((itemType == SCoopDynamicTask) || (itemType == SCoopTaskType)) {
        SCINM.targetCycleMicros -= this->quantumMicros; // reduce target cycle time
        _HelperNumberTask--;
        if (_HelperNumberTask == 0) SCINM.Atomic = 1;
#if HelperYIELDCYCLE == 0	
        if (_HelperNumberTask>0) SCINM.quantumMicrosReal = quantumMicros / _HelperNumberTask;
#endif	
#if HelperANDROIDMODE >=2
        if (itemType == SCoopDynamicTask)
            free(this->pStackAddr);
#endif
	}
}                     // destructor to remove task from list .. doesnt really work with "delete"

/******** START and LAUNCH SECTION ****************/
// static _HelperTask** _global_thisP; // 86Duino : save "this" pointer of class in global var.
                            //NOTE: we dont use this mechanism  in current version.
                            //             We assume the "this" pointer is saved in CPU registers for DJGPP.
// static int thisPIndex0 = 0, thisPIndex1 = 0; // 86Duino
void _HelperTask::start() {
    if (pStack) {                                       // sanity check if stack has been allocated by user or constructor ...
        if ((state & SCoopNEW)) {                       // if the task context is not yet set
            // _global_thisP[thisPIndex0++] = this; // 86Duino : push "this" pointer with 1.2.3.4.5...
            // thisPIndex1 = thisPIndex0; // 86Duino : becuase the latest data always be in head of our link list, so we will pop it from the highest index, not zero.
            if (setjmp(pEnv)) {  // simulate switching but with current context : back to same place !  
                // _HelperTask* tmp = _global_thisP[--thisPIndex1]; // 86Duino : pop "this" pointer with 5.4.3.2.1...
                if (state & SCoopRUNNABLE) {                 // this will be executed only when we comeback here with a backToTask the very first time
                    tempT = timer_GetClocks64();
                    spentClocks = prevRunTimes = runTimes = averageClocks = 0;
                    startFirstLoop();                       // quite equivalent to a "setjmp" mechanism
                }
            } else {
                pEnv->__esp = (unsigned long) (pStack - 4); // that will pop a pEvn pointer (4 bytes) form stack array when jump to setjmp() form longjmp(), so we minus  4 let it to pop.
                *(uint32_t*)pStack = (uint32_t) (pEnv); // In fact, this value is not used by CPU, the SP just pop 4 Bytes and do nothing
            }                                           // we can restore interrupts as we are finished with critical stack handling
		}                                           // continue forward to launch setup
        
        state = SCoopRUNNABLE;
        quantumMicros = SCINM.startQuantum;             // initialize quantum time provided by start (xx) or by user or by default
        SCINM.targetCycleMicros += quantumMicros;       // cumulate time to calculate target cycle time
        prevMicros = SCoopMicros();                     // memorize time , to calculate time spent in the task and in the cycle
        SCINM.Atomic = 0;
    }
} // end start()

void _HelperTask::startFirstLoop() {                   // will execute this function the first call to backToTask() made by yield() 
    state = SCoopRUNNING;    
    while (true)  {                                    // a SCoop task will never end ...
        if (!(state & SCoopPAUSED)) {
            loop();                                     // call user function (derived virtual loop or user adress)
            runTimes++; // 真的做完loop一次才加一，不是每次呼叫 yield() 加一
            yieldInline(quantumMicros);                // try to switch task if we reach the end of the user loop
        } else 
            yield(0);                                  // switch imediately to next task (or scheduler) if we are paused
    }
}

bool _HelperTask::launch() {
	if (state & SCoopRUNNABLE) {                   // make sure the task context is setup first and start() has been called already 
        if (!(state & (SCoopPAUSED | SCoopKILLING))) {
            coroutineState = IN_HELPER;       // 86Duino
            SCINM.Task = this;                     // we always can find a pointer to the current task in which we are running
            SCoopSwitch(&pEnv, &SCINM.mainEnv);
            return true;
        } else                    // return to scheduler / yield() or cycle()
            prevMicros = SCoopMicros();            // just to avoid jeopardizing the cycleMicros in fact
	} else if (state & SCoopNEW)
        start();              // initialize context if not done in the main arduino setup() section ...
    return false;
}                               

#if HelperANDROIDMODE >= 2  
void _HelperTask::kill() {
    if (itemType == SCoopDynamicTask)
        state |= (SCoopKILLING);
    if (myHelper.Task == this) yieldSwitch();      // quick return to main scheduler for treating the situation
}
#endif
  
  
/******** YIELD SECTION ****************/

void _HelperTask::yield() {                           // check if the quantum time alowed is elapsed, the switch to scheduler
    yieldInline(quantumMicros);
}

void _HelperTask::yield(micros_t quantum) {
    yieldInline(quantum);
}


void _HelperTask::yieldInline(micros_t quantum) {
    if (quantum) {
        register micros_t spent = SCoopMicros() - prevMicros;
        if (spent >= quantum) yieldSpent(spent);   // switch makes sense	   
    } else if (!SCINM.Atomic) {
        if (SCINM.Task != NULL) { // 結算現在的
            SCINM.Task->spentClocks += (timer_GetClocks64() - SCINM.Task->tempT);
        } // 沒有 else，因為不會發生
        
        yieldSwitch(); // we ve been called from a task context lets yield from there
        
        if (SCINM.Task != NULL) // 重置新 Task time
            SCINM.Task->tempT = timer_GetClocks64();
        else
            SCINM.mainT = timer_GetClocks64();
    }
}
  
  
void _HelperTask::yieldSpent(micros_t spent) {       // immediate jump back to scheduler
    if (SCINM.Atomic) return;                      // do nothing if we are in a atomic section
    
    if (SCINM.Task != NULL) { // 結算現在的
        SCINM.Task->spentClocks += (timer_GetClocks64() - SCINM.Task->tempT);
    } // 沒有 else，因為不會發生

    yieldSwitch(); // we ve been called from a task context lets yield from there
    
    if (SCINM.Task != NULL) // 重置新 Task time
        SCINM.Task->tempT = timer_GetClocks64();
    else
        SCINM.mainT = timer_GetClocks64();
}

   
void _HelperTask::yieldSwitch() { 
	register _HelperTask* temp;
	if ((HelperYIELDCYCLE == 1) &&                    // optimize speed by directly switching next adjacent task
	   ((temp=pNext) != NULL) &&                     // only if possible, otherwise back to main loop
       ((temp->state & (SCoopRUNNABLE | SCoopPAUSED | SCoopKILLING)) == SCoopRUNNABLE))	{
           SCINM.Current = temp;                  
           SCINM.Task = temp;          // lets go next
           SCoopSwitch(&(temp->pEnv), &pEnv);    // save our context and use next one
    } else {                                           // systematically return to main loop or scheduler if using cycle()
        coroutineState = IN_MAIN_LOOP;       // 86Duino
        SCINM.Task = NULL;
        SCoopSwitch(&SCINM.mainEnv, &pEnv);        // save context and return to main scheduler
    }												 // will return here by launch() from scheduler yield() or cycle()
    prevMicros = SCoopMicros();
}                                               // come back into the task HERE / NOW


ptrInt _HelperTask::stackLeft() {
    if (pStackAddr)                          // sanity check if stack has been initialized
        return SCoopMemSearch(pStackAddr, pStack, 0x55);
    else
        return 0;
}


/********* _Helper METHODS *******/
  
_Helper::_Helper() {       // constructor
    startQuantum      = SCoopDefaultQuantum;     // every task will get the default value unless changed in their setup or with sart()
    quantumMicros     = SCoopDefaultQuantum;     // the main loop also
#if HelperYIELDCYCLE == 0
    quantumMicrosReal = 0;                       // we do nt know yet the number of task registered
#endif
	targetCycleMicros = SCoopDefaultQuantum;     //  we do nt know yet the total cycle time
    
    // _global_thisP = new _HelperTask*[256]; // 86Duino
    Current = NULL; 
	Task    = NULL; 
	Atomic  = 1;                               // ensure yield is not activated
}

void _Helper::start(micros_t cycleTime, micros_t mainLoop) {  // define the total length of the cycle and the main loop quantum
    if (cycleTime < 0) cycleTime = 0;
    if (mainLoop < 0) mainLoop = 0;
    if (cycleTime < mainLoop) mainLoop = cycleTime;
    startQuantum = (cycleTime - mainLoop) / (_HelperNumberTask); 
    quantumMicros = mainLoop;                                // specific time for main loop
    coroutineState = IN_MAIN_LOOP; // 86Duino
    start();
}


void _Helper::start(micros_t cycleTime) {          // define the total length of the cycle for all tasks+1
    if (cycleTime < 0) cycleTime = 0;
    startQuantum = cycleTime / (_HelperNumberTask+1); // consider the main loop as a task
    quantumMicros = startQuantum;                 // time for the main loop will be the same then
    start();
}
  

void _Helper::start() {                          // start all objects in the list
#if HelperYIELDCYCLE == 0
    quantumMicrosReal = quantumMicros / _HelperNumberTask;  // divide time in slice, as we come back here at each task switch...
#endif
    // SCINM.Atomic = 0;
    cpuFreq = vx86_CpuCLK();
    mainT = scoopT = timer_GetClocks64(); // start to run loop() after this line
    mainSpentClocks = allSpentClocks = prevMainRunTimes = mainRunTimes = 0;
    prevBusyCPU = 0.0;
}

  // this is the main code for the Scheduler, relying on yield() method as a state machine
  
void _Helper::yield0() {                        // can be called from where ever in order to Force the switch to next task
    if (Task) Task->yield(0); else _Helper::yield();
}

void _Helper::yield() {                   // can be called from where ever in order to Force the switch to next task
    coroutineState = IN_HELPER;       // 86Duino
    
    st = timer_GetClocks64() - scoopT;
    
    if (st > (cpuFreq * 1000000LL)) {
        _HelperTask* temp = NULL;
        temp = _HelperFirstTaskItem;
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
    
    if (Task) {
        Task->yield();                   // we ve been called from a task context lets yield from there
    } else {
        if (Atomic) return;                      // self explaining
        
        register _HelperTask* temp;
        register micros_t time;
        if (Current == NULL) {                   // a cycle is completed
            coroutineState = IN_MAIN_LOOP;
            temp = _HelperFirstTaskItem;
            if (temp == NULL) return;             // no tasks in the list !

            // check overall target cycle time before launching first task
    
            if (quantumMicros) {                   // check if we are supposed to spend some time in the main loop or not
                time = SCoopMicros() - temp->prevMicros; // mesure whole cycle length based on first task information
                if (time < targetCycleMicros) return;   // back in main loop() until we reach the expected target cycle time
            }
            Current = temp;
		} else { // lets check intertask timing before launching the next 
#if HelperYIELDCYCLE == 0
            if (quantumMicrosReal) {              // check if we should spend quite some time in the main loop between 2 tasks
                time = SCoopMicros() 
                       - Current->prevMicros // give the time since last task executed
                       - Current->quantumMicros; // mesure whole cycle length based on first task information
                if (time < quantumMicrosReal) return;   // back in main loop() until we reach the expected target cycle time           
            }
#endif
		}
        
        coroutineState = IN_HELPER; // 86Duino
        mainSpentClocks += (timer_GetClocks64() - mainT);
        temp->tempT = timer_GetClocks64(); // 先重置在前
        temp->launch();
        Current = NULL;
        mainT = timer_GetClocks64();
        /*
        do { temp = Current; 
            temp->launch();                     // now launch tasks from the list, and may be all in a single launch()
            Current = temp->pNext;   
#if HelperANDROIDMODE >= 2                    // check if we autorize the killme
            if ((temp->state & SCoopKILLING) && (temp->itemType == SCoopDynamicTask))
                delete temp;	// killing done
#endif
#if HelperYIELDCYCLE == 0	                   // back to main task if we are not in yield cycle mode
            return; 
#endif	                                       // otherwise we just go back into the main loop      
	    } while (Current);
        */
	}
}

double _Helper::getFreeCPU() {
    
    if (prevBusyCPU > 0.0) {
        if (prevBusyCPU <= 100.0)
            return 100.0 - prevBusyCPU;
        else
            return 0.0;
    }
    
    return -1;
}

double _Helper::getBusyCPU(int TaskN) {
    _HelperTask* temp = NULL;
    int i = 1; // TaskN 從 1 開始, -1 是 main loop
    
    if (TaskN != -2) {
        
        if (TaskN == -1) return ((double)averageClocks / (cpuFreq * 1000000.0)) * 100.0;
        
        temp = _HelperFirstTaskItem;
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

unsigned long _Helper::getTaskRunTimes(int TaskN) {
    _HelperTask* temp = NULL;
    int i = 1; // TaskN 從 1 開始, -1 是 main loop
    unsigned long total = 0;
    
    if (TaskN != -2) {
        
        if (TaskN == -1) return prevMainRunTimes;
        
        temp = _HelperFirstTaskItem;
        while (temp != NULL) {
            if (i == TaskN) return temp->prevRunTimes;
            temp = temp->pNext;
            i++;
        }
        return -2; // Not found
    }

    return round((1.0 / prevBusyCPU) * 100.0);
}


#if HelperANDROIDMODE >= 1
_HelperTask* _Helper::startLoop(uint32_t stackSize) {
	uint8_t *stack = (uint8_t*)malloc(stackSize);
	if (!stack) return NULL;

	_HelperTask *task = new _HelperTask((SCoopStack_t*)stack, stackSize);
	if (!task) {
		free(stack);
		return NULL;
    }

    // object now exist and is registered in the list!
	task->itemType = SCoopDynamicTask;
	return task;
}
#endif

/*************** OVERLOAD STANDARD YIELD *****************/


void helperYield(void)  { SCINM.yield(); };              // overload standard arduino yield
void helperYield0(void) { SCINM.yield0(); };             // overload standard arduino yield


  
