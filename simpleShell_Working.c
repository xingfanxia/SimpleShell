// simpleshell.c Assignment
// Assignment Authors: Aman Panda and Xingfan Xia
// April 16th 2016

/* Simple example of using execvp with helpful function to read a line from 
 * stdin and parse it into an array of individual tokens
 * Author: Sherri Goings
 * Last Modified: 1/18/2014
 */

// Import
#include    <stdlib.h>
#include    <stdio.h>
#include    <fcntl.h>
#include    <unistd.h>
#include    <string.h>
#include    <sys/types.h>
#include    <sys/wait.h>
#include    <errno.h>

// Function declaration
char** readLineOfWords();

// Main function
int main()
{
    // Parameter used in readLineOfWords()
    const int MAX_LENGTH = 128;

    // Loop forever
    while(1){

        // Prompt user to input a shell command
        printf("\nenter a shell command (e.g. ls): ");
        fflush(stdout);

        // Read input
        char** words = readLineOfWords();

        // Bool to ensure that input is valid
        int invalidEntry = 0;

        // Variable used to iterate
        int j = 0;

        // Checking if input is valid
        while(words[j] != NULL){
            for(int k = 0; k < strlen(words[j]); k++){
                int ascii = words[j][k];
                if(!((ascii>=65 && ascii<=90) || (ascii>=97 && ascii<=122) || ascii==95 || (ascii>=45 && ascii<=57) || ascii == 60 || ascii == 62 || ascii == 38 || ascii == 124)){
                    invalidEntry = 1;
                    break;
                }
                if(ascii == 38 && (words[j+1] != NULL || k!=strlen(words[j])-1)){
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

        // If input is valid
        if(invalidEntry==0){

            // Variables to keep track if we need to deal with I/O redirection
            // or pipes or an ampersand.
            int outRedirect=0;
            int indexAtOutRedirect = 0;
            int inRedirect=0;
            int indexAtInRedirect = 0;
            int ifPipe = 0;
            int ampersand = 0;

            // Prints the tokens in the array separated by spaces
            // Also, record indexing info about redirections we will 
            // deal with later.
            int i=0; 
            printf("\nyou entered: ");
            while (words[i] != NULL) {
                if(strcmp(words[i],">") == 0 && words[i+1] != NULL){
                    outRedirect = 1;
                    indexAtOutRedirect = i;
                }
                if(strcmp(words[i],"<") == 0 && words[i+1] != NULL){
                    inRedirect = 1;
                    indexAtInRedirect = i;
                }
                if(strcmp(words[i],"|") == 0 && words[i+1] != NULL){
                    ifPipe = 1;
                }
                if(strcmp(words[i],"&")==0){

                    // Get rid of the ampersand
                    ampersand = 1;
                    char** newWords[100] = {NULL};
                    for(int j = 0; j < i; j++){
                         newWords[j] = words[j];
                    }
                    words = newWords;
                    break;
                }
                // Print user input
                printf("%s ", words[i++]);
            }
            printf("\n\n");

            // Fork program into parent and child
            int pid = fork();

            // Re-initialize iterator
            i = 0;

            // First child thread will deal with pipe if necessary
            if(pid == 0 && ifPipe == 1){
                while(words[i] != NULL){
                    char *current_token = words[i];

                    // Pipe found, deal with it.
                    if (strcmp(current_token, "|") == 0){
                        words[i] = NULL; 

                        char ** arg1 = &words[i+1];

                        int fds[2]; //check if file is there
                        if (pipe(fds) == -1) break; //if no file, break and exit

                        pid_t pid = fork(); //fork child process
                        if (pid == -1) break; //if child is invalid, break and exit
                        if (pid == 0){//if there is a child
                            close(fds[1]); 
                            dup2(fds[0], STDIN_FILENO);
                            execvp(arg1[0], arg1);
                        }else{// if there is no child
                            close(fds[0]); 
                            dup2(fds[1], STDOUT_FILENO);
                        }
                        break;
                    }
                    i++;
                }
            }

            // Re-initialize iterator
            i=0;

            // No pipes. Child will execute command.
            if(pid == 0){

                // If there is I/O redirection
                if(outRedirect == 1 || inRedirect == 1){

                    // This block of code simply finds the end of the actual command
                    // in the words array
                    int endCommand = indexAtOutRedirect;
                    if(outRedirect == 1 && inRedirect == 1){
                        if(indexAtInRedirect < indexAtOutRedirect){
                            endCommand = indexAtInRedirect;
                            //printf("endCommand = %d \n", endCommand);
                        }
                        //printf("indexAtInRedirect = %d\n ", indexAtInRedirect);
                        //printf("indexAtOutRedirect = %d\n ", indexAtOutRedirect);
                    }
                    else if(outRedirect == 1){
                        endCommand = indexAtOutRedirect;
                    }
                    else{
                        endCommand = indexAtInRedirect;
                    }

                    // Instantiate a new array filled with NULL values.
                    // This array will contain the actual command that will be 
                    // executed
                    char** command[100] = {NULL};
                    for(int j = 0; j < endCommand; j++){
                        command[j] = words[j];
                    }

                    // String that will hold info retrieved from a file
                    char* inputString;

                    // If there is input redirection
                    if(inRedirect == 1){
                        int fileIndex = indexAtInRedirect + 1;
                        //Source: http://www.programiz.com/c-programming/examples/read-file
                        FILE * fPointer;
                        fPointer = fopen(words[fileIndex], "r");
                        if (fPointer == -1){
                            printf("%d\n", fPointer);
                            printf("%s\n", strerror(errno));
                            break;
                        }
                        int line = 0;
                        char fileContents[512];
                        while(fgets(fileContents, 512, fPointer)){
                            line++;
                        }
                        fclose(fPointer);
                        inputString = fileContents;
                        int x = 0;
                        while(command[x] != NULL){
                            x++;
                        }
                        command[x] = inputString; 
                    }

                    // If there is output redirection. Execute command.
                    if(outRedirect == 1){
                        char* outFileName = words[indexAtOutRedirect + 1];
                        int pid, status;
                        int newfd = open(outFileName,O_CREAT|O_WRONLY, 0644);
                        dup2(newfd, 1);
                        execvp(command[0],command);
                        dup2(1,newfd);
                    }

                    //Execute command if only input redirection.
                    execvp(command[0], command);
                }
                else{
                    // Execute command if no I/O redirection and no pipes.
                    execvp(words[0],words);
                }
            }
            else{
                // If there was no ampersand, wait for the child process to terminate.
                if(ampersand!=1){
                    int returnStatus;    
                    waitpid(0, &returnStatus, 0);
                }
                // Else, there was an ampersand, and parent should not wait.
            }
        }
    }
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

