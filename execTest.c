#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv){

  printf("ARGV: \n");
  for(int i = 0; i < argc; ++i)
    printf("\t%s\n", argv[i]);

  char* target[argc + 2];
  for(int i = 0; i < argc; ++i)
    target[i] = argv[i];

  printf("TARGET: \n");
  for(int i = 0; i < argc; ++i)
    printf("\t%s\n", target[i]);
  target[argc] = "-1";
  target[argc + 1] = NULL;


  int childPID;
  switch(childPID = fork()){
    case -1:
      printf("Error\n");
    case 0: //If fork successfully creates child process
      execvp("ls", target);
      // execlp("ls", "argv[0]" , argv[1], NULL); //execvp - "v" accepts list as argument & "p" searches path for executable
      printf("Syserr\n");
    default: //Where parent process goes
      waitpid(childPID, NULL, WUNTRACED); //Waits for child process to finish to continue
  }
return 0;
}
