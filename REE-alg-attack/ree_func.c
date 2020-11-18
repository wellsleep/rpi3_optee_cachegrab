#include <stdio.h>

extern int REE_run(int);

int main() {
	printf("hello REE!\n");
	REE_run(888);
	return 0;
}
