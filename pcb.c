/*
 * Project 1
 *
 * Authors: Keegan Wantz, Carter Odem, Connor Lundberg
 * TCSS 422.
 */
 
 
 /*
	- Fixed terminate count and made sure to randomize it upon creation.
 
 
 */

#include"pcb.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int global_largest_PID = 0;

/*
 * Helper function to iniialize PCB data.
 */
void initialize_data(/* in-out */ PCB pcb) {
  pcb->pid = 0;
  PCB_assign_priority(pcb, 0);
  pcb->size = 0;
  pcb->channel_no = 0;
  pcb->state = 0;

  pcb->mem = NULL;

  pcb->context->pc = 0;
  pcb->context->ir = 0;
  pcb->context->r0 = 0;
  pcb->context->r1 = 0;
  pcb->context->r2 = 0;
  pcb->context->r3 = 0;
  pcb->context->r4 = 0;
  pcb->context->r5 = 0;
  pcb->context->r6 = 0;
  pcb->context->r7 = 0;
  
  pcb->max_pc = createMaxPC();
  pcb->creation = 0;
  pcb->termination = 0;
  pcb->terminate = rand() % MAX_TERM_COUNT;
  pcb->term_count = 0;
  pcb->waiting_timer = -1;
  pcb->io_trap_1 = malloc(sizeof(unsigned int) * 4);
  pcb->io_trap_2 = malloc(sizeof(unsigned int) * 4);
  populateIOTrap(pcb, 1);
  populateIOTrap(pcb, 2);
  
}

/*
 * Allocate a PCB and a context for that PCB.
 *
 * Return: NULL if context or PCB allocation failed, the new pointer otherwise.
 */
PCB PCB_create() {
    PCB new_pcb = malloc(sizeof(PCB_s));
    if (new_pcb != NULL) {
        new_pcb->context = malloc(sizeof(CPU_context_s));
        if (new_pcb->context != NULL) {
            initialize_data(new_pcb);
			PCB_assign_PID(new_pcb);
        } else {
            free(new_pcb);
            new_pcb = NULL;
        }
    }
	
    return new_pcb;
}

/*
	Creates a max PC for a running process.
*/
unsigned int createMaxPC() {

	int rand_max_pc = rand() % MAX_PCB;
	
	if (rand_max_pc < MIN_PCB) {
		rand_max_pc += rand() % MIN_PCB;
	}
	
	// printf("\nRAND MAC PC: %d\n", rand_max_pc);
	
	return rand_max_pc;
	
}

void populateIOTrap(PCB pcb, int io_array) {
	
	int i = 0;
	int rand_val = 0;
	
	for (i; i < 4; i++) {
		rand_val = rand() % pcb->max_pc;
	
		// if random value already exists in I/O trap, 
		// increase value
		while (ioTrapValueExists(pcb, rand_val)) {
			rand_val++;
		}
		
		
		if (io_array == 1) {
			pcb->io_trap_1[i] = rand_val;		
		} else {
			pcb->io_trap_2[i] = rand_val;
		}
		
	}
	
	
	// TODO: Remove later. Just for testing purposes.
	if (io_array == 1) {
		printf("\n");
		printf("IO TRAP 1: ");
	
		for (i = 0; i < 4; i++) {
			printf("%d ", pcb->io_trap_1[i]);
		}
		printf("\n");
		
	} else {
		
		printf("IO TRAP 2: ");
		for (i = 0; i < 4; i++) {
			printf("%d ", pcb->io_trap_2[i]);
		}
		
		printf("\n");
			
	}
	

	

	
}

int ioTrapValueExists(PCB pcb, int rand) {
	
	for (int i = 0; i < 4; i++) {
		if (rand == pcb->io_trap_1[i] || rand == pcb->io_trap_2[i]) {
			return 1;
			break;
			
		}
		
	}
	
	return 0;
	
}

/*
 * Frees a PCB and its context.
 *
 * Arguments: pcb: the pcb to free.
 */
void PCB_destroy(/* in-out */ PCB pcb) {
	free(pcb->context);
	free(pcb->io_trap_1);
	free(pcb->io_trap_2);
	free(pcb);// that thing

	  
}

/*
 * Assigns intial process ID to the process.
 *
 * Arguments: pcb: the pcb to modify.
 */
void PCB_assign_PID(/* in */ PCB the_PCB) {
    the_PCB->pid = global_largest_PID;
    global_largest_PID++;
}

/*
 * Sets the state of the process to the provided state.
 *
 * Arguments: pcb: the pcb to modify.
 *            state: the new state of the process.
 */
void PCB_assign_state(/* in-out */ PCB the_pcb, /* in */ enum state_type the_state) {
    the_pcb->state = the_state;
}

/*
 * Sets the parent of the given pcb to the provided pid.
 *
 * Arguments: pcb: the pcb to modify.
 *            pid: the parent PID for this process.
 */
void PCB_assign_parent(PCB the_pcb, int the_pid) {
    the_pcb->parent = the_pid;
}

/*
 * Sets the priority of the PCB to the provided value.
 *
 * Arguments: pcb: the pcb to modify.
 *            state: the new priority of the process.
 */
void PCB_assign_priority(/* in */ PCB the_pcb, /* in */ unsigned int the_priority) {
    the_pcb->priority = the_priority;
    if (the_priority > NUM_PRIORITIES) {
        the_pcb->priority = NUM_PRIORITIES - 1;
    }
}

/*
 * Create and return a string representation of the provided PCB.
 *
 * Arguments: pcb: the pcb to create a string representation of.
 * Return: a string representation of the provided PCB on success, NULL otherwise.
 */
void toStringPCB(PCB thisPCB, int showCpu) {
	printf("contents: ");
	
	printf("PID: %d, ", thisPCB->pid);

	switch(thisPCB->state) {
		case STATE_NEW:
			printf("state: new, ");
			break;
		case STATE_READY:
			printf("state: ready, ");
			break;
		case STATE_RUNNING:
			printf("state: running, ");
			break;
		case STATE_INT:
			printf("state: interrupted, ");
			break;
		case STATE_WAIT:
			printf("state: waiting, ");
			break;
		case STATE_HALT:
			printf("state: halted, ");
			break;
	}
	
	printf("priority: %d, ", thisPCB->priority);
	printf("PC: 0x%04X, ", thisPCB->context->pc);
	
	if (showCpu) {
		printf("mem: 0x%04X, ", thisPCB->mem);
		printf("parent: %d, ", thisPCB->parent);
		printf("size: %d, ", thisPCB->size);
		printf("channel_no: %d ", thisPCB->channel_no);
		toStringCPUContext(thisPCB->context);
	}
}


void toStringCPUContext(CPU_context_p context) {
	printf(" CPU context values: ");
	printf("ir:  %d, ", context->ir);
	printf("psr: %d, ", context->psr);
	printf("r0:  %d, ", context->r0);
	printf("r1:  %d, ", context->r1);
	printf("r2:  %d, ", context->r2);
	printf("r3:  %d, ", context->r3);
	printf("r4:  %d, ", context->r4);
	printf("r5:  %d, ", context->r5);
	printf("r6:  %d, ", context->r6);
	printf("r7:  %d\r\n", context->r7);
}
 
