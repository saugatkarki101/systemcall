// The MIT License (MIT)
//
// Copyright (c) 2016, 2017, 2020 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports five arguments

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  int token_index  = 0;
  int n;
  int pid_index = 0;
  int pid_hist[15];
  char *hist[15];
  for(int t=0;t<15;t++)
  {
    hist[t]= malloc(sizeof(char));
  }
  int hist_index=0;
  int count=0;
  int q =1;
  char str[5];
  int hist_len;
  char userCommand[15];

  while( 1 )
  {
    //to check if we successfully executed !n command and got out of for loop
    int check=0;

    if(q==0)
    {
       strcpy(cmd_str, userCommand) ;
       q=1;
    }
    else
    {  // Print out the msh prompt
      printf ("msh> ");
      // Read the command from the commandline.  The
      // maximum command that will be read is MAX_COMMAND_SIZE
      // This while command will wait here until the user
      // inputs something since fgets returns NULL when there
      // is no input
      while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
    }

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;

    //saveoff command string for history
    strcpy(hist[hist_index], cmd_str);
    hist_index++;
    if(hist_index>14) hist_index =0;


    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    for( token_index = 0; token_index < token_count; token_index ++ )
    {

      for(int i=0;i<15;i++)
      {
        sprintf(str,"!%d",i);
        int k = hist_index;
        if(strcmp(str,token[token_index])==0)
        {
          //Checking if the nth command exists in our command history
          if(i>k)
          {
            printf("Command not in history.\n");
            break;
          }
          q=0;
          strcpy(userCommand,hist[i]);
          check =1;
        }
      }

      if(check ==1)
      {
        break;
      }

     if(token[token_index] == NULL)
      {
        break;
      }

    else if((strcmp(token[token_index],"exit")==0)||(strcmp(token[token_index],"quit")==0))
      {
       exit(0);
      }

    else if(strcmp(token[token_index],"cd")==0)
    {
      //needs to be done in the Parent
      chdir(token[token_index+1]);
      break;
    }

    else if(strcmp(token[token_index],"showpids")==0)
    {
      //this sets the initial spot to print which will be our oldest.
      if(pid_index<15)
      {
        for(int i=1;i<=pid_index;i++)
          {
            printf("%d \n",pid_hist[i]);
          }
      }

      else
      {
        int index = pid_index;
        for(int i=0;i<15;i++)
        {
          printf("%d \n",pid_hist[pid_index]);
          if(index>14) index =0;
        }

      }
      break;
    }


    else if(strcmp(token[token_index],"history")==0)
    {
      int k = hist_index;
      for(int k=0;k<hist_index;k++)
      {
        printf("[%d] %s\n",k,hist[k]);
      }
      break;
    }

    else
    {
      pid_index++;
      if(pid_index>14) pid_index=0;
      pid_hist[pid_index] = fork(); //saving off return from fork into pid array
      if(pid_hist[pid_index]==0)
      {
        n = execvp(token[token_index],&token[token_index]);
        if(n==-1)
        {
        perror("execvp error!");
        }
      }

      else
      {
        int status;
        wait(&status);
      }
      break;
    }

    printf("%s: Command not found.\n",token[token_index]);
  }
    free( working_root );
  }

  return 0;
}
