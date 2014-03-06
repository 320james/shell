/*
 * A plug-in, which prints evaluates if a positive
 * integer is even or odd. Prints even if even, odd
 * if odd and "Must enter a integer" if
 * the number is not a integer
 *
 * esh> oddeven [integer]
 *
 * Marcus Tedesco and Michael Chang
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../esh.h"

static bool 
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'oddeven' initialized...\n");
    return true;
}

/* Tests if the input is odd or even
 * Prints odd, even, not an integer 
 * or an error message
 */
static bool
oddoreven(struct esh_command *cmd)
{
    if (strcmp(cmd->argv[0], "oddeven"))
        return false;

    if (cmd->argv[1] == NULL){
        printf("Must enter an integer as second argument\n");
        return true;
    }

    char * str = strdup(cmd->argv[1]);

    bool isAllDigits = true;

    //Check if all the input characters are digits
    //with the exception of a '-' at the beginning
    int i;
    for (i = 0; i < (int) strlen(str); i++){
        if (str[i] < '0' || str[i] > '9'){
            if(!(i == 0 && str[i] == '-'))
                isAllDigits = false;
        }
    }

    //Print odd or even if the input is all digits
    if(isAllDigits){
        int input = atoi(&str[(int)strlen(str)-1]);

        if(input == 0)
            printf("0 is neither odd nor even\n");
        else if(input%2 == 0)
            printf("%s is even\n", cmd->argv[1]);
        else
            printf("%s is odd\n", cmd->argv[1]);
    }
    else{
        printf("Input argument must enter an integer\n");
    }

    return true;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = oddoreven
};
