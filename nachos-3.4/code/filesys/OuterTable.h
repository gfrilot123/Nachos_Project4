#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "addrspace.h"

class OuterTable
{
    public:
        OuterTable();
        OuterTable();

        TranslationEntry* pageTable;
};

#endif
