//Begin Code changes by Robert Knott
//IPT creates an IPT object that is used to hold a copy of the thread that is using it
//as well as the virutal page number used to hold it in memory.
//
//The IPT object has no functions.  It is strictly used to hold the values that are used
//to point back to a process' thread and reference the virtual page number.
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "thread.h"

class IPT
{
    public:
	//Constructor
        IPT();
	//Desctructor
        ~IPT();

        Thread* thread;		//A pointer to the associated process' thread
        int vPageNumber;	//The process' virtual page number
};

#endif
//End Code changes by Robert Knott
