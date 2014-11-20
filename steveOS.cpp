#include<iostream>

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

//Global variables
const int SIZE = 50;
int index;
PCB jobtable[SIZE];

void startup(){
    index = 0;
}

void Crint(long a, long p[]) {
    if (index == SIZE)
        index = 0;

    jobtable[index].setJobNum(p[1]);
    jobtable[index].setPriority(p[2]);
    jobtable[index].setJobSize(p[3]);
    jobtable[index].setCPUTime(p[4]);
    jobtable[index].setCurrTime(p[5]);

    index++;
}

void Dskint(long a, long p[]){


}

void Drmint(long a, long p[]){


}

void Tro(long a, long p[]){


}

void Svc(long a, long p[]){


}
