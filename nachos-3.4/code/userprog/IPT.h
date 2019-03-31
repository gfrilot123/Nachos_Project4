#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "thread.h"

class IPT
{
    public:
        IPT();
        ~IPT();

        Thread* thread;
        int vPageNumber;

    private:
};

#endif