/* Simple example of using execvp with helpful function to read a line from 
 * stdin and parse it into an array of individual tokens
 * Author: Sherri Goings
 * Last Modified: 1/18/2014
 */

#include    <stdlib.h>
#include    <stdio.h>
#include    <unistd.h>
#include    <string.h>
#include    <sys/types.h>
#include    <sys/wait.h>
#include    <fcntl.h>
#include    <errno.h>


char** readLineOfWords();

int main()
{
    //int EOF = 0;
    //printf(EOF);
    int c;
  
  while (1)
  {
    printf("\nenter a shell command (e.g. ls): ");

    fflush(stdout);
    char** words = readLineOfWords();
    int invalidEntry = 0;
    int j = 0;
    while(words[j] != NULL){
        // if(words[j] == "-1"){
        //     EOF = -1;
        //     break;
        // }
        for(int k = 0; k < strlen(words[j]); k++){
            int ascii = words[j][k];
            if(!((ascii>=65 && ascii<=90) || !(ascii>=97 && ascii<=122) || !(ascii==95) || !(ascii>=45 && ascii<=57))){
                //printf("Invalid Entry");
                invalidEntry = 1;
                break;
            }
        }
        if(invalidEntry==1){
            printf("\nInvalid entry. Please try again.\n");
            break;
        }
        j++;
    }

    int pid = fork();

    if(pid == 0){

        {// io outredirection support
            int i = 0;
            while(words[i] != NULL){
                char *current_token = words[i];
                if (strcmp(current_token, ">") == 0){
                    words[i] = NULL; // single command support

                    char *file = words[i+1];
                    if (file == NULL) break;

                    int fd = open(file, O_RDWR | O_CREAT, 0x755);
                    if (fd == -1){
                        printf("%d\n", fd);
                        printf("%s\n", strerror(errno));
                        break;
                    }
                    dup2(fd, STDOUT_FILENO);
                    break;
                }
                i++;
            }
        }

        {// io inredirection support
            int i = 0;
            while(words[i] != NULL){
                char *current_token = words[i];
                if (strcmp(current_token, "<") == 0){
                    words[i] = NULL; // single command support

                    char *file = words[i+1];
                    if (file == NULL) break;

                    int fd = open(file, O_RDWR | O_CREAT, 0x755);
                    if (fd == -1){
                        printf("%d\n", fd);
                        printf("%s\n", strerror(errno));
                        break;
                    }
                    dup2(fd, STDOUT_FILENO);
                    break;
                }
                i++;
            }
        }

        {// pipe
            int i = 0;
            while(words[i] != NULL){
                char *current_token = words[i];
                if (strcmp(current_token, "|") == 0){
                    words[i] = NULL; 

                    char ** arg1 = &words[i+1];

                    int fds[2];
                    if (pipe(fds) == -1) break;

                    pid_t pid = fork();
                    if (pid == -1) break;
                    if (pid == 0){
                        close(fds[1]);
                        dup2(fds[0], STDIN_FILENO);
                        execvp(arg1[0], arg1);
                    }else{
                        close(fds[0]);
                        dup2(fds[1], STDOUT_FILENO);
                    }
                    break;
                }
                i++;
            }
        }
        execvp(words[0], words);
    }
    else{
        //printf("I'm the parent - %ld\n", pid, i);
        //fflush(stdout);
        int returnStatus;    
        waitpid(-1, &returnStatus, 0);

    }
  }
          // if(EOF == -1){
          //     return 0;
          // }
  
  // execute command in words[0] with arguments in array words
  // by convention first argument is command itself, last argument must be NULL
  //execvp(words[0], words);
 
  // execvp replaces current process so should never get here!
  //printf( "Will this get printed? No.\n" );
   return 0;
}

/* 
 * reads a single line from terminal and parses it into an array of tokens/words by 
 * splitting the line on spaces.  Adds NULL as final token 
 */
char** readLineOfWords() {

  // A line may be at most 100 characters long, which means longest word is 100 chars, 
  // and max possible tokens is 51 as must be space between each
  size_t MAX_WORD_LENGTH = 100;
  size_t MAX_NUM_WORDS = 51;

  // allocate memory for array of array of characters (list of words)
  char** words = (char**) malloc( MAX_NUM_WORDS * sizeof(char*) );
  int i;
  for (i=0; i<MAX_NUM_WORDS; i++) {
    words[i] = (char*) malloc( MAX_WORD_LENGTH );
  }

  // read actual line of input from terminal
  int bytes_read;
  char *buf;
  buf = (char*) malloc( MAX_WORD_LENGTH+1 );
  bytes_read = getline(&buf, &MAX_WORD_LENGTH, stdin);
 
  // take each word from line and add it to next spot in list of words
  i=0;
  char* word = (char*) malloc( MAX_WORD_LENGTH );
  word = strtok(buf, " \n");
  while (word != NULL && i<MAX_NUM_WORDS) {
    strcpy(words[i++], word);
    word = strtok(NULL, " \n");
  }

  // check if we quit because of going over allowed word limit
  if (i == MAX_NUM_WORDS) {
    printf( "WARNING: line contains more than %d words!\n", (int)MAX_NUM_WORDS ); 
  } 
  else
    words[i] = NULL;
  
  // return the list of words
  return words;
}

