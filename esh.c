/*
 * esh - the 'pluggable' shell.
 *
 * Developed by Godmar Back for CS 3214 Fall 2009
 * Virginia Tech.
 *
 * Marcus Tedesco & Michael Chang
 * mtedesco+mrchang
 * Feb. 26, 2014 
 *
 */
#include <stdio.h>
#include <readline/readline.h>
#include <unistd.h> 

#include "esh.h"

//Turns debug prints on and off
//#define DEBUG

static void
usage(char *progname)
{
    printf("Usage: %s -h\n"
        " -h            print this help\n"
        " -p  plugindir directory from which to load plug-ins\n",
        progname);

    exit(EXIT_SUCCESS);
}

/* Build a prompt by assembling fragments from loaded plugins that 
 * implement 'make_prompt.'
 *
 * This function demonstrates how to iterate over all loaded plugins.
 */
static char *
build_prompt_from_plugins(void)
{
    char *prompt = NULL;
    struct list_elem * e = list_begin(&esh_plugin_list);

    for (; e != list_end(&esh_plugin_list); e = list_next(e)) {
        struct esh_plugin *plugin = list_entry(e, struct esh_plugin, elem);

        if (plugin->make_prompt == NULL)
            continue;

        /* append prompt fragment created by plug-in */
        char * p = plugin->make_prompt();
        if (prompt == NULL) {
            prompt = p;
        } else {
            prompt = realloc(prompt, strlen(prompt) + strlen(p) + 1);
            strcat(prompt, p);
            free(p);
        }
    }

    /* default prompt */
    if (prompt == NULL)
        prompt = strdup("esh> ");

    return prompt;
}

/* The shell object plugins use.
 * Some methods are set to defaults.
 */
struct esh_shell shell =
{
    .build_prompt = build_prompt_from_plugins,
    .readline = readline,       /* GNU readline(3) */ 
    .parse_command_line = esh_parse_command_line /* Default parser */
};

/**
 * Assign ownership of ther terminal to process group
 * pgrp, restoring its terminal state if provided.
 *
 * Before printing a new prompt, the shell should
 * invoke this function with its own process group
 * id (obtained on startup via getpgrp()) and a
 * sane terminal state (obtained on startup via
 * esh_sys_tty_init()).
 */
static void
give_terminal_to(pid_t pgrp, struct termios *pg_tty_state)
{   
    #ifdef DEBUG
        printf("GIVING TERMINAL TO PGRP[%d]\n", (int)pgrp);
    #endif

    esh_signal_block(SIGTTOU);
    int rc = tcsetpgrp(esh_sys_tty_getfd(), pgrp);
    if (rc == -1)
        esh_sys_fatal_error("tcsetpgrp: ");

    if (pg_tty_state)
        esh_sys_tty_restore(pg_tty_state);
    esh_signal_unblock(SIGTTOU);
}

/* Print esh_pipeline for jobs 
 * Prints Running or Stopped followed by (literal command the user entered)
 */
static void
print_one_pipeline(struct esh_pipeline *pipe)
{
    struct list_elem * e = list_begin (&pipe->commands); 

    for (; e != list_end (&pipe->commands); e = list_next (e)) {
        struct esh_command *cmd = list_entry(e, struct esh_command, elem);
        if(pipe->status == 0 || pipe->status == 1)
            printf("Running                 (");
        else if(pipe->status == 2)
            printf("Stopped                 (");

        char **parts = cmd->argv;
        while (*parts) {
            printf("%s", *parts);
            fflush(stdout);
            parts++;
            if(*parts != NULL)
                printf(" ");
        }

        if (list_size(&pipe->commands) > 1) {
            printf(" | ");
        }
    }

    printf(")\n");
}

/* Print esh_command for fg 
 * Prints the literal command that was entered by the user
 */
static void
print_commands(struct esh_pipeline *pipe)
{
    struct list_elem * e = list_begin (&pipe->commands); 

    for (; e != list_end (&pipe->commands); e = list_next (e)) {
        struct esh_command *cmd = list_entry(e, struct esh_command, elem);
        
        char **parts = cmd->argv;
        while (*parts) {
            printf("%s", *parts);
            fflush(stdout);
            parts++;
            if(*parts != NULL)
                printf(" ");
        }

        if (list_size(&pipe->commands) > 1) {
            printf("| ");
        }
    }

    printf("\n");
}

/* Processes signal changes of a reaped children 
 * If the child has exited or terminated, remove the process and
 * give the terminal back to shell
 * If the child has stopped, change the status of the pipeline
 * and give the terminal back to the shell
 */
static void child_status_change(pid_t childpid, int status){
    #ifdef DEBUG
        printf("IN CHILD STATUS CHANGE\n");
    #endif

    if(childpid < 0)
        esh_sys_fatal_error("Error in status change ");

    //loop through the list of jobs
    struct list_elem * e;
    for (e = list_begin(&jobs_list); e != list_end(&jobs_list); e = list_next(e)) {
        //get a pipeline of this job
        struct esh_pipeline * pipe = list_entry(e, struct esh_pipeline, elem);
        struct list_elem * f;
        for (f = list_begin(&pipe->commands); f != list_end(&pipe->commands); f = list_next(f)) {
            struct esh_command * cmd = list_entry(f, struct esh_command, elem);

            if(cmd->pid == childpid){
                if( WIFEXITED(status)) {
                    #ifdef DEBUG
                        printf("Received signal that child process (%d) terminated. \n", childpid);
                        printf("Removing command from the pipeline\n");
                        printf("Pipeline status: %d\n", pipe->status);
                    #endif
                    list_remove(f);
                }
                if (WIFSTOPPED(status)) { 
                    #ifdef DEBUG
                        printf("Received signal that child process (%d) stopped. \n", childpid);
                    #endif
                    //if (WSTOPSIG(status) == 22) {
                    pipe->status = STOPPED;
                    esh_sys_tty_save(&pipe->saved_tty_state);
                    printf("\n[%d]+ ", pipe->jid);
                    print_one_pipeline(pipe);
                    give_terminal_to(getpgrp(), terminal);
                    //}
                }
                if (WIFCONTINUED(status)) {
                    #ifdef DEBUG
                        printf("Received signal that child process (%d) continued. \n", childpid);
                    #endif
                }
                if( WIFSIGNALED(status)) {
                    #ifdef DEBUG
                        printf("Received signal that child process (%d) received signal [%d] \n", 
                            childpid, WTERMSIG(status));
                    #endif
                    if(WTERMSIG(status) == 22){
                        #ifdef DEBUG
                            printf("Received signal that process (%d) received signal 22. \n", childpid);
                            printf("Removing command from the pipeline\n");
                            printf("Pipeline status: %d\n", pipe->status);
                        #endif
                        pipe->status = STOPPED;
                        esh_sys_tty_save(&pipe->saved_tty_state);
                        printf("\n[%d]+ ", pipe->jid);
                        print_one_pipeline(pipe);
                        give_terminal_to(getpgrp(), terminal);
                    }
                    else if(WTERMSIG(status) == 9){
                        #ifdef DEBUG
                            printf("Received signal that process (%d) received signal 9. \n", childpid);
                            printf("Removing command from the pipeline\n");
                            printf("Pipeline status: %d\n", pipe->status);
                        #endif
                        list_remove(f);
                        give_terminal_to(getpgrp(), terminal);
                    }
                    else if(WTERMSIG(status) == 2){
                        #ifdef DEBUG
                            printf("Received signal that process (%d) received signal 2 (interupt). \n", childpid);
                            printf("Removing command from the pipeline\n");
                            printf("Pipeline status: %d\n", pipe->status);
                        #endif
                        list_remove(f);
                        give_terminal_to(getpgrp(), terminal);
                    }                    
                }
                if(list_empty(&pipe->commands))
                        list_remove(e);
            } 
        }        
    }
    return;
}

/*
 * Signal Handler
 * Watch for signals to change
 * Catchs signal and send to child signal change for processing
 */
static void catch_child(int sig, siginfo_t *info, void *_ctxt)
{
    #ifdef DEBUG
        printf("CATCHING CHILD\n");
    #endif

    assert (sig == SIGCHLD);

    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WUNTRACED|WNOHANG)) > 0) {
        child_status_change(pid, status);
    }
}

/* Wait for all processes in this pipeline to complete, or for
 * the pipeline's process group to no longer be the foreground 
 * process group. 
 * You should call this function from a) where you wait for
 * jobs started without the &; and b) where you implement the
 * 'fg' command.
 * 
 * Implement child_status_change such that it records the 
 * information obtained from waitpid() for pid 'child.'
 * If a child has exited or terminated (but not stopped!)
 * it should be removed from the list of commands of its
 * pipeline data structure so that an empty list is obtained
 * if all processes that are part of a pipeline have 
 * terminated.  If you use a different approach to keep
 * track of commands, adjust the code accordingly.
 */
static void wait_for_job(struct esh_pipeline * pipeline)
{
    #ifdef DEBUG
        printf("WAITING FOR JOB\n");
    #endif

    assert(esh_signal_is_blocked(SIGCHLD));

    while (pipeline->status == FOREGROUND && !list_empty(&pipeline->commands)) {
        int status;

        pid_t child = waitpid(-1, &status, WUNTRACED|WNOHANG);
        if(child > 0)
            child_status_change(child, status);
    }
}

/* Return the jid the most recent job from the back of the jobs list 
 * Return 0 if the jobs list is empty
 */
static int recent_job(){
    if(!list_empty(&jobs_list)){
        struct list_elem * e = list_back(&jobs_list);
        struct esh_pipeline * pipeline = list_entry(e, struct esh_pipeline, elem);
        return pipeline->jid;
    }
    return 0;
}

/* Searches through the jobs list to find a job matching the parameter job_id 
 * Returns the pipeline with a matching jid
 * Returns NULL if the job could not be found
 */
static struct esh_pipeline * find_job(int job_id){
    struct list_elem * e;
    for( e = list_begin(&jobs_list); e != list_end(&jobs_list); e = list_next(e)) {
        struct esh_pipeline *tempjob = list_entry(e, struct esh_pipeline, elem);
        if (tempjob->jid == job_id) {
            return tempjob;
        }
    }
    return NULL;
}

int
main(int ac, char *av[])
{
    int opt;
    list_init(&esh_plugin_list);
    list_init(&jobs_list);
    job_id = 0;

    /* Process command-line arguments. See getopt(3) */
    while ((opt = getopt(ac, av, "hp:")) > 0) {
        switch (opt) {
        case 'h':
            usage(av[0]);
            break;

        case 'p':
            esh_plugin_load_from_directory(optarg);
            break;
        }
    }

    //if you want plugins to auto load
    //esh_plugin_load_from_directory("student-plugins/"); 
    esh_plugin_initialize(&shell);
    setpgid(0, 0);
    pid_t shell_pid = getpid();
    
    terminal = esh_sys_tty_init();
    give_terminal_to(shell_pid, terminal);

    /* Read/eval loop. */
    for (;;) {
        /* Do not output a prompt unless shell's stdin is a terminal */
        char * prompt = isatty(0) ? shell.build_prompt() : NULL;
        char * cmdline = shell.readline(prompt);
        free (prompt);

        if (cmdline == NULL)  /* User typed EOF */
            break;

        struct esh_command_line * cline = shell.parse_command_line(cmdline);
        free (cmdline);
        if (cline == NULL)                  /* Error in command line */
            continue;

        if (list_empty(&cline->pipes)) {    /* User hit enter */
            esh_command_line_free(cline);
            continue;
        }

        //get the pipeline from the default cline
        struct esh_pipeline * pipeline = list_entry(list_begin(&cline->pipes), struct esh_pipeline, elem);
        struct esh_command * commands = list_entry(list_begin(&pipeline->commands), struct esh_command, elem);

        //First command
        char * command = commands->argv[0];

        /**
          * Check if command matchs basic commands
        **/
        if (!strcmp(command, "exit")) {
            exit(EXIT_SUCCESS);
        }
        else if (!strcmp(command, "jobs")) {
            #ifdef DEBUG
                printf("jobs command\n");
            #endif

            //loop through jobs list and print all running or stopped jobs
            struct list_elem * e;
            for (e = list_begin(&jobs_list); e != list_end(&jobs_list); e = list_next(e)) {
                struct esh_pipeline * jobpipe = list_entry(e, struct esh_pipeline, elem);
                //print each job
                if(jobpipe != NULL){
                    printf("[%d] ", jobpipe->jid);
                    print_one_pipeline(jobpipe);
                }
            }
        }
        //since fg, bg, kill, stop all take a second argument, group together
        else if(!strcmp(command, "fg") || !strcmp(command, "bg")|| !strcmp(command, "kill") || !strcmp(command, "stop")){
            int jid;
            //Get the job id parameter if it is an argument
            //Assumes its an integer
            if(commands->argv[1] != NULL){
                jid = atoi(commands->argv[1]);
            }
            //If not get the most recently suspended or background job.
            else{

                jid = recent_job();
            }

            //If there is a job to process
            if(jid > 0){
                struct esh_pipeline * job;
                job = find_job(jid);

                if(!strcmp(command, "fg")){
                    #ifdef DEBUG
                        printf("foreground command\n");
                    #endif
                    esh_signal_block(SIGCHLD);

                    job->status = FOREGROUND;
                    //give the terminal to the foreground process
                    give_terminal_to(job->pgrp, terminal);
                    #ifdef DEBUG
                        printf("terminal was given to %d\n", job->pgrp);
                    #endif
                    //print the commands of the job that is now in the foreground
                    print_commands(job);

                    if( kill(-job->pgrp, SIGCONT) < 0){
                        esh_sys_fatal_error("fg error: kill SIGCONT ");
                    }
                    
                    wait_for_job(job);
                    //give terminal back to the shell when the command is done executing
                    give_terminal_to(shell_pid, terminal);

                    esh_signal_unblock(SIGCHLD);
                }

                if(!strcmp(command, "bg")){
                    #ifdef DEBUG
                        printf("background command\n");
                    #endif
                    //move the jobs to the background
                    job->status = BACKGROUND;

                    if (kill(-job->pgrp, SIGCONT) < 0) {
                        esh_sys_fatal_error("bg error: kill sigcont");
                    }
                }
                if(!strcmp(command, "kill")){
                    #ifdef DEBUG
                        printf("kill command\n");
                    #endif
                    if (kill(-job->pgrp, SIGKILL) < 0) {
                            esh_sys_fatal_error("KILL Error: ");
                        }   
                }
                if(!strcmp(command, "stop")){
                    #ifdef DEBUG
                        printf("stop command\n");
                    #endif
                    if (kill(-job->pgrp, SIGSTOP) < 0) {
                        esh_sys_fatal_error("STOP Error: ");
                    }
                }
            }
        }
        /* not a built in command
         * used http://www.gnu.org/software/libc/manual/html_node/Launching-Jobs.html#Launching-Jobs
         * as a model for launching jobs */
        else {
            //Mohammed said plugins do not need to be pipelined
            bool isPlugin = false;
            struct list_elem * e = list_begin(&esh_plugin_list);
            for (; e != list_end(&esh_plugin_list); e = list_next(e)) {
                struct esh_plugin *plugin = list_entry(e, struct esh_plugin, elem);
                if (plugin->process_builtin == NULL) {
                    continue;
                }
                //if the command is a plugin then process
                if(plugin->process_builtin(commands))
                    isPlugin = true;
                    continue;
            }
            //execute only if the command is not a basic or plugin command
            if(!isPlugin) {

                esh_signal_sethandler(SIGCHLD, catch_child);

                #ifdef DEBUG
                    printf("not a basic command or a plugin.\n");
                #endif

                job_id++;

                if(list_empty(&jobs_list)){
                    job_id = 1;
                }

                pipeline->jid = job_id;
                //set so group can be set after fork
                pipeline->pgrp = -1;

                //to use when forking
                pid_t childPid;

                struct list_elem * e = list_begin (&pipeline->commands); 
                for (; e != list_end (&pipeline->commands); e = list_next(e)) {
                    #ifdef DEBUG
                        printf("In the command loop...\n");
                    #endif

                    struct esh_command *cmd = list_entry(e, struct esh_command, elem);

                    //block the signal until everything is set to the desired outcome
                    esh_signal_block(SIGCHLD);

                    childPid = fork();

                    //fork was a success
                    if(childPid >= 0){
                         //child process
                        if(childPid == 0){
                            cmd->pid = getpid();
                            if(pipeline->pgrp == -1)
                                pipeline->pgrp = getpid();
                            //set pgid to current pid
                            if(setpgid(0,0) < 0) {      
                                esh_sys_fatal_error("Set process group unsuccessful (child): ");
                            }
                            #ifdef DEBUG
                                printf("\ngetPID (child): %d\n", getpid());
                                printf("getPGID(child): %d\n", getpgid(getpid()));
                                printf("pipeline->pgrp (child): %d\n", pipeline->pgrp);
                                printf("cmd->pid (child): %d\n", cmd->pid);
                                printf("command (child): %s\n", cmd->argv[0]);
                            #endif
                            if(pipeline->bg_job){
                                pipeline->status = BACKGROUND;
                                printf("background job entered");
                            }
                            //if in foreground, give terminal to command
                            else{
                                pipeline->status = FOREGROUND;
                                //give terminal to this pipeline
                                give_terminal_to(pipeline->pgrp, terminal);
                            }

                            //IO redirection
                            //If we must read from a file (<)
                            if (cmd->iored_input != NULL)
                            {
                                //open the file in read only mode
                                int fd_in = open(cmd->iored_input, O_RDONLY, 0);
                                dup2(fd_in, STDIN_FILENO);
                                close(fd_in);
                                cmd->iored_input = 0;
                            }

                            //If we must write to a file (>)
                            if (cmd->iored_output != NULL)
                            {
                                //if the user typed >> to append
                                if(cmd->append_to_output){
                                    int fd_out = open(cmd->iored_output, O_WRONLY | O_APPEND, 0666); 
                                    dup2(fd_out, STDOUT_FILENO);
                                    close(fd_out);
                                    cmd->iored_output = 0;
                                }
                                //if the file is not created, open a new file
                                //if there is an existing file truncate it
                                else{
                                    int fd_out = open(cmd->iored_output, O_CREAT | O_TRUNC | O_WRONLY, 0666); 
                                    dup2(fd_out, STDOUT_FILENO);
                                    close(fd_out);
                                    cmd->iored_output = 0;
                                }
                            }

                            //execute the command
                            if (execvp(cmd->argv[0], cmd->argv) < 0) {
                                esh_sys_fatal_error("Execvp failed: ");
                            }
                        }
                        //parent process
                        else{
                            cmd->pid = childPid;
                            if(pipeline->pgrp == -1)
                                pipeline->pgrp = childPid;
                            //set pgid to the childPid
                            if(setpgid(childPid, pipeline->pgrp)) {
                                esh_sys_fatal_error("Set process group unsuccessful (parent): ");
                            }
                            pipeline->status = FOREGROUND;
                            #ifdef DEBUG
                                printf("\ngetPID (parent): %d\n", getpid());
                                printf("getPGID(parent): %d\n", getpgid(getpid()));
                                printf("pipeline->pgrp (parent): %d\n", pipeline->pgrp);
                                printf("cmd->pid (parent): %d\n", cmd->pid);
                                printf("command (parent): %s\n", cmd->argv[0]);
                            #endif
                        }
                    }
                    //fork failed
                    else{
                        esh_sys_fatal_error("Fork was unsuccessful: ");
                    }

                    if (pipeline->bg_job) {
                        pipeline->status = BACKGROUND;
                        printf("[%d] %d\n", pipeline->jid, pipeline->pgrp);
                    } 

                    //if the pipeline is not empty after executing 
                    //add the pipeline to the jobs list
                    if(!list_empty(&cline->pipes)){
                        struct list_elem * e = list_pop_front(&cline->pipes);
                        list_push_back(&jobs_list, e);
                    }               
                }

                wait_for_job(pipeline);
                give_terminal_to(shell_pid, terminal);
                esh_signal_unblock(SIGCHLD);
            }
        }

        //esh_command_line_print(cline);
        esh_command_line_free(cline);
    }
    return 0;
}
