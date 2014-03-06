#include <stdbool.h>
#include <stdio.h>
#include "../esh.h"

static bool 
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'prompt' initialized...\n");
    return true;
}

static char * 
prompt(void)
{
    // the prompt must be dynamically allocated
    return strdup("\e[38;5;208m\e[1m'\e[38;5;196m:\e[38;5;33m.\e[38;5;226m_\e[38;5;33m.\e[38;5;196m:\e[38;5;208m'\e[0m> ");
	return strdup("\e[38;5;208m\e[1mhokies\e[208;5;88m\e[38;5;15m\e[1m\e[38;5;15m:>\e[0m ");
    //	("\e[7minverted\e[0m \e[4m\e[32mUnderlined==\e[0m \e[1mBold\e[0m\e[44m:>\e[0m");
}

struct esh_plugin esh_module = {
    .rank = 10,
    .init = init_plugin,
    .make_prompt = prompt
};
