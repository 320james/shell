/*
 * A plug-in, which inputs a sentence and converts it to
 * 1337 speak
 *
 * Marcus Tedesco & Michael Chang
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../esh.h"

static bool 
init_plugin(struct esh_shell *shell)
{
    printf("Plugin '1337' initialized...\n");
    return true;
}

/* Prints the command line arguments in 1337 speak   */
static bool
printleet(struct esh_command *cmd)
{
    if (strcmp(cmd->argv[0], "1337"))
        return false;

    //Needs additional arguments to run
    if (cmd->argv[1] == NULL){
        printf("/v\\|_|$7 !/\\/|D|_|7 @o|o|!7!0/\\/@1 @|26|_|/v\\3/\\/7$ (must input additional arguments)\n");
        return true;
    }

    int j = 1;

    //Gets all the words of the "sentence"
    while(cmd->argv[j] != NULL) {
        char * str = strdup(cmd->argv[j]);

        int i;

        //Print the 1337 characters associated with the current character
        for (i = 0; i < (int) strlen(str); i++){
            switch (str[i])
            {
              case 'a':
                 printf("@");
                 break;
              case 'A':
                 printf("@");
                 break;
              case 'b':
                 printf("|3");
                 break;
              case 'B':
                 printf("|3");
                 break;
              case 'c':
                 printf("<");
                 break;
              case 'C':
                 printf("<");
                 break;
              case 'd':
                 printf("o|");
                 break;
              case 'D':
                 printf("o|");
                 break;
              case 'e':
                 printf("3");
                 break;
              case 'E':
                 printf("3");
                 break;
              case 'f':
                 printf("ph");
                 break;
              case 'F':
                 printf("ph");
                 break;
              case 'g':
                 printf("6");
                 break;
              case 'G':
                 printf("6");
                 break;
              case 'h':
                 printf("]-[");
                 break;
              case 'H':
                 printf("]-[");
                 break;
              case 'i':
                 printf("!");
                 break;
              case 'I':
                 printf("!");
                 break;
              case 'j':
                 printf("_|");
                 break;
              case 'J':
                 printf("_|");
                 break;
              case 'k':
                 printf("|{");
                 break;
              case 'K':
                 printf("|{");
                 break;
              case 'l':
                 printf("1");
                 break;
              case 'L':
                 printf("1");
                 break;
              case 'm':
                 printf("/v\\");
                 break;
              case 'M':
                 printf("/v\\");
                 break;
              case 'n':
                 printf("/\\/"); 
                 break;
              case 'N':
                 printf("/\\/"); 
                 break;
              case 'o':
                 printf("0");
                 break;
              case 'O':
                 printf("0");
                 break;
              case 'p':
                 printf("|D");
                 break;
              case 'P':
                 printf("|D");
                 break;
              case 'q':
                 printf("kw");
                 break;
              case 'Q':
                 printf("kw");
                 break;
              case 'r':
                 printf("|2");
                 break;
              case 'R':
                 printf("|2");
                 break;
              case 's':
                 printf("$");
                 break;
              case 'S':
                 printf("$");
                 break;
              case 't':
                 printf("7");
                 break;
              case 'T':
                 printf("7");
                 break;
              case 'u':
                 printf("|_|");
                 break;
              case 'U':
                 printf("|_|");
                 break;
              case 'v':
                 printf("\\/");    
                 break;
              case 'V':
                 printf("\\/");    
                 break;
              case 'w':
                 printf("\\/\\/");   
                 break;
              case 'W':
                 printf("\\/\\/");    
                 break;
              case 'x':
                 printf("><");
                 break;
              case 'X':
                 printf("><");
                 break;
              case 'y':
                 printf("'/");
                 break;
              case 'Y':
                 printf("'/");
                 break;
              case 'z':
                 printf("2");
                 break;
              case 'Z':
                 printf("2");
                 break;
              default:
                printf("%c", str[i]);  
                break;
            }
        }
        printf(" ");
        j++;
    }

    printf("\n");

    return true;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = printleet
};
