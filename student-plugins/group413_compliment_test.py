#!/usr/bin/python
#
# Testing 'compliment' plugin.
#
import sys, imp, atexit
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");
import pexpect, shellio, signal, time, os, re, proc_check

#Ensure the shell process is terminated
def force_shell_termination(shell_process):
	c.close(force=True)

#pulling in the regular expression and other definitions
definitions_scriptname = sys.argv[1]
plugin_dir = sys.argv[2]
def_module = imp.load_source('', definitions_scriptname)
logfile = None
if hasattr(def_module, 'logfile'):
    logfile = def_module.logfile

#spawn an instance of the shell
c = pexpect.spawn(def_module.shell + plugin_dir, drainpty=True, logfile=logfile)

atexit.register(force_shell_termination, shell_process=c)

# set timeout for all following 'expect*' calls to 4 seconds
c.timeout = 4

# run a command
c.sendline("compliment 1")

assert c.expect("You are beautiful!") == 0, "Wrong compliment!"

# run a command
c.sendline("compliment 25")

assert c.expect("compliment: usage:") == 0, "Wrong input!"

# send EOF
c.sendeof()

# send SIGINT in case the EOF doesn't quit their shell
c.sendintr()

shellio.success()
