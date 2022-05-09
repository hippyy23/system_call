
# GENERALE DESCRIPTION

## Process Client_0
When **Client_0 process** is started (with the command `./client_0 <HOME>/myDir/`, where <HOME> is the user's home directory, i.e. the value contained in the environment variable HOME), creates a mask that allows it to receive only SIGINT and SIGUSR1 signals and waits for one of these signals.
 - When the **SIGUSR1** signal is received, the Client_0 process terminates.
 - When the **SIGINT** signal is received (Ctrl-C from the keyboard), the Client_0 process:
	 - sets its current directory to a path passed from the command line at program startup program 		startup,
	 - greets the user by printing the string “Ciao *USER*, ora inizio l’invio dei file contenuti
in *CURRDIR*”, where *USER* is the user name and *CURRDIR* is the current directory,
	- accesses all the folders in your current directory and loads into memory all the paths to files whose names begins with the string *"sendme_"* and whose size is less than 4KB
	- determines the number of these files and sends it via **FIFO1** to the server, then waits for confirmation from the server on **ShrMem**,
	- once confirmation has been received from the server, for each file generates a child process **Client_i** which performs the following operations:
		- opens the file,
		- determines the total number of characters,
		- divides the file into four parts containing the same number of characters. 
		*(if the number of characters is not divisible by 4, the last part will contain a smaller number of characters)*,
		- prepares the four messages for sending,
		- wait on a semaphore until all clients **Client_1 - Client_N** have reached this point in the execution.
		- when the semaphore allows **Client_i** to continue, it sends the first message to
**FIFO1**, the second to **FIFO2**, the third to **MsgQueue** and the fourth to **ShdMem**  *(inside the messages, **Client_i** also sends its own PID and the name of the file name with full path)*,
		- closes the file and terminates.
	- (Client_0) waits on the **MsgQueue** for a message from the server informing it that all output files have been created by the server and that the server has completed its operations.
	- Once this message has been received, **Client_0** unblocks the **SIGINT** and **SIGUSR1** signals and waits again to receive one of the two signals.
 
 ## Process Server
 **Server process**, on receiving from **FIFO1** the number of files transmitted by the client 
- stores this number
- writes a confirmation message on **ShdMem**
- starts receiving cyclically on each of the four channels
On receiving messages from the various channels, it performs the following operations:
- it stores the **PID** of the sender process, the file name with complete path and the part of the file transmitted
- once all four parts of a file have been received, it puts them together in the correct order and saves them in a text file, where each part of the file is stored in the correct order in a text file in which each of the four parts is separated from the next by a blank line (newline character) and has the header:
 "[**Part j**, of the file **FILENAME**, sent by the **PID** process via **CHANNEL**]", where **j** is a number from 1 to 4 depending on the
based on the part of the file, **FILENAME** is the name of the source file including the complete path, **PID** is the PID of the sender process, and **CHANNEL** is the communication channel.
- The file will be called with the same name (and path) of the original file but with the addition of the postfix "_out".
- when it has received and saved all the files it sends a termination message to the
**MsgQueue ** so that it can be recognised by **Client_0** as a job completion message.
- waits again on **FIFO1** for a new value
- Upon arrival of the **SIGINT** command (Ctrl-C from terminal) the Server process removes all IPCs and terminates.
