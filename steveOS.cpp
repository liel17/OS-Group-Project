#include <iostream>

using namespace std;


class PCB {
	public:
	    PCB() {
            latch_bit = false;
	    }

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

	private:
	    long
	        jobNum,
	        priority,
	        jobSize,
	        CPU_time,
	        curr_time;
        bool latch_bit;
};

//Calls to SOS
void ontrace();
void offtrace();
void siodrum(long, long, long, long);
void siodisk(long);

//Global variables
const int SIZE = 50;
int index;
PCB jobtable[SIZE];
int start;

//Startup
void startup(){
    index = 0;
    start = 0;
}

//Interrupt handlers
void Crint(long &a, long *p) {
    if (index == SIZE)
        index = 0;

    jobtable[index].setJobNum(p[1]);
    jobtable[index].setPriority(p[2]);
    jobtable[index].setJobSize(p[3]);
    jobtable[index].setCPUTime(p[4]);
    jobtable[index].setCurrTime(p[5]);
    jobtable[index].setLatchBit(false);

    index++;
/*Testing siodrum
    ontrace();
    long jobNum = jobtable[index].getJobNum();
    long jobSize = jobtable[index].getJobSize();
    start += jobSize;
    siodrum(jobNum, jobSize, start, 0);
    a = 2;
    offtrace();
    */
}

void Dskint(long &a, long *p){
    jobtable[index].setCurrTime(p[5]);
}

void Drmint(long &a, long *p){
    jobtable[index].setCurrTime(p[5]);
}

void Tro(long &a, long *p){
    jobtable[index].setCurrTime(p[5]);
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
}
