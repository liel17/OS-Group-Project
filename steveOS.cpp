#include <list>
#include <queue>

 /*------------------------------------------------------------------------+
 |                         Data Structures                                 |
 |                                                                         |
 | Process: Represemts a job being passed into OS                          |
 | FreeSpaceEntry: Represemts an entry in the free space table             |
 | jobtable: An STL vector used to represent the job table. Holds Process  |
 |           objects (PCB blocks)                                          |
 | drumqueue: An STL queue used to represent the drum queue. Holds Process |
 |            objects.                                                     |
 | freespacetable: An STL vector used to represent a free space table.     |
 |                 Holds FreeSpaceEntry objects                            |
 +------------------------------------------------------------------------*/
class Process {
	public:
	    //Constructor
	    Process(long *p) {
            jobNum = *(p+1);
	        priority = *(p+2);
	        jobSize = *(p+3);
	        CPU_time = *(p+4);
	        curr_time = *(p+5);
	    }

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

	private:
	    long
	        jobNum,     // Job number
	        priority,   // Priority of job
	        jobSize,    // Size of job
	        CPU_time,   // Max CPU time of job
	        curr_time;  // Current run time
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
std::queue<Process> drumqueue;
std::vector<FreeSpaceEntry> freespacetable;

 /*-----------------+
 | Global variables |
 +-----------------*/
long nextAvailAdd;  // Indicate next available address
bool drumInUse;     // Flag if drum is in use
bool fsExists;      // Flag if free space is available

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
void scheduler();
bool findSpace(long);
long getSpace(long);

 /*--------+
 | Startup |
 +--------*/
void startup() {
    nextAvailAdd = 0;
    drumInUse = false;
    fsExists = true;
    ontrace();
}

 /*-----------------------------------------------------------------------+
 |                      Interrupt handlers                                |
 |                                                                        |
 | Crint: A new Process object is created and initialized with parameters |
 |        passed in from *p and new FreeSpaceEntry object is created and  |
 |        stored onto the FreeSpaceTable.
 +-----------------------------------------------------------------------*/
void Crint(long &a, long *p) {
    Process process(p);
    jobtable.push_back(process);
    drumqueue.push(process);

    FreeSpaceEntry entry;
    freespacetable.push_back(entry);

    long jobNum = process.getJobNum();
    long jobSize = process.getJobSize();

    if(!drumInUse) {
        // check Free Space Table...
        fsExists = findSpace(jobSize);

        // If there is memory:
        if (fsExists) {
            drumInUse = true;

            // Update free space table
            int index = freespacetable.size() - 1;
            freespacetable[index].insert(jobSize);

            siodrum(jobNum, jobSize, nextAvailAdd, 0);

            // Check free space table and get next available (nextAvailAdd)
            nextAvailAdd = getSpace(jobSize);
        }
        else {
          // If there is no memory...
            // Search jobtable for a process to kick out
        };
    };




    //call scheduler();
}

void Dskint(long &a, long *p) {
}

void Drmint(long &a, long *p) {
}

void Tro(long &a, long *p) {
}

void Svc(long &a, long *p) {
    switch (a) {
    case 5:
        break;
    case 6:
        break;
    default:
        break;
    }
}

// Helper functions
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

long getSpace(long jobSize) {
    long max = 0;

 	for(int i = 0; i < freespacetable.size(); i++) {
        if(freespacetable[i].getFS() >= max)
            max = freespacetable[i].getFS();
 	}

    return max;
}
