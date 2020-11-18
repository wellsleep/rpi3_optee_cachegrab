#include <stdio.h>

extern int REE_run(int);

int main() {
	printf("hello REE!\n");
	REE_run(888); // in cachegrab, the trigger buffer is '378' instead
	return 0;
}
