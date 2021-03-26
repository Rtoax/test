#include <stdio.h>
#include <malloc.h>

typedef struct CuIncMsg {
	int refCnt;
	char *pMsg;
}CuIncMsg_t;


int cucReleaseIncMsg(CuIncMsg_t **pIncMsg) {
	free((*pIncMsg)->pMsg);
	free(*pIncMsg);
}

int main() {
	int *i = malloc(64);
	int I[12] = {0};

	printf("%p,%p\n", i, I);


	CuIncMsg_t *pMsg = NULL;
	CuIncMsg_t *__pMsg = malloc(sizeof(CuIncMsg_t));
	pMsg = __pMsg;

	cucReleaseIncMsg(&pMsg);

	free((void*)0x110046);
}
