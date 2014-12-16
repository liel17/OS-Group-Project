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
	        maxCPUtime = *(p+4);
	        curr_time = *(p+5);

	        inCoreBit = latchBit = blockBit
                    = killBit = IOpending = false;

            CPUtimeUsed = 0;
	    }

	    Process() {}

        //Setter and getter functions
	    void setJobNum(long val) {jobNum = val;}
	    long getJobNum() {return jobNum;}

	    void setPriority(long val) {priority = val;}
	    long getPriority() {return priority;}

	    void setJobSize(long val) {jobSize = val;}
	    long getJobSize() {return jobSize;}

	    void setMaxCPUTime(long val) {maxCPUtime = val;}
	    long getMaxCPUTime() {return maxCPUtime;}

	    void setCurrTime(long val) {curr_time = val;}
	    long getCurrTime() {return curr_time;}

	    void setAddress(long val) {address = val;}
	    long getAddress() {return address;}

	    void setCPUtimeUsed(long val) {CPUtimeUsed = val;}
	    long getCPUtimeUsed() {return CPUtimeUsed;}

	    void setInCoreBit(bool b) {inCoreBit = b;}
	    bool isInCore() {return inCoreBit;}

	    void setLatchBit(bool b) {latchBit = b;}
	    bool isLatched() {return latchBit;}

	    void setBlockBit(bool b) {blockBit = b;}
	    bool isBlocked() {return blockBit;}

	    void setKillBit(bool b) {killBit = b;}
	    bool isTerminated() {return killBit;}

	    void setIOpending(bool b) {IOpending = b;}
	    bool haspendingIO() {return IOpending;}

	private:
	    long
	        jobNum,     // Job number
	        priority,   // Priority of job
	        jobSize,    // Size of job
	        maxCPUtime, // Max CPU time of job
	        curr_time,  // Current run time
	        address,    // Current memory address of job
            CPUtimeUsed;

        bool
            inCoreBit,  // Flag if job is swapped in
            latchBit,   // Flag if job is swapped out
            blockBit,   // Flag if job has been blocked
            killBit,    // Flag if job has been terminated
            IOpending;  // Flag if job has pending IO

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
long timestamp;

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
Process scheduler(long&, long*);
void dispatcher(long&, long*);
bool findSpace(long);
long getSpace(long);
void combine();
void timer(long);


 /*--------+
 | Startup |
 +--------*/
void startup() {
    nextAvailAdd = start = timestamp = 0;
    drumInUse = false;
    fsExists = true;
    CPUidle = true;
    //ontrace();
}

 /*-------------------+
 | Interrupt handlers |
 +-------------------*/
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
    timer( *(p+5) );
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
            CPUTime = runningProc.getMaxCPUTime();
            jobNum = runningProc.getJobNum();
            break;
        }
    }

    dispatcher(a, p);
}

void Drmint(long &a, long *p) {
    timer( *(p+5) );
    runningProc = scheduler(a, p);
    drumqueue.pop_back();

    drumInUse = false;
    dispatcher(a, p);
}

void Tro(long &a, long *p) {
    timer( *(p+5) );
    dispatcher(a, p);
}

void Svc(long &a, long *p) {
    timer( *(p+5) );

    runningProc = scheduler(a, p);
    long jobNum = runningProc.getJobNum();
    long jobSize = runningProc.getJobSize();
    long address = runningProc.getAddress();

    switch (a) {
    case 5:
        timer( *(p+5) );

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
        runningProc.setIOpending(true);

        // Update job table
        for (int i = 0; i < jobtable.size(); i++) {
            if ( jobtable[i].getJobNum() == *(p+1) )
                jobtable[i] = runningProc;
        }


        // Send the job to do I/O
        siodisk(jobNum);
        diskIsBusy = true;
        CPUidle = false;

        dispatcher(a, p);
        break;

    case 7:
        // Search the job table for the process being blocked
        for (int i = 0; i < jobtable.size(); i++) {
            if ( jobtable[i].getJobNum() == *(p+1) )
                jobtable[i].setBlockBit(true);
        }

        CPUidle = true;
        a = 1;
        break;
    }
}

 /*-------------------------------------------------------------------+
 |                         CPU Scheduling                             |
 |                                                                    |
 | scheduler(): Uses FCFS to find the next job to schedule            |
 +-------------------------------------------------------------------*/

Process scheduler(long &action, long *params) {
    long baseAddress;
    long newJobSize;
    long newCPUTime;

    Process newproc;

    // Search the jobtable for the corresponding job number
    // and ensure the process has not been terminated
    for (int i = 0; i < jobtable.size(); i++) {
        if ( jobtable[i].getJobNum() == *(params+1) && !jobtable[i].isTerminated()) {
            baseAddress = start;
            newproc.setAddress(start);

            newJobSize = jobtable[i].getJobSize();
            newproc.setJobSize(newJobSize);

            newCPUTime = jobtable[i].getMaxCPUTime();
            newproc.setMaxCPUTime(newCPUTime);

            newproc.setJobNum( jobtable[i].getJobNum() );

            break;
        }
    }

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


/*-------------------------+
| Internal calls within OS |
+-------------------------*/

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

void timer(long time) {
    long interval = time - timestamp;
    timestamp = time;
    long newTime = interval + runningProc.getCPUtimeUsed();
    runningProc.setCPUtimeUsed(newTime);
}

void dispatcher(long &action, long *params) {
    action = 2;
    *(params+2) = runningProc.getAddress();
    *(params+3) = runningProc.getJobSize();
    *(params+4) = quantum;
}
