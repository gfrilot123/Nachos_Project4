// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include <stdio.h>        // FA98
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "addrspace.h"   // FA98
#include "sysdep.h"   // FA98

// begin FA98

// ----------------------------------------------------------------------------
// processCreator() function created for forking Join() method
void setValid();
void processCreator(int arg);
static int SRead(int addr, int size, int id);
static void SWrite(char *buffer, int size, int id);

// end FA98

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

//---------------------------------------------------------------------
// Global variables declared for switch cases Exec() and Join()


int i,j,k,buffAdd;
AddrSpace *space;
char *file;
Thread *t;
processNode * newProcess;
//*********************************************************************
// Begin code changes by Gerald Frilot
// Global variable used to determine what page is next
//*********************************************************************
int pageRequested;
//*********************************************************************
// End code changes by Gerald Frilo
//*********************************************************************

//Predefined
void
ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);
	int arg1 = machine->ReadRegister(4);
	int arg2 = machine->ReadRegister(5);
	int arg3 = machine->ReadRegister(6);
	int Result;


	char *ch = new char [500];
	switch ( which )
	{
	case NoException :
		break;
	case SyscallException :

		// for debugging, in case we are jumping into lala-land
		// Advance program counters.
		machine->registers[PrevPCReg] = machine->registers[PCReg];
		machine->registers[PCReg] = machine->registers[NextPCReg];
		machine->registers[NextPCReg] = machine->registers[NextPCReg] + 4;

		switch ( type )
		{

//Funtionaility provided by Chau Cao to extend on Halt
		case SC_Halt :
			DEBUG('t',"Shutdown, initiated by user program.\n");
			//SExit(0);
			printf("\nShutdown initiated by user program. PID: %d\n", currentThread->getProcessId());
			printf("Memory available before deallocation: \n");
			memMap->Print();
			printf("\nMemory available after deallocation: \n");
			for(int i = currentThread->space->getMemIndex(); i < currentThread->space->getMemIndex() + currentThread->space->getNumPages(); i++) {
					memMap->Clear(i);
			}
			memMap->Print();
			interrupt->Halt();
			break;
//*************************************************************************************************
// SC_Exit method implemented by built in user function Exit()
//************************************************************************************************
		case SC_Exit :


				//Begin code changes by Chau Cao
					printf("\nExit() syscall received by process number: %d\n",currentThread->getProcessId());
					printf("Process completed with Exit() value: %d \n",arg1);
					SExit(arg1);
				//End code changes by Chau Cao

		break;

//********************************************************************************************
// Begin code changes by Gerald Frilot
// SC_Exec switch case call funtion Exec() with a filename parameter and Exec() Forks a thread to
// method processCreator()
//************************************************************************************************

//Edits provided by Chau Cao
		case SC_Exec :


			printf("\nEXEC() syscall invoked in ");
			printf(currentThread->getName());
			printf(" by ");
			buffAdd=machine->ReadRegister(4);
			if(buffAdd!=0)
			printf("PID :%d\n", currentThread->getProcessId());


			file = new char[100];
			if(!machine->ReadMem(buffAdd,1,&arg1))return;
			i=0;

			while(arg1!=0)
			{
				file[i]=(char)arg1;
				buffAdd+=1;
				i++;

				if(!machine->ReadMem(buffAdd,1,&arg1))return;
			}

			file[i]=(char)0;
			Exec(file);
			delete [] file;
			break;

//Functionaility implemented by Chau Cao
		case SC_Join:


				printf("\nJoin() is being called by PID: %d on PID: %d\n", currentThread->getProcessId(), arg1);
				Join(arg1);
				break;

//Functionaility implemented by Chau Cao
	 case SC_Yield:

			if(type==10)
			{
			printf("\nYield() syscall received by PID: %d ", currentThread->getProcessId());
			printf("\n");
	 		Yield();
			}
	 		break;

		// Predefined
		case SC_Read :
			if (arg2 <= 0 || arg3 < 0){
				printf("\nRead 0 byte.\n");
			}
			Result = SRead(arg1, arg2, arg3);
			machine->WriteRegister(2, Result);
			DEBUG('t',"Read %d bytes from the open file(OpenFileId is %d)",
			arg2, arg3);
			break;
		// Predefined
		case SC_Write :
			for (j = 0; ; j++) {
				if(!machine->ReadMem((arg1+j), 1, &i))
					j=j-1;
				else{
					ch[j] = (char) i;
					if (ch[j] == '\0')
						break;
				}
			}
			if (j == 0){
				printf("\nWrite 0 byte.\n");
				// SExit(1);
			} else {
				DEBUG('t', "\nWrite %d bytes from %s to the open file(OpenFileId is %d).", arg2, ch, arg3);
				SWrite(ch, j, arg3);
			}
			break;


			default :
			printf("Not a valid syscall\n");

			break;
		}
		break;

	// Predefined
	//Begin Code Changes by Chau Cao
	//Removed all assertions and exit gracefully
	case ReadOnlyException :
		puts ("ReadOnlyException");
		if (currentThread->getName() == "main")
		//ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;
	case BusErrorException :
		puts ("BusErrorException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;
	case AddressErrorException :
		puts ("Pointer out of Bounds");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(-1);
		//SExit(1);
		break;
	case OverflowException :
		puts ("OverflowException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;
	case IllegalInstrException :
		puts ("IllegalInstrException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;
	case NumExceptionTypes :
		puts ("NumExceptionTypes");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		SExit(1);
		break;


		//*********************************************************************
		// Begin code changes by Gerald Frilot
		// When a miss occurs, puts is a debug feature printing the parameter
		//to the console setValid is a fuction that captures the fault and
		// makes the changes necessary to bring the right page into main memory
		//*********************************************************************
		case PageFaultException:
			puts("PageFaultException - Miss\n");

			setValid();
			break;
			//*********************************************************************
			// End code changes by Gerald Frilot
			//*********************************************************************

			default :
		     printf("Unexpected user mode exception %d %d\n", which, type);
		      if (currentThread->getName() == "main")

		      SExit(1);
		break;

	}
	delete [] ch;
}

//Predefined
static int SRead(int addr, int size, int id)  //input 0  output 1
{
	char buffer[size+10];
	int num,Result;

	//read from keyboard, try writing your own code using console class.
	if (id == 0)
	{
		scanf("%s",buffer);

		num=strlen(buffer);
		if(num>(size+1)) {

			buffer[size+1] = '\0';
			Result = size+1;
		}
		else {
			buffer[num+1]='\0';
			Result = num + 1;
		}

		for (num=0; num<Result; num++)
		{  machine->WriteMem((addr+num), 1, (int) buffer[num]);
			if (buffer[num] == '\0')
			break; }
		return num;

	}
	//read from a unix file, later you need change to nachos file system.
	else
	{
		for(num=0;num<size;num++){
			Read(id,&buffer[num],1);
			machine->WriteMem((addr+num), 1, (int) buffer[num]);
			if(buffer[num]=='\0') break;
		}
		return num;
	}
}


//Predefined
static void SWrite(char *buffer, int size, int id)
{
	//write to terminal, try writting your own code using console class.
	if (id == 1)
	printf("%s", buffer);
	//write to a unix file, later you need change to nachos file system.
	if (id >= 2)
	WriteFile(id,buffer,size);
}

//********************************************************************************************
// Begin code changes by Gerald Frilot
// Method processCreator() receives an int argument and is forked by both cases in Exec() depending
// on what method forked it first for now.
//***************************************************************************************************
void processCreator(int arg)
{

	Yield();//<----This line added by Chau Cao
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	machine->Run();
	ASSERT(FALSE);
}

//*********************************************************************
//End code changes by Gerald Frilot
//*********************************************************************

//*********************************************************************
// Begin code changes by Gerald Frilot
// set Valid is the function called that loop from the page requested to
// the last instruction on that page.
// When the end of the page is reached, another exception is called
// and the exception is thrown again untill all pages are in
// mainmemory.
//*********************************************************************
void setValid()
{
	for(int i =pageRequested; i< currentThread->space->getNumPages();i++)
	{
	machine->pageTable[i].virtualPage = i;
	machine->pageTable[i].physicalPage =  i;
	machine->pageTable[i].valid = true;
	machine->pageTable[i].use = true;
	memMap->Mark(i);
	printf("Page availability after adding the process:\n ");
	memMap->Print();
	for(int j=i+1; j<i+2;j++)
	{

		printf("Page requested:%d%s ",j,"\n");
		machine->pageTable[j].valid = false;
		pageRequested=j;
		if(!machine->pageTable[j].valid)
		puts("PageFaultException - Miss\n");

	}
	printf("Page availability after adding the process:\n ");
	memMap->Print();
}


}
//*********************************************************************
// End code changes by Gerald Frilot
//*********************************************************************


// *******************************************************************************************
// Begin code changes by Gerald Frilot
// Method Exec() receives a file name and returns an int (SpaceId) value
// Contains a switch case that determines what process has rights to the Exec() call first for now
//*************************************************************************************************

//Code Editions added by Chau Cao to incoporate into global process list
//Process id functionalities added by Chau Cao
int Exec(char *name)
{
	printf("filename : ( %s ",name);
	printf(" )");
	printf("\n");
	t = new Thread(name);
	newProcess = new processNode; //added by Chau Cao
	OpenFile * executable = fileSystem->Open(name);
	interrupt->SetLevel(IntOff);
	space = new AddrSpace(executable);
	t->space=space;
	delete executable;
	t->Fork(processCreator, currentId);

	//Following are additions added by Chau Cao
	newProcess->process = t;
	newProcess->PID = currentId;
	newProcess->parentId = -1;
	newProcess->next = NULL;
	processList->next = newProcess;
	processList = newProcess;
	currentId++;
	machine->WriteRegister(2, processList->PID);
	//end code changes by Chau Cao

	interrupt->SetLevel(IntOn);
	return processList->PID;
}

//Begin Code Changes by Chau Cao
//Join function. References Global list
//Sets calling thread to sleep until child returns
int Join(int id)
{
	processNode * temp;
	temp = rootList;
	bool set = false;
	if(temp == NULL) {
		printf("Temp is null\n");
		SExit(1);
	}
	while(set != true) {
		if(temp==NULL)
		{

		SExit(1);
		}
		else
		{


		if(temp->PID == id){
			temp->parentId = currentThread->getProcessId();
			set = true;
		}
		else {
			temp = temp->next;
		}
	}

	}
	currentThread->space->SaveState();
	interrupt->SetLevel(IntOff);
	currentThread->Sleep();
	interrupt->SetLevel(IntOn);
	currentThread->space->RestoreState();
}

//Exit Functionality Provided by Chau Cao
//Switch case based on exit status provided to function
// ----case 0 ----
//	Program ended normally
//	Deallocates the memory used by the exiting process
//	Pulls up the exiting proccesses struct and checks if it has a parent
//	Searches for parent node with id found
//	If the parent isn't found, prvoide error
//----case 1 ----
//	Fatal memory error. Exits process
//----case -1----
//	Address Space Error. Exits process
//----Default Case----
// Generic possible non normal returns
void SExit(int status){
	int x = 0;
	processNode* temp;
	processNode* notherTemp;
	switch (status) {
		case 0:
			interrupt->SetLevel(IntOff);
			printf("The program exited normally. Exiting... \n");
			x = currentThread->space->getMemIndex();
			printf("Processes index in Memory: %d\n", x);
			printf("\nMemory available before deallocation: \n");
			memMap->Print();
			printf("\nMemory available after deallocation: \n");
			while(x < currentThread->space->getMemIndex() + currentThread->space->getNumPages())
			{
				memMap->Clear(x);
				x++;
			}
			memMap->Print();

			temp = rootList;
			while(temp != NULL) {
				if(temp->PID == currentThread->getProcessId())
				{
					break;
				}
				else {
					temp = temp->next;
				}
			}

			notherTemp = rootList;
			if(temp->parentId > -1)
			{
				while(notherTemp != NULL) {
					if(notherTemp->PID == temp->parentId){
						break;
					}
					notherTemp = notherTemp->next;
				}
				if(notherTemp != NULL) {
					if(notherTemp->process != NULL) {
						notherTemp->process->setStatus(READY);
						scheduler->ReadyToRun(notherTemp->process);
					}
					else {
						printf("The parent process has already finished running...\n");
					}
				}
				else {
					printf("End of global process structure reached. Could not locate Parent... \n");
				}
			}
			currentThread->Finish();
			break;
		case 1:
			printf("Fatal Exception Thrown. Exiting nachos...\n");
			currentThread->Finish();
			break;
		case -1:
			printf("Exiting process prior to loading into memory...\n");
			currentThread->Finish();
			break;
		default:

		/*
			removed by Gerald Frilot for testing of  demand paging
		*/
			//printf("This process returned a non standard exit status...STATUS: %d \n", status);
			//printf("Process: %s with PID: %d\n", currentThread->getName(), currentThread->getProcessId());
			//printf("Processes index in Memory: %d\n", currentThread->space->getMemIndex());
			//printf("Memory available before deallocation: \n");
			//memMap->Print();
			//printf("Memory available after deallocation: \n");
			//for(int i = currentThread->space->getMemIndex(); i < currentThread->space->getMemIndex() + currentThread->space->getNumPages(); i++) {
					//memMap->Clear(i);
			//}
			//memMap->Print();

			temp = rootList;
			while(temp != NULL) {
				if(temp->PID == currentThread->getProcessId())
				{
					break;
				}
				else {
					temp = temp->next;
				}
			}

			notherTemp = rootList;
			if(temp->parentId > -1)
			{
				while(notherTemp != NULL) {
					if(notherTemp->PID == temp->parentId){
						break;
					}
					notherTemp = notherTemp->next;
				}
				if(notherTemp != NULL) {
					if(notherTemp->process != NULL) {
						notherTemp->process->setStatus(READY);
						scheduler->ReadyToRun(notherTemp->process);
					}
					else {
						printf("The parent process has already finished running...\n");
					}
				}
				else {
					printf("End of global process structure reached. Could not locate Parent... \n");
				}
			}
			currentThread->Finish();
	}
}

//Code Addition by Chau Cao
//Yield system call functionality
//Saves user state and calls thread->Yield
//On return restores user state
void Yield() {
	//currentThread->space->SaveState();
	currentThread->SaveUserState();
	currentThread->Yield();
	currentThread->space->RestoreState();
	currentThread->RestoreUserState();
}

// end FA98
