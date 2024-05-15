# Mini shell

This assignment involves implementing a mini-shell called msh (Mini-SHell) in C. The shell will support parsing user input, executing pipelines of commands, handling foreground and background tasks, and performing file redirection. The assignment is divided into four milestones:


Milestone 0: Parsing Sequences, Pipelines, and Commands: In this milestone, the focus is on parsing the shell's input and creating data structures to represent sequences, pipelines, and commands.


Milestone 1: Executing Commands and Pipelines: The goal of this milestone is to implement the execution of commands and pipelines. This involves using fork, exec, pipe, dup, and close system calls to execute commands in pipelines.


Milestone 2: Job Control: In this milestone, job control functionality is added to the shell. This includes handling signals (e.g., SIGCHLD, SIGTERM, SIGCONT), tracking background processes, and allowing foreground tasks to be interrupted or put into the background.


Milestone 3: File Redirection: The final milestone focuses on implementing file redirection. This includes redirecting the standard output and standard error of commands to specified files using the > and >> operators.


Additionally, there is an opportunity for extra credit, which involves implementing additional job control features and an autocomplete functionality using a prefix trie (ptrie).


The code organization includes a msh_main.c file that contains the main loop of the shell. The mshparse directory holds the files specific to parsing the shell input, which will be built into a library. You can add additional .c files in the same directory.

