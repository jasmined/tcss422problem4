/*
 * Project 1
 *
 * Authors: Keegan Wantz, Carter Odem, Connor Lundberg
 * TCSS 422.
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
	
	int i = 0;
	
	new_pcb->io_trap_1[i] = (rand() % 15) * 2;
	new_pcb->io_trap_2[i] = (rand() % 15) * 2;
	
	
	for (i = 1; i < 4; i++) {
		new_pcb->io_trap_1[i] = new_pcb->io_trap_1[i - 1] + (rand() % 30) * 5;
		new_pcb->io_trap_2[i] = new_pcb->io_trap_1[i - 1] + (rand() % 30) * 5;
	}
	printf("IO TRAP 1: ");
		for (i = 0; i < 4; i++) {
		new_pcb->io_trap_1[i] = new_pcb->io_trap_1[i - 1] + (rand() % 30) * 5;
		new_pcb->io_trap_2[i] = new_pcb->io_trap_1[i - 1] + (rand() % 30) * 5;
		printf("%d ", new_pcb->io_trap_1[i]);
	}
	printf("\n");
	printf("IO TRAP 2: ");
		for (i = 0; i < 4; i++) {

		printf("%d ", new_pcb->io_trap_2[i]);
	}
	
	printf("\n");
	
    return new_pcb;
}

/*
 * Frees a PCB and its context.
 *
 * Arguments: pcb: the pcb to free.
 */
void PCB_destroy(/* in-out */ PCB pcb) {
	  free(pcb->context);
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
 
/*char * toStringPCB(/* in  PCB the_pcb, int showAll) {
    /* Oversized buffer for creating the initial version of the string. 
    char temp_buf[1000];
    unsigned int cpos = 0;

	if (showAll) {
		cpos += sprintf(temp_buf, "contents: PID: %d, Priority: %d, state: %u, "
				"memloc: %p size: %u channel: %X ",
				the_pcb->pid, the_pcb->priority, the_pcb->state,
				the_pcb->mem, the_pcb->size, the_pcb->channel_no);

		/* Append the context: 
		sprintf(temp_buf + cpos, "PC: 0x%04X, IR: %04X, "
				"r0: %04X, r1: %04X, r2: %04X, r3: %04X, r4: %04X, "
				"r5: %04X, r6: %04X, r7: %04X",
				the_pcb->context->pc, the_pcb->context->ir, the_pcb->context->r0,
				the_pcb->context->r1, the_pcb->context->r2, the_pcb->context->r3,
				the_pcb->context->r4, the_pcb->context->r5, the_pcb->context->r6,
				the_pcb->context->r7);
	} else {
		cpos += sprintf(temp_buf, "contents: PID: %d, Priority: %d, state: %u, "
				"memloc: %p ", the_pcb->pid, the_pcb->priority, the_pcb->state, the_pcb->mem);

		/* Append the context: 
		sprintf(temp_buf + cpos, "PC: 0x%04X", the_pcb->context->pc);
	}
	
    /* A string that can be returned and -not- go out of scope. 
    char * ret_val = malloc(sizeof(char) * (strlen(temp_buf) + 1));

    /* Make sure ret_val is not null before populating it. 
    if (ret_val != NULL) {
        strcpy(ret_val, temp_buf);
    }

    return ret_val;
}*/
