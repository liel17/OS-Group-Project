
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

    long getSize() {return SIZE;}
    private:
        const long SIZE = 100;  // Maximum memory size allotted
        long freeSpace;         // Size entry
        long startAdd;          // Address entry

};

std::vector<Process> jobtable;
std::queue<Process> drumqueue;
std::queue<Process> cpuqueue;
std::queue<Process> ioqueue;
std::vector<FreeSpaceEntry> freespacetable;

 /*-----------------+
 | Global variables |
 +-----------------*/
long nextAvailAdd;  // Indicate next available address
bool drumInUse;     // Flag if drum is in use
bool drumToMem = true;     // Indicates a drum to memory transfer
bool fsExists;      // Flag if free space is available

 /*-----------------+
 |   Manages Time   |
 +-----------------*/
bool idle = true;   // True when idling.
long totalTime = 0;
long stopwatch = 0;

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
void Timer(long time);

 /*--------+
 | Startup |
 +--------*/
void startup() {
    nextAvailAdd = 0;
    drumToMem = true;
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

   Timer(*p+5);

   drumInUse = false;

   if(drumToMem)
   {
        drumqueue.front().IsInMemory(true);

        /////////////////////////Add IO IF STATEMENT
        cpuqueue.push(drumqueue.front());
        drumqueue.pop();
   }

   if(drumqueue.empty())
   {
       drumInUse = false;

   }
else
{
    getSpace(drumqueue.front().getJobSize());
    if(!drumInUse)

    {
        freespacetable[freespacetable.size()-1].insert(drumqueue.front().getJobSize());

        if(findSpace(drumqueue.front().getJobSize()))
        {

            nextAvailAdd = getSpace(drumqueue.front().getJobSize());
            siodrum(drumqueue.front().getJobNum(), drumqueue.front().getJobSize(),nextAvailAdd, 0);
            drumInUse = true;

    for (int i = 0; i < jobtable.size(); i++)
          {
            if(jobtable.at(i).getJobNum() == drumqueue.front().getJobNum())
            {
              //  jobtable.at(i).IsInMemory(true);
               // jobtable.at(i).setSA(nextAvailAdd); MAKE SET ADRESS FUNCTION
            }
          }


        }
        else
        {

            for (int i = 0; i < jobtable.size(); i++)
          {
            if(jobtable.at(i).getJobNum() == drumqueue.front().getJobNum())
            {
                //jobtable.at(i).IsInMemory(false);
                //jobtable.at(i).setSA(nextAvailAdd);SET SWAPPER
            }
          }
            drumqueue.pop();
        }


    }
    idle = true;
}
    ///////////// MAKE INTO FUNCTION///////
    if(cpuqueue.empty())
    {
         a =1;
         idle = true;
    }

    else
    {

    }
//////////////////////////////////////////

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

void Timer(long currentTime){
    if(!idle)
    {

        totalTime += currentTime-stopwatch;
        totalTime += currentTime-stopwatch;
        cpuqueue.front().addTime(currentTime-stopwatch);
    }



}
