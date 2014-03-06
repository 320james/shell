Student Information
-------------------
mtedesco, Marcus Tedesco
mrchang, Michael Chang


How to execute the shell
------------------------
In the esh/ directory
Run make 
Run ./esh
To run with plugins run: ./esh -p student-plugins or ./esh -p plugins

To test plugins:
	stdriver.py -p plugins plugins.tst
	stdriver.py -p student-plugins studentplugins.tst

Important Notes
---------------
All basic tests pass without a problem. Advanced works without issues. We did
not implement piping. Our system does not have any critical notes.


Description of Base Functionality
---------------------------------
jobs, fg, bg, kill, stop, \ˆC, \ˆZ 

jobs: displays the a list of current jobs both running and stopped
-We loop through our list of jobs and print the jid of each pipeline.
 Then we send the pipeline to print_one_pipeline to print the proper
 formatting of Running/Stopped followed by the literal command

fg: brings a background job to the foreground
-If a job id is sent as an argument capture that id otherwise we get 
 the most recent job from the background. Then we set the status to 
 foreground, give the terminal to the job, print the literal command,
 wait for the job to finish and then give the terminal back to the shell

bg: continues a job that is stopped in the foreground and sends it to 
	the background
-If a job id is sent as an argument capture that id otherwise we get 
 the most recent job. Sets the status to background and then sends a 
 signal to continue running

kill: kills a job
-Sends a kill signal to the job. Catch the signal and remove the job 
 from the jobs list. Give the terminal back to shell

stop: stops a job
-Sends a stop signal to the job. Catch the signal and send the job to
 the background. Give the terminal back to shell

^C: interupts the job in the foreground
-Catch the signal in the similar way that we catch a kill signal. Acts
 in the same manner

^Z: stops the job in the foreground
-Catch the signal in the similar way that we catch a stop signal. Acts
 in the same manner


Description of Extended Functionality
-------------------------------------
<describe your IMPLEMENTATION of the following functionality:
I/O, Pipes, Exclusive Access >

I/O: directs the input or output of the command using <, >, or >>
-We use the iored_input and iored_output attributes to know if the command
 requires redirection. If there is something to input we open the file and
 direct the contents to standard in. If there is something to output we 
 either open a new file or open an existing file and then either truncate 
 the contents or append the contents depending on if > or >> was used

Exclusive access: gives exclusive access of the terminal to the command
-Because of the way we implemented basic functionally exclusive access
 works natively. This works because commands like vim and nano request 
 control of the terminal and since they are in the foreground we wait
 for the process to finish before continuing 

Pipes: we did not implement piping


List of Plugins Implemented
---------------------------
(Written by Your Team)

Plugin name: 1337
Description: Inputs any number of arguments from the command line
			 and converts the characters to "leet speak"

Plugin name: oddeven
Description: Inputs an integer and prints if the number is even or odd

(Written by Others)

Plugin: doge
Group: group403

Plugin: chdir
Group: group422

Plugin: converter
Group: group422

Plugin: summation
Group: group422

Plugin: reverseString 
Group: group422

Plugin: binHex
Group: group422

Plugin: dec2hex
Group: group456

Plugin: dec2bin
Group: group456

Plugin: reverseEcho
Group: group432

Plugin: toCelsius
Group: group432

Plugin: war
Group: group415

Plugin: zodiac
Group: group415

Plugin: harrypotter
Group: group415

Plugin: starwars
Group: group407

Plugin: piglatin
Group: group451

Plugin: yolo
Group: group451

Plugin: yomama
Group: group413

Plugin: compliment
Group: group413

Plugin: seeya
Group: group413