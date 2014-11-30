class PCB {
	public:
	    PCB() {}

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

	    void setLatchBit(bool b) {latch_bit = b;}
	    bool isDoingIO() {return latch_bit;}

	    void setKillBit(bool b) {kill_bit = b;}
	    bool isComplete() {return kill_bit;}

	    void setBlockBit(bool b) {block_bit = b;}
	    bool isBlocked() {return block_bit;}

        void setInCoreBit(bool b) {inCore_bit = b;}
	    bool isInCore() {return inCore_bit;}

	private:
	    long
	        jobNum,
	        priority,
	        jobSize,
	        CPU_time,
	        curr_time;
        bool
            latch_bit,
            kill_bit,
            block_bit,
            inCore_bit;

};

//Global variables
const int SIZE = 50;
int index;
PCB jobtable[SIZE];
long start;

//Calls to SOS
void ontrace();
void offtrace();
void siodrum(long, long, long, long);   //TransferDir must be converted from int to long
void siodisk(long);

// Internal calls within OS
void swapper(PCB);
void scheduler(PCB);

//Startup
void startup(){
    index = 0;
    start = 0;
}

//Interrupt handlers
void Crint(long &a, long *p) {
    if (index == SIZE)
        index = 0;

    jobtable[index].setJobNum(*(p+1));
    jobtable[index].setPriority(*(p+2));
    jobtable[index].setJobSize(*(p+3));
    jobtable[index].setCPUTime(*(p+4));
    jobtable[index].setCurrTime(*(p+5));
    jobtable[index].setLatchBit(false);

    swapper(jobtable[index]);

    a = 1;
    start += jobtable[index].getJobSize();
    index++;
}

void Dskint(long &a, long *p){
    jobtable[index].setCurrTime(*(p+5));
}

void Drmint(long &a, long *p){
    a = 1;
    jobtable[index].setCurrTime(*(p+5));
}

void Tro(long &a, long *p){
    jobtable[index].setCurrTime(*(p+5));
}

void Svc(long &a, long *p){
    switch (a) {
    case 5:
        break;
    case 6:
        break;
    default:
        break;
    }

    jobtable[index].setCurrTime(*(p+5));
}

//Internal helper functions
void swapper(PCB process) {
    long jobNum = process.getJobNum();
    long jobSize = process.getJobSize();
    siodrum(jobNum, jobSize, start, 0);
}

/*
void scheduler(PCB process) {

}
*/
