/* 
 Jarren Briscoe
 Shell created in C. Supports exit, pwd, cd (just "cd" goes to home directory), and cd DIRECTORY commands. Nanosh.c handles errors then passes unsupported commands to a fork.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

void cmdExit(int argc, char **argv) {
 //currently empty
}

// getParameters returns the argc, the number of words found in cmd
//  while populating argv with pointers to each word
int getParameters(char *cmd, char **argv) {
	char *token;
	int argc = 0;
	token = strtok(cmd, " \t\n");
	while (token != NULL) {
		argv[argc] = token;
		argc++;
		token = strtok(NULL, " \t\n");
	}
	argv[argc] = NULL;		// set last + 1 argument to NULL
	return argc;
}

int main(int argc, char **argv) {

	char cmd[1024];
	char *rc;
	int myArgc = 0;
	char *myArgv[1000];
	
	while (1) {
		printf("nanosh: ");
		fflush(stdout);
		rc = fgets(cmd, sizeof(cmd), stdin);
		if (rc == NULL) {
			exit(0);
		}
		myArgc = getParameters(cmd, myArgv);

		// if no words typed: restart loop
		if (myArgc == 0) {			
			continue;
		}

		// if the first word is "exit", terminate the program
		if (strcmp(myArgv[0], "exit") == 0) {
			if(myArgc == 1) exit(0);
			else{
				errno = 22;
				perror("exit command failed");
			}
			cmdExit(myArgc, myArgv);
			continue;
		}

		// add if statements here for the other internal commands
		else if (strcmp(myArgv[0], "pwd") == 0) {
			char pwd[2048];
			if(myArgc != 1){
				errno = EINVAL;
				perror("");
			}
			else if(getcwd(pwd, sizeof(pwd)) != NULL){
				printf("%s\n", pwd);
			}
			continue;
		}
		else if (strcmp(myArgv[0], "cd") == 0) {
			if(myArgc == 1){
				chdir(getenv("HOME"));
				continue;
			}
			else if(myArgc > 2){
				errno = EINVAL;
				perror("");
				continue;
			}
			char myPath[2048];
			//manipulate path text
			_Bool absolute = 1;
			if(myArgv[1][0] != '/'){
				absolute = 0;
				char myPath[2048];
				getcwd(myPath, sizeof(myPath));				
				strcat(myPath, "/");
				strcat(myPath, myArgv[1]);
			}
			//absolute path
			if(absolute){
				if(opendir(myArgv[1]) != NULL){
					chdir(myArgv[1]);
					continue;
				}
			}
			else if(opendir(myPath) != NULL){
				chdir(myPath);
				continue;
			}
			errno = ENOENT;
			perror("");
			continue;
		}
		//white space already ignored
		//   and a default action that calls a function to fork()
		else{
			int status;
			pid_t myFork = fork();
			if(myFork > 0){
				//wait for child process
				waitpid(myFork, &status, 0);
			}
			//   and exec() while the parent issues waitpid()
			else if(myFork == 0){
				execvp(myArgv[0], myArgv);
				//if execvp didn't work
				errno = ENOENT;
				perror("Child failed");
				exit(1);
			}
			else{
				//random error message
				errno = 33;
				perror("Fork failed");
			}
			continue;
		}
	}

	return 0;
} 
