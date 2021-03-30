/**
 * 测试 gdb watch 调试
 * 
 * (gdb) watch * [addr]
 * 
 * 作者：荣涛
 * 日期：2021年3月30日
 */

#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#define NR_CELLS 8
#define NR_VTY  NR_CELLS

struct cell_struct {
	int id;
	char name[60];
};

struct vty_struct {
    int id;
    int task_id;
    int sock_id;
	char name[60];
};

static struct cell_struct *my_cells = NULL;
static struct vty_struct *my_vty = NULL;

static void set_cell(struct cell_struct *cell, int id, char *name) {
	assert(cell && "set_cell: Cell is NULL.");
	cell->id = id;
	snprintf(cell->name, sizeof(cell->name), "%s:%d", name, id);
}
static void set_vty(struct vty_struct *vty, int id, char *name) {
	assert(vty && "set_vty: VTY is NULL.");
	vty->id = id;
    vty->task_id = 0xff33 + id;  //创建进程
    vty->sock_id = 0xee44 + id;  //创建 socket
	snprintf(vty->name, sizeof(vty->name), "%s:%d", name, id);
}
void show_cells() {
	assert(my_cells && "show_cells: Cell is NULL.");
	int i;
	for(i=0; i< 8; i++) { 
		printf("%4d - %s\n", my_cells[i].id, my_cells[i].name);
	}
}
void show_vtys() {
    printf("--------------------------------\n");
	assert(my_vty && "show_vtys: VTY is NULL.");
	int i;
	for(i=0; i< 8; i++) { 
		printf("%4d - %s, %x, %x\n", my_vty[i].id, my_vty[i].name, my_vty[i].task_id, my_vty[i].sock_id);
	}
}

void* test_task_fn(void* unused)
{
	printf("new thread.\n");	

	struct cell_struct *new_cells = malloc(sizeof(struct cell_struct)*NR_CELLS);
	struct vty_struct *new_vtys = malloc(sizeof(struct vty_struct)*NR_VTY);

	int i;
	for(i=0; i<NR_CELLS; i++) {
		set_cell(&new_cells[i], i+1, "cell");
	}
	my_cells = new_cells;
	for(i=0; i<NR_VTY; i++) {
		set_vty(&new_vtys[i], i+1, "vty");
	}
	my_vty = new_vtys;

    pthread_exit(NULL);
}



/* The main program. */
int main ()
{
	pthread_t thread_id;
    
	pthread_create(&thread_id, NULL, test_task_fn, NULL);

	pthread_join(thread_id, NULL);

	printf("task exit.\n");

    show_vtys();

    /* 将 越界，写坏 `my_vty` */
    memset(&my_cells[NR_CELLS], 0x0, sizeof(struct cell_struct)*4);
    
//    show_cells();
    show_vtys();

    memset(&my_vty[0], 0xff, sizeof(struct vty_struct));
    
    show_vtys();

    printf("Exit program.\n");

	return 0;
}

