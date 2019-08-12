#include <stdio.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

#define SYSCALL_ADDED 354

int main(int argc, char *argv[]){
	
	long int ret;

	ret = syscall(SYSCALL_ADDED);
	printf("Return Value from syscall is: %d\n", ret);
	printf("Please check system console(using \" dmsg \" for system call output.\n");

	return 0;
}