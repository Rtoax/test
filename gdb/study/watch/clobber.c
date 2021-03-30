#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

struct cell_struct {
	int id;
	char name[60];
};


static struct cell_struct *DU_CELLS = NULL;
static struct cell_struct *CUUP_CELLS = NULL;

static void set_cell(struct cell_struct *cell, int id, char *name) {
	assert(cell && "set_cell: Cell is NULL.");
	cell->id = id;
	snprintf(cell->name, sizeof(cell->name), "%s:%d", name, id);
}

void* test_task_fn(void* unused)
{
	printf("new thread.\n");	

	struct cell_struct *du_cells = malloc(sizeof(struct cell_struct)*8);
	struct cell_struct *cuup_cells = malloc(sizeof(struct cell_struct)*8);

	int i;
	for(i=0; i<8; i++) {
		set_cell(&du_cells[i], i+1, "ducell");
		set_cell(&cuup_cells[i], i+1, "cuupcell");
	}
	DU_CELLS = du_cells;
	CUUP_CELLS = cuup_cells;

    pthread_exit(NULL);
}

void show_du_cells() {
	int i;
	for(i=0; i< 8; i++) {
		printf("%4d - %s\n", DU_CELLS[i].id, DU_CELLS[i].name);
	}
}
void show_cuup_cells() {
	int i;
	for(i=0; i< 8; i++) { 
		printf("%4d - %s\n", CUUP_CELLS[i].id, CUUP_CELLS[i].name);
	}
}

/* The main program. */
int main ()
{
	pthread_t thread_id;
    
	pthread_create(&thread_id, NULL, test_task_fn, NULL);

	pthread_join(thread_id, NULL);

	printf("task exit.\n");

	int i;
	for(i=-8; i< 8; i++) {
		DU_CELLS[i*4].id = 1024;
		show_cuup_cells();
	}

    printf("Exit program.\n");

	return 0;
}

