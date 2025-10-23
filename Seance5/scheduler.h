#pragma once

typedef void (*asyncfn)();

void init_scheduler(int delay_ms);

int thread_create(asyncfn);

void yield();

/* start running scheduler until all tasks terminate */
void start_scheduler(void);

void print_adress(void);