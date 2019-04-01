//Begin Code changes by Robert Knott
//As mentioned in IPT.h, the IPT object has no functions, and is only used to hold a pointer
//back to a process' thread and the virutal page number the process uses.  As such, the
//constructor and destructor are empty and are only nominally called to create and delete
//a new instance of the object.
#include "IPT.h"

//Constructor
IPT::IPT()
{}

//Destructor
IPT::~IPT()
{}

//End Code changes by Robert Knott
