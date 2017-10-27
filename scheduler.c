/*
	10/12/2017
	Authors: Connor Lundberg, Carter Odem, Jasmine Dacones
	
	In this project we will be making a simple Round Robin scheduling algorithm
	that will take a single ReadyQueue of PCBs and run them through our scheduler.
	It will simulate the "running" of the process by randomly changing the PC value
	of the process as well as incorporating various interrupts to show the effects
	it has on the scheduling simulator.
	
	This file holds the defined functions declared in the scheduler.h header file.
*/

#include "scheduler.h"



unsigned int sysstack;
int switchCalls;

PCB privileged[4];
int privilege_counter = 0;
int ran_term_num = 0;
int terminated = 0;
int quantumSize;
int quantum_count;

int io_timer = 0;

int io_trap_1_status = 0;
int io_trap_2_status = 0;

/*
	This function is our main loop. It creates a Scheduler object and follows the
	steps a normal Round Robin Scheduler would to "run" for a certain length of time,
	push new PC onto stack when timer interrupt occurs, then call the ISR, scheduler,
	dispatcher, and eventually an IRET to return to the top of the loop and start
	with the new process.
*/
void timer () {
	unsigned int pc = 0;
	int totalProcesses = 0, iterationCount = 1;
	Scheduler thisScheduler = schedulerConstructor();
	for (;;) {
		if (totalProcesses >= MAX_PCB_TOTAL) {
			printf("Reached max PCBs, ending Scheduler.\r\n");
			break;
		}
		printf("Iteration: %d\r\n", iterationCount);
		if (!(iterationCount % RESET_COUNT)) {
			printf("\r\nRESETTING MLFQ\r\n");
			resetMLFQ(thisScheduler);
		}
		totalProcesses += makePCBList(thisScheduler);		
		
		if (totalProcesses > 1) {
			pc = runProcess(pc, quantumSize);
			sysstack = pc;
			terminate(thisScheduler); 
			pseudoISR(thisScheduler, TIMER_INT);
			pc = thisScheduler->running->context->pc;
		}
		
		printSchedulerState(thisScheduler);
		iterationCount++;
		
	}
	schedulerDeconstructor(thisScheduler);
}

// Newly constructed loop of timer()
/*
	

*/
void mainLoop() {
	
	int totalProcesses = 0, iterationCount = 1;
	Scheduler thisScheduler = schedulerConstructor();
	totalProcesses += makePCBList(thisScheduler);

	
	for (;;) {
		
		printf("Iteration: %d\r\n", iterationCount);
		thisScheduler->running->context->pc++;
		thisScheduler->running->term_count++; // ??
		
		if (checkTermination(thisScheduler) == 1) {
			terminate(thisScheduler);	
		}
	
		if (checkTimerInt() == 1) {
			pseudoISR(thisScheduler, TIMER_INT);
			iterationCount++;
			totalProcesses += makePCBList(thisScheduler);	
			printSchedulerState(thisScheduler);
		} 
		
		if (checkIoTrap(thisScheduler->running) == 1) {
			pseudoISR(thisScheduler, IO_TRAP);
		}
		
		if (checkIoInt(thisScheduler->interrupted) == 1) {
			pseudoISR(thisScheduler, IO_INT);
		}
		
		
		if (thisScheduler->running->context->pc == thisScheduler->running->max_pc) {
			thisScheduler->running->context->pc = 0;
		}
		
		
		if (totalProcesses >= MAX_PCB_TOTAL) {
			printf("Reached max PCBs, ending Scheduler.\r\n");
			break;
		}
		
		printf("Iteration: %d\r\n", iterationCount);
		if (!(iterationCount % RESET_COUNT)) {
			printf("\r\nRESETTING MLFQ\r\n");
			resetMLFQ(thisScheduler);
		}
	}
	
	schedulerDeconstructor(thisScheduler);
}


/*
	This creates the list of new PCBs for the current loop through. It simulates
	the creation of each PCB, the changing of state to new, enqueueing into the
	list of created PCBs, and moving each of those PCBs into the ready queue.
*/
int makePCBList (Scheduler theScheduler) {
	int newPCBCount = rand() % MAX_PCB_IN_ROUND;
	//int newPCBCount = 1;
	
	int lottery;
	for (int i = 0; i < newPCBCount; i++) {
		PCB newPCB = PCB_create();
		newPCB->state = STATE_NEW;
		q_enqueue(theScheduler->created, newPCB);
		
		lottery = rand() % 8;
		
		// creates privileged pcb
		if (privilege_counter < 4 && (lottery == 3 || lottery == 5)) {
			privileged[privilege_counter] = newPCB;
		
			
			privilege_counter++;
		}
	}
	printf("Making New PCBs: \r\n");
	if (newPCBCount) {
		while (!q_is_empty(theScheduler->created)) {
			PCB nextPCB = q_dequeue(theScheduler->created);
			nextPCB->state = STATE_READY;
			toStringPCB(nextPCB, 0);
			printf("\r\n");
			pq_enqueue(theScheduler->ready, nextPCB);
		}
		printf("\r\n");

		if (theScheduler->isNew) {
			printf("Dequeueing PCB ");
			toStringPCB(pq_peek(theScheduler->ready), 0);
			printf("\r\n\r\n");
			theScheduler->running = pq_dequeue(theScheduler->ready);
			theScheduler->running->state = STATE_RUNNING;
			theScheduler->isNew = 0;
			// quantumSize = theScheduler->ready[0]->quantum_size;
		}
	}
	
	return newPCBCount;
}

/*
	Checks to see if the quantum count has reached the maximum
	value. If so, the quantum count is set back to 0 and throws a
	timer interrupt, which then calls the psuedoISR. If not, the
	quantum count is incremented by 1.
*/
int checkTimerInt() {
	if (quantum_count < quantumSize) {
		quantum_count++;
		return 0;
	} else {
		quantum_count = 0;
		return 1;
	}
}


/*
	Checks if the running PCB's PC has reached a trap in one
	of the I/O trap arrays. If so, an interrupt is thrown and
	the psuedoISR will run. If not, it just returns 0.
*/
int checkIoTrap(PCB running) {
	
	int i = 0;
	
	for (i; i < TRAP_COUNT; i++) {
		if (running->context->pc == running->io_trap_1[i]) {
			io_trap_1_status = 1;
			return 1;
			break;
		}
		
		if (running->context->pc == running->io_trap_2[i]) {
			io_trap_2_status = 1;
			return 1;
			break;
		}
	}
	
	return 0;
}



/*
	Checks if an I/O request has completed. If so, an interrupt is thrown and
	the psuedoISR will run. If not, it just returns 0.
*/
int checkIoInt(PCB blocked) {
	
	// ??
	if (io_timer == blocked->waiting_timer) {
		return 1;
	} else {
		io_timer++;
		return 0;
	}
	
}


/*
	Checks to see if the current termination count has reached its
	full run and if it's not a privileged PCB. If so, it returns 1
	and will be marked for termination. Otherwise, it returns 0 and
	continues its run.
*/
int checkTermination(Scheduler theScheduler) {
	
	if (theScheduler->running->termination > 0
		&& theScheduler->running->term_count >= theScheduler->running->termination
		&& isPrivileged(theScheduler->running) == 0) {
		return 1;
		
	}
	
	return 0;
}


/*
	Creates a random number between 3000 and 4000 and adds it to the current PC.
	It then returns that new PC value.
*/
unsigned int runProcess (unsigned int pc, int quantumSize) {
	//(priority * PRIORITY_JUMP_EXTRA is the difference in time slice length between
	//priority levels.
	unsigned int jump;
	if (quantumSize != 0) {
		jump = rand() % quantumSize;
	}
	
	pc += jump;
	return pc;
}

/*
	If a random generated value is 15 or less, a process will be terminated. But
	only if this process is not a "privileged PCB".
*/
void terminate(Scheduler theScheduler) {
	ran_term_num = rand() % RANDOM_VALUE;
	
	
	if (theScheduler->running != NULL && ran_term_num <= MAX_VALUE_PRIVILEGED 
	&& isPrivileged(theScheduler->running) == 0) {
		
		theScheduler->running->state = STATE_HALT;
	
	}
	
}


/*
	This acts as an Interrupt Service Routine, but only for the Timer interrupt.
	It handles changing the running PCB state to Interrupted, moving the running
	PCB to interrupted, saving the PC to the SysStack and calling the scheduler.
*/
void pseudoISR (Scheduler theScheduler, int interrupt_type) {
	if (theScheduler->running->state != STATE_HALT) {
		theScheduler->running->state = STATE_INT;
		theScheduler->interrupted = theScheduler->running;
		theScheduler->running->context->pc = sysstack;
	}

	scheduling(interrupt_type, theScheduler);
	pseudoIRET(theScheduler);
}


/*
	Prints the state of the Scheduler. Mostly this consists of the MLFQ, the next
	highest priority PCB in line, the one that will be run on next iteration, and
	the current list of "privileged PCBs" that will not be terminated.
*/
void printSchedulerState (Scheduler theScheduler) {
	printf("MLFQ state at iteration end\r\n");
	toStringPriorityQueue(theScheduler->ready);

	/*if (theScheduler->running && theScheduler->interrupted) {
		toStringPCB(theScheduler->running, 0);
		toStringPCB(theScheduler->interrupted, 0);
	}*/
	printf("\r\n");
	
	int index = 0;
	
	
	while(privileged[index] != NULL && index < MAX_PRIVILEGE) {
		printf("PCB PID %d, PRIORITY %d, PC %d\r\n", 
		privileged[index]->pid, privileged[index]->priority, 
		privileged[index]->context->pc);
		index++;
	}
	printf("\r\n");
	
	if (pq_peek(theScheduler->ready)) {
		printf("Going to be running next ");
		toStringPCB(theScheduler->running, 0);
		printf("\r\n");
		printf("Next highest priority PCB ");
		toStringPCB(pq_peek(theScheduler->ready), 0);
		printf("\r\n\r\n\r\n");
	} else {
		printf("Going to be running next ");
		if (theScheduler->running) {
			toStringPCB(theScheduler->running, 0);
		}

		printf("Next highest priority PCB contents: The MLFQ is empty!\r\n");
		printf("\r\n\r\n\r\n");
	}
}


/*
	Used to move every value in the MLFQ back to the highest priority
	ReadyQueue after a predetermined time. It does this by taking the first value
	of each ReadyQueue (after the 0 *highest priority* queue) and setting it to
	be the new last value of the 0 queue.
*/
void resetMLFQ (Scheduler theScheduler) {
	for (int i = 1; i < NUM_PRIORITIES; i++) {
		ReadyQueue curr = theScheduler->ready->queues[i];
		if (!q_is_empty(curr)) {
			if (!q_is_empty(theScheduler->ready->queues[0])) {
				theScheduler->ready->queues[0]->last_node->next = curr->first_node;
				theScheduler->ready->queues[0]->last_node = curr->last_node;
				theScheduler->ready->queues[0]->size += curr->size;
			} else {
				theScheduler->ready->queues[0]->first_node = curr->first_node;
				theScheduler->ready->queues[0]->last_node = curr->last_node;
				theScheduler->ready->queues[0]->size = curr->size;
			}
			resetReadyQueue(curr);
		}
	}
}


/*
	Resets the contents of each PCB in a queue.
*/
void resetReadyQueue (ReadyQueue queue) {
	ReadyQueueNode ptr = queue->first_node;
	while (ptr) {
		ptr->pcb->priority = 0;
		ptr = ptr->next;
	}
	queue->first_node = NULL;
	queue->last_node = NULL;
	queue->size = 0;
}

/*
	If the interrupt that occurs was a Timer interrupt, it will simply set the 
	interrupted PCBs state to Ready and enqueue it into the Ready queue. It then
	calls the dispatcher to get the next PCB in the queue.
*/
void scheduling (int interrupt_type, Scheduler theScheduler) {
	if (interrupt_type == TIMER_INT && theScheduler->running->state != STATE_HALT) {
		theScheduler->interrupted->state = STATE_READY;
		if (theScheduler->interrupted->priority < (NUM_PRIORITIES - 1)) {
			theScheduler->interrupted->priority++;
		} else {
			theScheduler->interrupted->priority = 0;
		}
		pq_enqueue(theScheduler->ready, theScheduler->interrupted);

		
		int index = isPrivileged(theScheduler->running);
		
		if (index != 0) {
			privileged[index] = theScheduler->running;
		}
		
		
	} else if (interrupt_type == IO_TRAP && io_trap_1_status == 1 && theScheduler->running->state != STATE_HALT) {
		theScheduler->running->state = STATE_WAIT;
		theScheduler->running->waiting_timer = quantumSize * (rand() % 3 + 1) + rand() % 100;
		q_enqueue(theScheduler->waiting_io_1, theScheduler->running);
		
		
	} else if (interrupt_type == IO_TRAP && io_trap_2_status == 1 && theScheduler->running->state != STATE_HALT) {
		theScheduler->running->state = STATE_WAIT;
		theScheduler->running->waiting_timer = quantumSize * (rand() % 3 + 1) + rand() % 100;
		q_enqueue(theScheduler->waiting_io_2, theScheduler->running);
		
		
	} else if (interrupt_type == IO_INT && theScheduler->running->state != STATE_HALT) {
		
	}
	
	if (theScheduler->running->state == STATE_HALT) {
		q_enqueue(theScheduler->killed, theScheduler->running);
		theScheduler->running = NULL;
		
		terminated++;
	}
	
	theScheduler->running = pq_peek(theScheduler->ready);
	
	if (terminated >= TOTAL_TERMINATED) {
		while(!q_is_empty(theScheduler->killed)) {
			PCB_destroy(q_dequeue(theScheduler->killed));
		}
	}

	
	dispatcher(theScheduler);
	
}


/*
	This simply gets the next ready PCB from the Ready queue and moves it into the
	running state of the Scheduler.
*/
void dispatcher (Scheduler theScheduler) {
	if (pq_peek(theScheduler->ready)->state != STATE_HALT) {
		quantumSize = getNextQuantumSize(theScheduler->ready);
		theScheduler->running = pq_dequeue(theScheduler->ready);
		theScheduler->running->state = STATE_RUNNING;
	}
}


/*
	This simply sets the running PCB's PC to the value in the SysStack;
*/
void pseudoIRET (Scheduler theScheduler) {
	theScheduler->running->context->pc = sysstack;
}


/*
	This will construct the Scheduler, along with its numerous ReadyQueues and
	important PCBs.
*/
Scheduler schedulerConstructor () {
	Scheduler newScheduler = (Scheduler) malloc (sizeof(scheduler_s));
	newScheduler->created = q_create();
	newScheduler->killed = q_create();
	newScheduler->blocked = q_create();
	newScheduler->ready = pq_create();
	newScheduler->waiting_io_1 = q_create();
	newScheduler->waiting_io_2 = q_create();
	newScheduler->running = NULL;
	newScheduler->interrupted = NULL;
	newScheduler->isNew = 1;
	
	return newScheduler;
}


/*
	This will do the opposite of the constructor with the exception of 
	the interrupted PCB which checks for equivalancy of it and the running
	PCB to see if they are pointing to the same freed process (so the program
	doesn't crash).
*/
void schedulerDeconstructor (Scheduler theScheduler) {
	q_destroy(theScheduler->created);
	q_destroy(theScheduler->killed);
	q_destroy(theScheduler->blocked);
	q_destroy(theScheduler->waiting_io_1);
	q_destroy(theScheduler->waiting_io_2);
	pq_destroy(theScheduler->ready);
	PCB_destroy(theScheduler->running);
	if (theScheduler->interrupted == theScheduler->running) {
		PCB_destroy(theScheduler->interrupted);
	}
	free (theScheduler);
}

/*
	Checks to see if a PCB is a "privileged PCB" by iterating through
	the queue.
*/
int isPrivileged(PCB pcb) {
	if (pcb != NULL) {
		for (int i = 0; i < 4; i++) {
			if (privileged[i] == pcb) {
				return i;
				break;
			}	
		}
	}
	
	return 0;	
}


void main () {
	// FILE *f;
    // f = freopen("mlfq-history.txt", "w", stdout);
	
	setvbuf(stdout, NULL, _IONBF, 0);
	time_t t;
	srand((unsigned) time(&t));
	sysstack = 0;
	switchCalls = 0;
	quantumSize = 0;
	quantum_count = 0;
	timer();
	// mainLoop();
}