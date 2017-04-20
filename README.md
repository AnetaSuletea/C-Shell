# C-Shell
A simple command line interpretor in C, able to interpret a set  of internally-implemented commands from Linux terminal.
The set of commands:
 - The "cp" command with the parameters -i, -r (-R), -t, -v
 - The "tee" command with the parameter  -a
 - The "dirname" command
 
 - A command called "help", that will list all available commands with implemented options.
 - A command called "version", that will offer author information.
 - A command "exit".
 
 The command line interpretor offers the following facilities:
 - Accept user commands from command line. The interpretor will expose a specific prompt showing that it is prepared to accept a user command.
 - Execute user commands, from the list of accepted commands. If the user writes a command that was not internally implemented, a system call will be made. 
 - If an error occures the program will display it
 - The program supports pipes in the comands
 - History support: if the user presses the Up or DOwn arrow key, it shows the previous or next command in the terminal just like the real terminal does and auto-complete them in the terminal


