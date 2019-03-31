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
#include "noff.h"
#include "sysdep.h"
#include "translate.h"

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader(NoffHeader *noffH)
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

	NoffHeader noffH;
	unsigned int i, size, pAddr, counter;
	space = false;

	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) &&
			(WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

	// how big is address space?
	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize; // we need to increase the size
	// to leave room for the stack
	numPages = divRoundUp(size, PageSize);
	size = numPages * PageSize;

	// Print statements added here to direct the flow of traffic
	printf(" Main memory holds %d%s ", NumPhysPages, " pages.\n");
	printf("The current process uses %d%s ", numPages, " pages total.\n");
	printf("One page in main memory is  %d%s ", PageSize, " bytes long.\n");
	printf("The total page size for the process being executed is  %d%s", size, " bytes long.\n");

	space = true;

	/**
				Begin code changes by Gerald Frilot
				We want demand paging.
				One page at a time until the CPU requests the next one so we can
				fit more in memory.


		*/
	//OuterTable* tablePointer = pointTable;
	//pointTable[currentThread->getID()%4].pageTable = (void*) pageTable;

	//TranslationEntry* pointTable[4];

	pageTable = new TranslationEntry[numPages];
	for (i = 0; i < numPages; i++)
	{
		pageTable[i].virtualPage = i; // for now, virtual page # = phys page #
		pageTable[i].physicalPage = -1;
		//**********************************************
		// VALID BIT SET TO FALSE HERE BY GERALD FRILOT
		//**********************************************
		pageTable[i].valid = FALSE;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE; // if the code segment was entirely on
																	 // a separate page, we could set its
																	 // pages to be read-only

		//*******************************************************************************************************************
		//  Created by Gerald Frilot
		//  Created a char array of size 10 elements
		// 	snprintf receives an array ,size of the array , and a string following that is placed into the array
		//  Instantiated a boolean var equal to the Creation of a global file system pointer to the char array and its size
		//  fileSwapper is an Openfile var that is declared in the addspace header that gets assigned to Open of the swapArray
		//  To create a file, you need a name for it and a size.
		//*********************************************************************************************************************
		char swapArray[20] = {NULL};

		snprintf(swapArray, 10, "%i.swap", currentThread->getID()); // Does not allow overflow

		bool fileCreation = fileSystem->Create(swapArray, size);
		fileSwapper = fileSystem->Open(swapArray);

		//****************************************************************************************************
		// Created by Gerald Frilot
		// Two buffers created
		// One of size noffH.code and another of size noffH.initdata
		// The executable will read instructions at the beginning of the buffer to the
		// size of the buffer at position
		// The fileswapper pointer is writing at the buffer of code size at 0th position.
		// Init data position happens at noffh.code.size
		// The entire code size must be passed here so the CPU knows when to stop executing
		// Fragments are placed into mainmemory in the size of 256 bytes wide.
		// Executable reads first and an interrupt is triggered.
		// The fileswapper pointer takes control and assists during the interrupts.
		//****************************************************************************************************

		if (noffH.code.size > 0)
		{

			char *noffcode = new char[noffH.code.size];
			//fileSwapper does a read from the buffer, this many bytes ,at position 0
			executable->ReadAt(noffcode, noffH.code.size, noffH.code.inFileAddr);

			//fileSwapper does a write to the buffer, this many bytes ,at position 0
			fileSwapper->WriteAt(noffcode, noffH.code.size, 0);
		}
		if (noffH.initData.size > 0)
		{

			char *initNoffCode = new char[noffH.initData.size];
			executable->ReadAt(initNoffCode, noffH.initData.size, noffH.initData.inFileAddr);
			fileSwapper->WriteAt(initNoffCode, noffH.initData.size, noffH.code.size);
		}
	}
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

//Because the initialization already zeroes out the memory to be used,
//is it even necessary to clear out any garbage data during deallocation?

AddrSpace::~AddrSpace()
{
	// Only clear the memory if it was set to begin with
	// which in turn only happens after space is set to true

	if (space)
	{
		if (replaceChoice == 0)
		{
			for (int i = 0; i < numPages; i++) // We need an offset of startPage + numPages for clearing.

				if (pageTable[i].physicalPage != -1)
					memMap->Clear(pageTable[i].physicalPage);
		}
		delete[] pageTable;

		memMap->Print();
	}
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void AddrSpace::InitRegisters()
{
	int i;

	for (i = 0; i < NumTotalRegs; i++)
		machine->WriteRegister(i, 0);

	// Initial program counter -- must be location of "Start"
	machine->WriteRegister(PCReg, 0);

	// Need to also tell MIPS where next instruction is, because
	// of branch delay possibility
	machine->WriteRegister(NextPCReg, 4);

	// Set the stack register to the end of the address space, where we
	// allocated the stack; but subtract off a bit, to make sure we don't
	// accidentally reference off the end!
	machine->WriteRegister(StackReg, numPages * PageSize - 16);
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

/*
	Begin code by Gerald Frilot
	Swapwriting method receives a virtual address and an evict notice
	It returns the virtual Location index used.
	enter receives the specific pagetable index passed from virtualLocation
	The fileswapper writes a virtual page in mainMemory physical location
	256 bytes wide from the virtualLocations position
	Writing the page to main memory first asigns a boolean flag named evict
	ofr the page that needs to be replaced by a read.
	Return the virtual location for further analysis.

*/
int AddrSpace::swapWriting(int virtualLocation, bool evict)
{
	TranslationEntry enter;

	enter = pageTable[virtualLocation];
	fileSwapper->WriteAt(&(machine->mainMemory[enter.physicalPage * PageSize]),
											 PageSize, virtualLocation * PageSize);

	if (evict)
	{
		enter.physicalPage = -1;
		enter.valid = FALSE;
	}

	return virtualLocation;
}

/*

	Begin code by Gerald Frilot
	Swapreading method takes in two parameters of type int and
	Following an interrupt is an infinite loop that sends the CPU looking
	for a match from its virtual address to a physical address with a valid bit set to true.
	swapReading receives the virtual address and the swapfile pointer is responsible for
	getting the data where it needs to be.


*/
int AddrSpace::swapReading(int fromVirtualLocation, int PhysicalLocation)
{

	fileSwapper->ReadAt(&(machine->mainMemory[PhysicalLocation * PageSize]),
											PageSize, fromVirtualLocation * PageSize);
	pageTable[fromVirtualLocation].valid = true;
	pageTable[fromVirtualLocation].physicalPage = PhysicalLocation;

	return 0;
}
