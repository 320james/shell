#!/usr/bin/python
#
# Block header comment
#
#
import sys, imp, atexit
sys.path.append("/home/courses/cs3214/software/pexpect-dpty/");
import pexpect, shellio, signal, time, os, re, proc_check

#Ensure the shell process is terminated
def force_shell_termination(shell_process):
	c.close(force=True)

#pulling in the regular expression and other definitions
definitions_scriptname = sys.argv[1]
def_module = imp.load_source('', definitions_scriptname)
logfile = None
if hasattr(def_module, 'logfile'):
    logfile = def_module.logfile

#spawn an instance of the shell
c = pexpect.spawn(def_module.shell, drainpty=True, logfile=logfile)
atexit.register(force_shell_termination, shell_process=c)

# ensure that shell prints expected prompt
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

#create a new file 
c.sendline("echo -n test input... > in_test.txt")

# print the contents of the in_test
c.sendline("cat in_test.txt")

# make sure the test file was created correctly
assert c.expect_exact("test input...") == 0, "Shell did not print the expected prompt"

# remove the test file
c.sendline("rm in_test.txt")

#exit
c.sendline("exit");
assert c.expect("exit\r\n") == 0, "Shell output extraneous characters"


shellio.success()
