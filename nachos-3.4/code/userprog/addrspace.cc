// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "syscall.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
		if(executable == NULL)
		{
			printf("Executable is null. Exiting\n");
			SExit(-1);
		}
    NoffHeader noffH;
    unsigned int i, size;

		size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
			+ UserStackSize;
	  // Code added by Joseph Aucoin
		// Variables to store executable length and char buffer
		//int execLength = executable->Length();
		char* buffer = new char[size];

		// Creates process swap file based on currentID
		char processId[100];
		sprintf(processId, "%d.swap", currentId);
		//printf("%s\n",processId);

		// Creation and opening of swap files
		fileSystem->Create( processId , size);
		OpenFile* openFile = fileSystem->Open(processId);
		// Copies the user program into a swap files for reading
		openFile->WriteAt(buffer, size, 0);
		//printf("%d\n", executable->Length());

		// End code added by Joseph Aucoin

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
		if(noffH.noffMagic != NOFFMAGIC) {
			printf("There is a noff error with the desired user program. Exiting\n");
			SExit(-1);//MAKE THIS MEANINGFUL
		}
    //ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
// Size of executable only /instructions line for line
		printf("noffH.code.size: %d%s",noffH.code.size,"\n");
// Size of local variabes in main
		printf("noffH.initData.size: %d%s",noffH.initData.size,"\n");
//Size of global variables in main
		printf("noffH.uninitData.size: %d%s",noffH.uninitData.size,"\n");
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
		printf("UserStackSize: %d%s",UserStackSize,"\n");
		printf("Size of program before numPages: %d%s",size,"\n");
    numPages = divRoundUp(size, PageSize);
		printf("numPages: %d%s",numPages,"\n");

    size = numPages * PageSize;
		printf("Size of program after numPages: %d%s",size,"\n");


    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
					numPages, size);
// first, set up the translation
    pageTable = new TranslationEntry[numPages];

		//Begin Code Changes by Chau Cao

		//Print out the bitmap before attempting to bring the user program into memory
		if(memMap == NULL){
			printf("CRITICAL ERROR: GLOBAL BITMAP IS NULL. HOW DID THIS HAPPEN?!\n");
			SExit(-4);//legit don't know how this could happen but who knows
		}
		printf("Page availability before adding the process:\n ");
		//memMap->setMarks();		//sets marks

		memMap->Print();
		setMemory();

		//Code changes by Chau Cao
		//Calls OpenFile.ReadAt to pull the instructions and initData that needs to be
		//	read into memory.
		//The code instructions are read in and stored starting at memIndex
		//the phyiscal address is determined by the physical page alloted to the page table

		ExceptionType exception;
		int physicalAddress;
    if (noffH.code.size > 0) {
    	DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",noffH.code.virtualAddr, noffH.code.size);

			exception = addTranslate(noffH.code.virtualAddr, &physicalAddress, noffH.code.size, FALSE);

			if (exception != NoException) {
				machine->RaiseException(exception, noffH.code.virtualAddr);
			}
			else {
				//executable->ReadAt(&(machine->mainMemory[physicalAddress]), noffH.code.size, noffH.code.inFileAddr);
				//openFile
				//inFile Address = segment 40

			}
		}

    if (noffH.initData.size > 0) {
      DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", noffH.initData.virtualAddr, noffH.initData.size);

			exception = addTranslate(noffH.initData.virtualAddr, &physicalAddress, noffH.initData.size, FALSE);
			if (exception != NoException) {
				machine->RaiseException(exception, noffH.initData.virtualAddr);
			}
			executable->ReadAt(&(machine->mainMemory[physicalAddress]),noffH.initData.size, noffH.initData.inFileAddr);
    }



}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete [] pageTable;
}

//addTranslate is a modified copy of the translate function from machine in ordering
//to calculate physical address based on the virtual page table
//Basically copied directly from translate and editted to work properly
//Begin Code Changes by Chau Cao
ExceptionType AddrSpace::addTranslate(int virtAddr, int* physAddr, int size, bool writing)
{
    int i;
    unsigned int vpn, offset;
    TranslationEntry *entry;
    unsigned int pageFrame;

    DEBUG('a', "\tTranslate 0x%x, %s: ", virtAddr, writing ? "write" : "read");

// check for alignment errors
    if (((size == 4) && (virtAddr & 0x3)) || ((size == 2) && (virtAddr & 0x1))){
				DEBUG('a', "alignment problem at %d, size %d!\n", virtAddr, size);
					return AddressErrorException;
    }

// calculate the virtual page number, and offset within the page,
// from the virtual address
    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;


	entry = &pageTable[vpn];
	if (entry == NULL) {				// not found
    	    DEBUG('a', "*** no valid TLB entry found for this virtual page!\n");
    	    return PageFaultException;		// really, this is a TLB fault,
						// the page may be in memory,
						// but not in the TLB
    }

    if (entry->readOnly && writing) {	// trying to write to a read-only page
				DEBUG('a', "%d mapped read-only at %d in TLB!\n", virtAddr, i);
				return ReadOnlyException;
    }
    pageFrame = entry->physicalPage;

    // if the pageFrame is too big, there is something really wrong!
    // An invalid translation was loaded into the page table or TLB.
    if (pageFrame >= NumPhysPages) {
			DEBUG('a', "*** frame %d > %d!\n", pageFrame, NumPhysPages);
			return BusErrorException;
    }
    entry->use = TRUE;		// set the use, dirty bits
    if (writing)
		entry->dirty = TRUE;
    *physAddr = pageFrame * PageSize + offset;
    ASSERT((*physAddr >= 0) && ((*physAddr + size) <= MemorySize));
    DEBUG('a', "phys addr = 0x%x\n", *physAddr);
    return NoException;
}
//end code changes

//setMemory function for address space constructor
//Begin Code Changes by Chau Cao
void AddrSpace::setMemory() {
	//loops through the number of pages that need to be placed into main memory.
	//Virtual Page in page table is just the current index.
	//Physical Page: startIndex is the starting index of the found contiguous memory
	//use this as the offset and increment by x while assigning the following.
	//Mark the coinciding bit in memMap as used, and assign that location to Physical Page
	//Call bzero on that location in mainMemory to clear that frame of size 256 for the
	//	instructions being stored then break
	//The assignemnts mimic the one provided in the base exception
	printf("Index %d\n", memIndex);
	for (int x = 0; x < numPages; x++) {
		pageTable[x].virtualPage = x;
		pageTable[x].valid = false;
		pageTable[x].use = false;
		pageTable[x].dirty = false;
		pageTable[x].readOnly = false;
		if(pageTable[x].valid)
		{
		printf("Page availability after adding the process:\n ");
		memMap->Mark(x);
		memMap->Print();
		}
		InitRegisters();
	}

}
//end code changes by Chau Cao

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------
void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++) {
			machine->WriteRegister(i, 0);

		}
    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, (numPages * PageSize) - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
	currentThread->SaveUserState();
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

//Begin Code Changes by Chau Cao
//Helper functions to return private variables
int AddrSpace::getNumPages() {
	return numPages;
}
int AddrSpace::getMemIndex() {
	return memIndex;
}
//End Code Changes by Chau Cao
