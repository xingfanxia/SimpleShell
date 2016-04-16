## This is assignment for CS332 Operating System

### TO DOs:

- [x] Basic shell – use the example programs forktest.c and execexample.c to create a basic shell that
      reads a line of input, forks a child process to exec the given command, and waits for the child
      process to finish before looping to read the next line of input.


- [x] Add some processing of the array of tokens to determine which of the operators may be present
      (<, >, |, &)
- [x] Handle waiting vs. not waiting based on the &, and make sure you remove the & from the array
      before calling exec.
- [x] To support I/O redirection, modify the child process created by fork() by adding some code to
      open the input and output files specified on the command line. This should be done using the open() system call. Next, use the dup2() system call to replace the standard input or standard output streams with the appropriate file that was just opened. Finally, call execvp() to run the program after removing the redirect tokens from the array.
- [x] Pipes are a little trickier: you will need to call a fork/exec for each command on the line. Then you should use the pipe() system call to create a pair of pipe file descriptors before calling fork(). After the fork both processes will have access to both sides of the pipe. The reading process should immediately close the write file descriptor, and the writing process should immediately close the read file descriptor. At this point each process uses dup2() to copy the remaining pipe descriptor over STDIN or STDOUT as appropriate.
- [ ] Simplify Aman's version.

### Sherri's words:
- When you are done, bask in the glory of a working shell! You should now have a good operational understanding of the user-mode side of some of the most important UNIX system calls.

- Code Size: My finished shell is ~300 lines of C code, including plenty of debugging code, blank lines, and comments. Your shell should not be a lot larger than this (in other words, if it gets a lot larger then you’re probably doing something wrong).


