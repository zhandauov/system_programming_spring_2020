
Compilation:
	The included Makefile will compile the user.c and kscan.c files for the user program and and kernel module respectively. The default “make” command will compile as well use insmod on the the compiled kscan.ko file afterwards. Be ready to input your password for sudo for the insmod command after compilation completes.

Functionality:

Process Search (ps): 
With the program compiled, using ./user ps followed by a process name will search through running tasks with the “for_each_process” macro from within <linux/sched.h>.
Using “./user ps ext” will return each running process with the substring “ext”. Using “./user ps” with no parameter will simply return every process currently running.

Kill by Process Name (kn):
With the program compiled, usering ./user kn followed by a process name will kill the process with that name.

Monitor System Calls (sc):
The system call monitor uses the ptrace system call to trace all system calls that a set of processes make. 
Usage example: ./user -sc 1234 6112 5532 9991
Traces system calls made by processes 1234, 6112, 5532, and 9991
