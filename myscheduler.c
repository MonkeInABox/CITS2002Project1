#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//  CITS2002 Project 1 2023
//  Student1:   23360073   Dylan Arto
//  Student2:   23390554   Jeremy Butson


//  myscheduler (v1.0)
//  Compile with:  cc -std=c11 -Wall -Werror -o myscheduler myscheduler.c


//  THESE CONSTANTS DEFINE THE MAXIMUM SIZE OF sysconfig AND command DETAILS
//  THAT YOUR PROGRAM NEEDS TO SUPPORT.  YOU'LL REQUIRE THESE //  CONSTANTS
//  WHEN DEFINING THE MAXIMUM SIZES OF ANY REQUIRED DATA STRUCTURES.

#define MAX_DEVICES                     4
#define MAX_DEVICE_NAME                 20
#define MAX_COMMANDS                    10
#define MAX_COMMAND_NAME                20
#define MAX_SYSCALLS_PER_PROCESS        40
#define MAX_RUNNING_PROCESSES           50

//  NOTE THAT DEVICE DATA-TRANSFER-RATES ARE MEASURED IN BYTES/SECOND,
//  THAT ALL TIMES ARE MEASURED IN MICROSECONDS (usecs),
//  AND THAT THE TOTAL-PROCESS-COMPLETION-TIME WILL NOT EXCEED 2000 SECONDS
//  (SO YOU CAN SAFELY USE 'STANDARD' 32-BIT ints TO STORE TIMES).

#define DEFAULT_TIME_QUANTUM            100
#define TIME_CONTEXT_SWITCH             5
#define TIME_CORE_STATE_TRANSITIONS     10
#define TIME_ACQUIRE_BUS                20


//  ----------------------------------------------------------------------
//                      GLOBAL VARIABLES
//  ----------------------------------------------------------------------

#define CHAR_COMMENT                    "#"

char placeHolderC[100][150];                    //The array of placeholder strings when reading the commands file
char placeHolderS[MAX_DEVICES + 4][150];        //The array of placeholder strings when reading the sysconfig file
char *deviceName[MAX_DEVICES];                  //The array of device names gathered from the sysconfig file
int readSpeed[MAX_DEVICES];                     //The array of devices read speeds gathered from the sysconfig file
int writeSpeed[MAX_DEVICES];                    //The array of devices write speeds gathered from the sysconfig file
char readyQ[MAX_COMMANDS][21];                  //The array of functions currently waiting in ready
char runningQ[MAX_COMMANDS][21];                //The array of functions currently waiting in running
char blockedQ[MAX_COMMANDS][21];                //The array of functions currently waiting in blocked
int totalTime = 0;                              //A counter for the total amount of time that has passed (returned at end)
int CPUTime = 0;                                //A counter for the total amount of time that has passed, but only involving the CPU (returned at end)
int CPUPercent = 0;                             //The percentage of time in CPU vs. elsewhere (total time/cpu time) times 100
char commandName[MAX_COMMAND_NAME][MAX_COMMANDS * MAX_SYSCALLS_PER_PROCESS];
                                                //Array of names of the different commands in the commands file
int waitTime[100];                              //Array of the wait times for each function in the commands file
char* function[100];                            //Array of the function names for each function in the commands file
char* position[100];                            //Array of the position in the system (i.e. hard-drive) for each function in the commands file
int sleepTime[MAX_SYSCALLS_PER_PROCESS];        //Array of the sleep time for each function in the commands file (empty if not sleeping)
int amountOfB[100];                             //Array of the amount of Bytes to be passed for each function in the commands file
int where = 0;                                  //A counter to see where the function will be going, shown in execute commands
int commandExecutingIndex = 0;                  //The index of the function that is currently being processed
int commandNameIndex = 0;                       //The index of the command name currently being processed
int dataBus = 0;                                //1 if the databus has already been gathered this function, 0 if not


//  ----------------------------------------------------
//       READ FUNCTIONS
//  ----------------------------------------------------

void read_sysconfig(char argv0[], char filename[])
{
FILE *sysconfigFile;

    //open and read the system config file, obtaining each line as a string
    sysconfigFile = fopen(filename, "r");
    if(sysconfigFile == NULL){
        exit(EXIT_FAILURE);
    }
    int line = 0;
    while(!feof(sysconfigFile) && !ferror(sysconfigFile)){
        if(fgets(placeHolderS[line], 150, sysconfigFile) != NULL){
            line++;
        }
    }
    fclose(sysconfigFile);
    //split each line into the different types of data, putting the respective data in their respective arrays, 
    //using dataTypeNumber to keep track of what type of data is being accessed. 
    int dataTypeNumber = 0;
    for(int i = 2; i < 7; i++) {
        if(strcmp(placeHolderS[i], CHAR_COMMENT) != 0){
            char* stringTemp;
            stringTemp = strtok(placeHolderS[i], " ");
            while(stringTemp != NULL){
                if(dataTypeNumber == 1){
                    deviceName[i-2] = stringTemp;
                }
                if(dataTypeNumber == 2){
                    readSpeed[i-2] = atoi(stringTemp);
                }
                if(dataTypeNumber == 3){
                    writeSpeed[i-2] = atoi(stringTemp);
                }
                dataTypeNumber++;
                stringTemp = strtok(NULL, " ");
            }
            dataTypeNumber = 0;                             //Reset dataTypeNumber back to zero for the next function
        }
    }
}

//inserts all instructions of the process that's spawned in between the parent process, returning the index
//where the parent process will continues copying information into the arrays.
int spawn_read(int currentIndex, char commName[]){
    //printf("spawn read commence \n");
    //printf("%s \n", commName);
    //printf("%i \n", strcmp(placeHolderC[19], commName));
    int sleep = 0;
    int dataTypeNumber = 0;
    int totalWait = 0;
    int placeHolderCIndex = 0;
    commandNameIndex++;
    for(int i = 0; i < MAX_SYSCALLS_PER_PROCESS*MAX_COMMANDS; i++){
        if(strcmp(placeHolderC[i], commName) == 0){
            placeHolderCIndex = i;
            strcpy(commandName[commandNameIndex], placeHolderC[i]);
            printf("broke \n");
            break;
        }
    }
    int i = currentIndex;
    while(strcmp(placeHolderC[placeHolderCIndex], "#") != 13){
            i++;
            //printf("%s\n", placeHolderC[placeHolderCIndex]);
            placeHolderCIndex++;
            //printf("%s\n", placeHolderC[placeHolderCIndex]);
            char* stringTemp;
            stringTemp = strtok(placeHolderC[placeHolderCIndex], " ");
            while(stringTemp != NULL){
                if(dataTypeNumber == 0){
                    //printf("%s\n", stringTemp);
                    waitTime[i] = atoi(stringTemp) - totalWait;
                    totalWait = atoi(stringTemp);
                }
                if(dataTypeNumber == 1){
                    function[i] = stringTemp;
                    if(strcmp(stringTemp, "sleep") == 0){
                        sleep = 1;
                    }
                    else{sleep = 0;}
                }
                if(dataTypeNumber == 2){
                    if(sleep == 1){
                        sleepTime[i] = atoi(stringTemp);
                    }
                    else{position[i] = stringTemp;}
                }
                if(dataTypeNumber == 3){
                    amountOfB[i] = atoi(stringTemp);
                }
                dataTypeNumber++;
                stringTemp = strtok(NULL, " ");
            }
            dataTypeNumber = 0;                               //Reset back to zero for the next function
        }
        return i-1;
    }

void read_commands(char argv0[], char filename[])
{
    //open and read the commands file, obtaining each line as a string
    FILE *commandsFile;
    commandsFile = fopen(filename, "r");
    int line = 0;
    while(!feof(commandsFile) && !ferror(commandsFile)){
        if(fgets(placeHolderC[line], 1000, commandsFile) != NULL){
            line++;
        }
    }
    fclose(commandsFile);

    //split each line into the different types of data, putting the respective data in their respective arrays, 
    //using dataTypeNumber to keep track of what type of data is being accessed. 
    int spawnConfirm = 0;
    int sleep = 0;
    int i = -1;
    int placeHolderCIndex = 0;
    int dataTypeNumber = 0;
    int totalWait = 0;
    if(strcmp(placeHolderC[placeHolderCIndex], "#") == 13){
        placeHolderCIndex++;
        strcpy(commandName[commandNameIndex], placeHolderC[placeHolderCIndex]);
        while(strcmp(placeHolderC[placeHolderCIndex], "#") != 13){
            i++;
            placeHolderCIndex++;
            char* stringTemp;
            stringTemp = strtok(placeHolderC[placeHolderCIndex], " ");
            //printf("%s\n", placeHolderC[i]);
            while(stringTemp != NULL){
                if(dataTypeNumber == 0){
                    waitTime[i] = atoi(stringTemp) - totalWait;
                    totalWait = atoi(stringTemp);
                }
                if(dataTypeNumber == 1){
                    function[i] = stringTemp;
                    if(strcmp(stringTemp, "sleep") == 0){
                        sleep = 1;
                    }
                    else{
                        sleep = 0;
                    }
                    if(strcmp(stringTemp, "spawn") == 0){
                        spawnConfirm = 1;
                    }
                    else{
                        spawnConfirm = 0;
                    }
                }
                if(dataTypeNumber == 2){
                    if(sleep == 1){
                        sleepTime[i] = atoi(stringTemp);
                    } else if(spawnConfirm == 1){
                        i = spawn_read(i,stringTemp);
                    }
                    else{
                        position[i] = stringTemp;
                    }
                }
                if(dataTypeNumber == 3){
                    amountOfB[i] = atoi(stringTemp);
                }
                dataTypeNumber++;
                stringTemp = strtok(NULL, " ");
            }
            dataTypeNumber = 0;                                 //Reset back to zero for the next function
        }
        
    }
    printf("exec");
    for(int j = 0; j < 7; j++){
        printf("%i", waitTime[j]);
    }
    for(int j = 0; j < 7; j++){
        printf("%s", function[j]);
    }
}


//  ----------------------------------------------------
//      FUNCTIONS TO BE USED BY EXECUTE
//  ----------------------------------------------------


//moved here after blocked
void pushReadyFromBlocked(int commandIndex){
    printf("readyFromBlocked begin");
    //add to blocked queue
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(blockedQ[i], commandName[commandNameIndex]) == 0){
            while(strcmp(blockedQ[i], "\0") != 0){
                strcpy(blockedQ[i], blockedQ[i+1]);
                i++;
            }
            break;
        }
    }
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(readyQ[i], "\0") == 0){
            strcpy(readyQ[i], commandName[commandNameIndex]);
            break;
        }
    }
    //move back to running
    where = 1;
    totalTime += TIME_CORE_STATE_TRANSITIONS;
    CPUTime += TIME_CORE_STATE_TRANSITIONS;
    printf("readyFromBlocked end");
}

//moved here once a function has been run
void pushReadyFromRunning(int commandIndex){
    //added to running queue
    printf("readyFromRunning begin");
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(runningQ[i], commandName[commandNameIndex]) == 0){
            while(strcmp(runningQ[i], "\0") != 0){
                strcpy(runningQ[i], runningQ[i+1]);
                i++;
            }
            break;
        }
    }
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(readyQ[i], "\0") == 0){
            strcpy(readyQ[i], commandName[commandNameIndex]);
            break;
        }
    }
    //moved back to running
    totalTime += TIME_CORE_STATE_TRANSITIONS;
    CPUTime += TIME_CORE_STATE_TRANSITIONS;
    where = 1;
    printf("readyFromRunning end");
}


//when a function is blocked (or sleeping), it goes here
void pushBlocked(int commandIndex){
    //add function to blocked queue
    printf("Blocked begin");
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(runningQ[i], commandName[commandNameIndex]) == 0){
            while(strcmp(runningQ[i], "\0") != 0){
                strcpy(runningQ[i], runningQ[i+1]);
                i++;
            }
            break;
        }
    }
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(blockedQ[i], "\0") == 0){
            strcpy(blockedQ[i], commandName[commandNameIndex]);
            break;
        }
    }
    //if it is here because it is sleeping, add the sleep time
    if(sleepTime[commandIndex] != 0){
        totalTime += sleepTime[commandIndex];
        CPUTime += sleepTime[commandIndex];
        commandExecutingIndex++;
    }
    //move it to ready
    totalTime += TIME_CORE_STATE_TRANSITIONS;
    CPUTime += TIME_CORE_STATE_TRANSITIONS;
    where = 3;
    printf("blocked end");
}


//When a command is running, it is given to this function
void pushRunning(int commandIndex){
    //add command to running queue
    printf("running begin");
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(readyQ[i], commandName[commandNameIndex]) == 0){
            while(strcmp(readyQ[i], "\0") != 0){
                strcpy(readyQ[i], readyQ[i+1]);
                i++;
            }
            break;
        }
    }
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(runningQ[i], "\0") == 0){
            strcpy(runningQ[i], commandName[commandNameIndex]);
            break;
        }
    }
    //check the time quantum and divide the function accordingly
    while(waitTime[commandIndex] != 0){
        if(waitTime[commandIndex] <= DEFAULT_TIME_QUANTUM){
            totalTime += waitTime[commandIndex];
            CPUTime += waitTime[commandIndex];
            waitTime[commandIndex] = 0;
        } else{
            totalTime += DEFAULT_TIME_QUANTUM;
            waitTime[commandIndex] -= DEFAULT_TIME_QUANTUM;
            pushReadyFromRunning(commandIndex);
            for(int i = 0; i < MAX_COMMANDS; i++){
                if(strcmp(readyQ[i], commandName[commandNameIndex]) == 0){
                    while(strcmp(readyQ[i], "\0") != 0){
                        strcpy(readyQ[i], readyQ[i+1]);
                        i++;
                    }
                    break;
                }
            }
            for(int i = 0; i < MAX_COMMANDS; i++){
                if(strcmp(runningQ[i], "\0") == 0){
                    strcpy(runningQ[i], commandName[commandNameIndex]);
                    break;
                }
            }
        }
    }
    //if there is sleep time (i.e. it is a sleep function), where is 2, so it can be blocked
    if(sleepTime[commandIndex] != 0){
        where = 2;
    }
    //otherwise if it is a read or write function, check which device it is using, given by the deviceIndex
    else if(strcmp(function[commandIndex], "write") == 0 || strcmp(function[commandIndex], "read") == 0){
        int deviceIndex = 0;
        for(int i = 0; deviceName[i] != NULL; i++){
            if(strcmp(deviceName[i], position[commandIndex]) == 0){
                deviceIndex = i;
                break;
            }
        }
        //if write, use the write speed of the device to get the total time
        if(strcmp(function[commandIndex], "write") == 0){
            int time = amountOfB[commandIndex] / (writeSpeed[deviceIndex]/1000000);
            if(dataBus == 0){
                totalTime += TIME_ACQUIRE_BUS;
                CPUTime += TIME_ACQUIRE_BUS;
                dataBus = 1;
            }
            while(time != 0){
                if(time <= DEFAULT_TIME_QUANTUM){
                    totalTime += time;
                    time = 0;
                    totalTime += TIME_CONTEXT_SWITCH;
                    CPUTime += TIME_CONTEXT_SWITCH;
                } else{
                    totalTime += DEFAULT_TIME_QUANTUM;
                    time -= DEFAULT_TIME_QUANTUM;
                    pushReadyFromRunning(commandIndex);
                    for(int i = 0; i < MAX_COMMANDS; i++){
                        if(strcmp(readyQ[i], commandName[commandNameIndex]) == 0){
                            while(strcmp(readyQ[i], "\0") != 0){
                                strcpy(readyQ[i], readyQ[i+1]);
                                i++;
                            }
                            break;
                        }
                    }
                    for(int i = 0; i < MAX_COMMANDS; i++){
                        if(strcmp(runningQ[i], "\0") == 0){
                            strcpy(runningQ[i], commandName[commandNameIndex]);
                            break;
                        }
                    }
                    totalTime += TIME_CONTEXT_SWITCH;
                    CPUTime += TIME_CONTEXT_SWITCH;
                    dataBus = 0;
                }
            }
            where = 4;
            commandExecutingIndex++;
        }
        //if read, use the read speed of the device to get the total time
        else if(strcmp(function[commandIndex], "read") == 0){
            int time = amountOfB[commandIndex] / (readSpeed[deviceIndex]/1000000);
            if(dataBus == 0){
                totalTime += TIME_ACQUIRE_BUS;
                CPUTime += TIME_ACQUIRE_BUS;
                dataBus = 1;
                totalTime += TIME_CONTEXT_SWITCH;
                CPUTime += TIME_CONTEXT_SWITCH;
            }
            while(time != 0){
                if(time <= DEFAULT_TIME_QUANTUM){
                    totalTime += time;
                    time = 0;
                } else{
                    totalTime += DEFAULT_TIME_QUANTUM;
                    time -= DEFAULT_TIME_QUANTUM;
                    pushReadyFromRunning(commandIndex);
                    for(int i = 0; i < MAX_COMMANDS; i++){
                        if(strcmp(readyQ[i], commandName[commandNameIndex]) == 0){
                            while(strcmp(readyQ[i], "\0") != 0){
                                strcpy(readyQ[i], readyQ[i+1]);
                                i++;
                            }
                            break;
                        }
                    }
                    for(int i = 0; i < MAX_COMMANDS; i++){
                        if(strcmp(runningQ[i], "\0") == 0){
                            strcpy(runningQ[i], commandName[commandNameIndex]);
                            break;
                        }
                    }
                    CPUTime += TIME_CONTEXT_SWITCH;
                    totalTime += TIME_CONTEXT_SWITCH;
                    dataBus = 0;
                }
            }
            where = 4;
            commandExecutingIndex++;
        }
    }
    //otherwise if its spawn...
    else if(strcmp(commandName[commandIndex], "spawn") == 0){
        where = -1;
    }
    //otherwise it is an exit function, in which case it will end the process
    else{
        commandExecutingIndex = -1;
        where = -1;
    }
    printf("running end");
}


//function that each command goes through first when originally activated
int pushReadyFromNew(int commandIndex){
    //Place new command name in the ready queue
    printf("runningFromNew begin");
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(readyQ[i], "\0") == 0){
            strcpy(readyQ[i], commandName[commandNameIndex]);
            break;
        }
    }
    //return 1 to get to the next function
    printf("runningFromNew end");
    return 1; 
}


//  ----------------------------------------------------
//      EXECUTE FUNCTION TO CALL ALL OTHERS
//  ----------------------------------------------------

void execute_commands()
{
    //initialise new command by passing through ready:
    where = pushReadyFromNew(commandExecutingIndex);
    //checking what where has become before moving onto the respective function, allowing for
    //the function to move through the desired path. 
    while(commandExecutingIndex != -1){
        if(where == 1){
            pushRunning(commandExecutingIndex);
        }
        if(where == 2){
            pushBlocked(commandExecutingIndex);
        }
        if(where == 3){
            pushReadyFromBlocked(commandExecutingIndex);
        }
        if(where == 4){
            pushReadyFromRunning(commandExecutingIndex);
        }
    }
    //calculate CPU percentage using the CPU time and the total time
    CPUPercent = (CPUTime * 100) / totalTime;
}

//  ----------------------------------------------------
//                  MAIN
//  ----------------------------------------------------

int main(int argc, char *argv[])
{
//  ENSURE THAT WE HAVE THE CORRECT NUMBER OF COMMAND-LINE ARGUMENTS
    if(argc != 3) {
        printf("Usage: %s sysconfig-file command-file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

//  READ THE SYSTEM CONFIGURATION FILE
    read_sysconfig(argv[0], argv[1]);

//  READ THE COMMAND FILE
    read_commands(argv[0], argv[2]);

//  EXECUTE COMMANDS, STARTING AT FIRST IN command-file, UNTIL NONE REMAIN
    execute_commands();

//  PRINT THE PROGRAM'S RESULTS
    printf("measurements  %i  %i\n", totalTime, CPUPercent);

    exit(EXIT_SUCCESS);
}

//  vim: ts=8 sw=4
