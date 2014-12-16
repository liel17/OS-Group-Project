#include <vector>
#include <iostream>

 /*-------------------------------------------------------------------------+
 |                         Data Structures                                  |
 |                                                                          |
 | Process: Represemts a job being passed into OS                           |
 | FreeSpaceEntry: Represemts an entry in the free space table              |
 | jobtable: An STL vector used to represent the job table. Holds Process   |
 |           objects (PCB blocks)                                           |
 | drumqueue: An STL vector used to represent the drum queue. Holds Process |
 |            objects.                                                      |
 | freespacetable: An STL vector used to represent a free space table.      |
 |                 Holds FreeSpaceEntry objects                             |
 +-------------------------------------------------------------------------*/
class Process {
	public:
	    //Constructor
	    Process(long *p) {
            jobNum = *(p+1);
	        priority = *(p+2);
	        jobSize = *(p+3);
	        CPU_time = *(p+4);
	        curr_time = *(p+5);
	        inCoreBit = latchBit = blockBit = killBit = false;
	    }

	    Process() {}

        //Setter and getter functions
	    void setJobNum(long val) {jobNum = val;}
	    long getJobNum() {return jobNum;}

	    void setPriority(long val) {priority = val;}
	    long getPriority() {return priority;}

	    void setJobSize(long val) {jobSize = val;}
	    long getJobSize() {return jobSize;}

	    void setCPUTime(long val) {CPU_time = val;}
	    long getCPUTime() {return CPU_time;}

	    void setCurrTime(long val) {curr_time = val;}
	    long getCurrTime() {return curr_time;}

	    void setAddress(long val) {address = val;}
	    long getAddress() {return address;}

	    void setInCoreBit(bool b) {inCoreBit = b;}
	    bool isInCore() {return inCoreBit;}

	    void setLatchBit(bool b) {latchBit = b;}
	    bool isLatched() {return latchBit;}

	    void setBlockBit(bool b) {blockBit = b;}
	    bool isBlocked() {return blockBit;}

	    void setKillBit(bool b) {killBit = b;}
	    bool isTerminated() {return killBit;}

	private:
	    long
	        jobNum,     // Job number
	        priority,   // Priority of job
	        jobSize,    // Size of job
	        CPU_time,   // Max CPU time of job
	        curr_time,  // Current run time
	        address;    // Current memory address of job

        bool
            inCoreBit,  // Flag if job is swapped in
            latchBit,   // Flag if job is swapped out
            blockBit,   // Flag if job has been blocked
            killBit;    // Flag if job has been terminated

};

class FreeSpaceEntry {
    public:
    //Constructors
    FreeSpaceEntry(long jobSize, long start) {
        freeSpace = jobSize;
        startAdd = start;
    }

    FreeSpaceEntry() : freeSpace(SIZE), startAdd(0) {}

    //Setter and getter functions
    void setFS(long val) { freeSpace = val; }
    long getFS() {return freeSpace;}

    void setSA(long val) { startAdd = val; }
    long getSA() { return startAdd; }

    // Other functions
    void insert(long jobSize) {
		freeSpace -= jobSize;
		startAdd += jobSize;
	}

	void remove(long jobSize, long start) {
	    freeSpace = jobSize;
		startAdd = start;
	}

    private:
        const long SIZE = 100;  // Maximum memory size allotted
        long freeSpace;         // Size entry
        long startAdd;          // Address entry
};

std::vector<Process> jobtable;
std::vector<Process> drumqueue;
std::vector<Process> IOqueue;
std::vector<FreeSpaceEntry> freespacetable;

 /*-----------------+
 | Global variables |
 +-----------------*/
long nextAvailAdd;  // Indicate next available address
long start;         // Used to set base address
long currTime;      // Current run time
long stopclock;
bool drumInUse;     // Flag if drum is in use
bool diskIsBusy;    // Flag if disk is busy
bool fsExists;      // Flag if free space is available
bool doingIO;       // Flag if IO is being done
bool CPUidle;       // Flag is CPU is idle

long quantum = 5;

Process runningProc;

 /*-------------+
 | Calls to SOS |
 +-------------*/
void ontrace();
void offtrace();
void siodrum(long, long, long, long);   //TransferDir must be converted from int to long
void siodisk(long);

/*-------------------------+
| Internal calls within OS |
+-------------------------*/
void swapper();
Process scheduler(long&, long*);
bool findSpace(long);
long getSpace(long);
void timer(long);
void bookkeep(long);
void combine();

 /*--------+
 | Startup |
 +--------*/
void startup() {
    nextAvailAdd = start = 0;
    drumInUse = false;
    fsExists = true;
    CPUidle = true;
    ontrace();
}

 /*-----------------------------------------------------------------------+
 |                      Interrupt handlers                                |
 |                                                                        |
 | Crint: A new Process object is created and initialized with parameters |
 |        passed in from *p and new FreeSpaceEntry object is created and  |
 |        stored onto the FreeSpaceTable. This will also be used to swap  |
 |        jobs into memory.
 +-----------------------------------------------------------------------*/
void Crint(long &a, long *p) {
    Process process(p);

    jobtable.push_back(process);

    FreeSpaceEntry entry;
    freespacetable.push_back(entry);

    long jobNum = process.getJobNum();
    long jobSize = process.getJobSize();

    // If the drum isn't in use...
    if(!drumInUse) {
        // check Free Space Table...
        fsExists = findSpace(jobSize);

        // If there is memory:
        if (fsExists) {
            drumInUse = true;
            drumqueue.push_back(process);
            combine();

            // Update free space table
            int index = freespacetable.size() - 1;
            start = freespacetable[index].getSA();

            jobtable[index].setAddress(start);

            freespacetable[index].insert(jobSize);

            // Begin swap in

            for(int i = 0; i < jobtable.size(); i++) {
                if (jobtable[i].getJobNum() == *(p+1))
                    jobtable[i].setInCoreBit(true);
                break;
            }
            siodrum(jobNum, jobSize, start, 0);
            jobtable[index].setInCoreBit(true);
        }
    }
    a = 1;
}

void Dskint(long &a, long *p) {
    currTime = *(p+5);
    diskIsBusy = false;
    long baseAddress;
    long jobSize;
    long CPUTime;
    long jobNum;

    for (int i = 0; i < jobtable.size(); i++) {
        if(jobtable[i].getJobNum() == *(p+1)) {
            jobtable[i].setLatchBit(false);
            baseAddress = runningProc.getAddress();
            jobSize = runningProc.getJobSize();
            CPUTime = runningProc.getCPUTime();
            jobNum = runningProc.getJobNum();
            break;
        }
    }

    a = 2;
    *(p+2) = baseAddress;
    *(p+3) = jobSize;
    *(p+4) = quantum;
}

void Drmint(long &a, long *p) {
    currTime = *(p+5);
    runningProc = scheduler(a, p);

    drumInUse = false;

    a = 2;
    *(p+2) = runningProc.getAddress();
    *(p+3) = runningProc.getJobSize();
    *(p+4) = quantum;
}

void Tro(long &a, long *p) {
    currTime = *(p+5);
    a = 2;
}

void Svc(long &a, long *p) {
    currTime = *(p+5);

    runningProc = scheduler(a, p);
    long jobNum = runningProc.getJobNum();
    long jobSize = runningProc.getJobSize();
    long address = runningProc.getAddress();

    switch (a) {
    case 5:
        currTime = *(p+5);

        // Search the job table for the process being terminated
        for (int i = 0; i < jobtable.size(); i++) {
            if (jobtable[i].getJobNum() == *(p+1))
                jobtable[i].setInCoreBit(false);
                jobtable[i].setKillBit(true);
                IOqueue.pop_back();
            break;
        }

        // Update free space table
        freespacetable[0].remove( runningProc.getJobNum(), runningProc.getAddress() );
        freespacetable.pop_back();

        CPUidle = true;
        a = 1;
        break;

    case 6:
        //Push the job onto the I/O queue
        IOqueue.push_back(runningProc);

        // Send the job to do I/O
        siodisk(jobNum);
        diskIsBusy = true;
        CPUidle = false;

        a = 2;
        *(p+2) = address;
        *(p+3) = jobSize;
        *(p+4) = quantum;
        break;

    case 7:
        // Search the job table for the process being blocked
        for (int i = 0; i < jobtable.size(); i++) {
            if ( jobtable[i].getJobNum() == *(p+1) )
                jobtable[i].setBlockBit(true);
        }

        CPUidle = true;
        a = 1;
        *(p+2) = runningProc.getAddress();
        *(p+3) = runningProc.getJobSize();
        *(p+4) = quantum;
        break;
    }
}

 /*-------------------------------------------------------------------+
 |                         CPU Scheduling                             |
 |                                                                    |
 | scheduler(): Uses FCFS to find the next job to schedule              |
 |
 |
 |
 +-------------------------------------------------------------------*/

Process scheduler(long &action, long *params) {
    long baseAddress;
    long newJobSize;
    long newCPUTime;

    Process newproc;

    for (int i = 0; i < jobtable.size(); i++) {
        if ( jobtable[i].getJobNum() == *(params+1) ) {
            baseAddress = start;
            newproc.setAddress(start);

            newJobSize = jobtable[i].getJobSize();
            newproc.setJobSize(newJobSize);

            newCPUTime = jobtable[i].getCPUTime();
            newproc.setCPUTime(newCPUTime);

            newproc.setJobNum( jobtable[i].getJobNum() );

            break;
        }
    }

    long index = jobtable.size() - 1;
    //start += jobtable[index].getAddress();

    return newproc;
}

/*-------------------------------------------------+
|                 Memory Management                |
| Memory is managed using the first fit algorithm. |
|                                                  |
| combine(): Iterates through the free space table |
|            and combines contiguous free space    |
+-------------------------------------------------*/

void combine() {
    for (int i = 0; i < freespacetable.size()-1; i++) {
        long space = freespacetable[i].getFS() + freespacetable[i].getSA();
        if (space == freespacetable[i+1].getFS()) {
            freespacetable[i].setSA( freespacetable[i+1].getSA() );
            freespacetable[i].setFS( freespacetable[i+1].getFS() );
            freespacetable.pop_back();
        }
    }
}


/*------------------------------------------+
|        Internal calls within OS           |
|                                           |
|                                           |
|                                           |
+------------------------------------------*/

bool findSpace(long jobSize) {
 	long max = 0;

 	for(int i = 0; i < freespacetable.size(); i++) {
        if(freespacetable[i].getFS() >= max)
            max = freespacetable[i].getFS();
 	}

    if (max >= jobSize)
        return true;

    return false;
}

// BUG: RETURNS DIFFERENCE
long getSpace(long jobSize) {
    long max = 0;

 	for(int i = 0; i < freespacetable.size(); i++) {
        if(freespacetable[i].getFS() >= max)
            max = 100 - freespacetable[i].getFS();
 	}

    return max;
}

void timer (long currTime) {
    quantum = currTime - quantum;
}

void bookkeep(long time) {
    long interval = time - stopclock;
}
