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

#define CHAR_COMMENT                    '#'

char placeHolderC[100][1000];
char placeHolderS[MAX_DEVICES + 4][150];
char *deviceName[MAX_DEVICES];
int readSpeed[MAX_DEVICES];
int writeSpeed[MAX_DEVICES];
char readyQ[MAX_COMMANDS][21];
char runningQ[MAX_COMMANDS][21];
char blockedQ[MAX_COMMANDS][21];
int totalTime = 0;
int CPUtime = 0;
char commandName[MAX_COMMAND_NAME][MAX_COMMANDS*MAX_SYSCALLS_PER_PROCESS];
int waitTime[100];
char* function[100];
char* position[100];
int sleepTime[MAX_SYSCALLS_PER_PROCESS];
int amountOfB[100];
int where = 0;
int commandExecutingIndex = 0;
int done = 0;
int commandNameIndex = 0;


void read_sysconfig(char argv0[], char filename[])
{
FILE *sysconfigFile;

    //open and read the system config file, getting each line as a string
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
    //for(int i = 0; i < line; i++){
        //printf("%s", placeHolderS[i]);
    //}
    //split each line into the different types of data, putting the respective data in their respective arrays, 
    //using dataTypeNumber to keep track of what type of data is being accessed. 
    int dataTypeNumber = 0;
    for(int i = 1; i < 7; i++) {
        if(placeHolderS[i] != "#"){
            char* stringTemp;
            stringTemp = strtok(placeHolderS[i], " ");
            while(stringTemp != NULL){
                if(dataTypeNumber == 1){
                    deviceName[i-2] = stringTemp;
                    //printf("%s \n", deviceName[i-2]);
                    //printf("%i", i-2);
                }
                if(dataTypeNumber == 2){
                    readSpeed[i-2] = atoi(stringTemp);
                    //printf("%i \n", readSpeed[i-2]);
                    //printf("%i \n", i-2);
                }
                if(dataTypeNumber == 3){
                    writeSpeed[i-2] = atoi(stringTemp);
                    //printf("%i \n", writeSpeed[i-2]);
                    //printf("%i \n", i-2);
                }
                dataTypeNumber++;
                stringTemp = strtok(NULL, " ");
            }
            dataTypeNumber = 0;
        }
    }
}

void read_commands(char argv0[], char filename[])
{
    FILE *commandsFile;
    commandsFile = fopen(filename, "r");
    int line = 0;
    while(!feof(commandsFile) && !ferror(commandsFile)){
        if(fgets(placeHolderC[line], 1000, commandsFile) != NULL){
            line++;
        }
    }
    fclose(commandsFile);

    //for(int i = 0; i < line; i++){
        //printf("%s", placeHolderC[i]);
    //}
    //int waitTime[100];
    //char* function[100];
    //char* position[100];
    //int sleepTime[100];
    //int amountOfB[100];
    int sleep = 0;
    int i = 0;
    int dataTypeNumber = 0;
    //while(placeHolderC[i] != NULL){
    if(strcmp(placeHolderC[i], "#") == 13){
        i++;
        strcpy(commandName[i-1], placeHolderC[i]);
        commandNameIndex = i-1;
        while(strcmp(placeHolderC[i], "#") != 13){
            i++;
            char* stringTemp;
            stringTemp = strtok(placeHolderC[i], " ");
            while(stringTemp != NULL){
                if(dataTypeNumber == 0){
                    if(i == 2){
                        waitTime[i-2] = atoi(stringTemp);
                    }
                    else {
                        waitTime[i-2] = atoi(stringTemp) - waitTime[i-3];
                    }
                    //printf("%i \n", waitTime[commandNum-1]);
                    //printf("%i", commandNum-1);
                }
                if(dataTypeNumber == 1){
                    function[i-2] = stringTemp;
                    if(strcmp(stringTemp, "sleep") == 0){
                        sleep = 1;
                    }
                    else{sleep = 0;}
                    //printf("%s \n", function[i-2]);
                    //printf("%i \n", i-2);
                    //printf("%i \n", sleep);
                }
                if(dataTypeNumber == 2){
                    if(sleep == 1){
                        sleepTime[i-2] = atoi(stringTemp);
                        //printf("%i", atoi(stringTemp));
                        //printf("%i", sleepTime);
                    }
                    else{position[i-2] = stringTemp;}
                    //printf("%s \n", position[i-2]);
                    //printf("%i \n", i-2);
                    //printf("%i \n", sleepTime[i-2]);
                }
                if(dataTypeNumber == 3){
                    amountOfB[i-2] = atoi(stringTemp);
                    //printf("%i \n", amountOfB[i-2]);
                    //printf("%i \n", i-2);
                }
                dataTypeNumber++;
                stringTemp = strtok(NULL, " ");
            }
            dataTypeNumber = 0;
        }
        
    }  
    printf("%s", commandName[0]);
    //printf("%i", sleepTime);
}



//  ----------------------------------------------------------------------



void pushReadyFromBlocked(int commandIndex){
    printf("ReadyFromBlocked commence \n");
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
    where = 1;
    totalTime += TIME_CORE_STATE_TRANSITIONS;
    printf("ReadyFromBlocked end \n");
}

void pushReadyFromRunning(int commandIndex){
    printf("ReadyFromRunning commence \n");
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
    totalTime += TIME_CORE_STATE_TRANSITIONS;
    printf("ReadyFromRunning end \n");
}

void pushBlocked(int commandIndex){
    printf("pushBlocked commence \n");
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
    //printf("%i", fromSleep);
    if(sleepTime[commandIndex] != 0){
        totalTime += sleepTime[commandIndex];
        commandExecutingIndex++;
    }
    totalTime += TIME_CORE_STATE_TRANSITIONS;
    where = 3;
    printf("pushBlocked end \n");
}

void pushRunning(int commandIndex){
    printf("pushRunning commence\n");
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
    while(waitTime[commandIndex] != 0){
        if(waitTime[commandIndex] <= DEFAULT_TIME_QUANTUM){
            totalTime += waitTime[commandIndex];
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
            totalTime += TIME_CONTEXT_SWITCH;
        }
    }
    printf("TEST TEST: %i \n", sleepTime[0]);
    if(strcmp(function[commandIndex], "exit") == 13){
        printf("pass");
        commandExecutingIndex = -1;
        where = -1;
    }
    printf("after waits: %i\n", totalTime);
    if(sleepTime[commandIndex] != 0){
        where = 2;
    }
    printf("TEST TEST: %i \n", sleepTime[0]);
    if(strcmp(function[commandIndex], "write") == 13 ||strcmp(function[commandIndex], "read") == 13){
        printf("pop");
        int deviceIndex = 0;
        printf("%i", deviceIndex);
        for(int i = 0; deviceName[i] != NULL; i++){
            printf("%i", strcmp(deviceName[i], position[commandIndex]));
            if(strcmp(deviceName[i], position[commandIndex]) == 0){
                deviceIndex = i;
                printf("%i", deviceIndex);
            }
        }
        printf("%i", strcmp(function[commandIndex], "write"));
        if(strcmp(function[commandIndex], "write") == 0){
            int time = amountOfB[commandIndex] / writeSpeed[deviceIndex];
            totalTime += time;
            totalTime += TIME_ACQUIRE_BUS;
            where = 4;
            commandExecutingIndex++;
            totalTime += TIME_CORE_STATE_TRANSITIONS;
        }
        if(strcmp(function[commandIndex], "read") == 0){
            int time = amountOfB[commandIndex] / readSpeed[deviceIndex];
            totalTime += time;
            totalTime += TIME_ACQUIRE_BUS;
            where = 4;
            commandExecutingIndex++;
            totalTime += TIME_CORE_STATE_TRANSITIONS;
        }
    }
    printf("pushRunning end\n");
}

int pushReadyFromNew(int commandIndex){
    printf("ReadyFromNew commence\n");
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(readyQ[i], "\0") == 0){
            strcpy(readyQ[i], commandName[commandNameIndex]);
            break;
        }
    }
    printf("%i", commandIndex);
    printf("readyFromNew end\n");
    return 1; 
}

int execute_commands()
{
    where = pushReadyFromNew(commandExecutingIndex);
    while(commandExecutingIndex != -1){
        printf("where at start: %i \n", where);
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
        printf("where at end: %i \n", where);
        //if(done == 1 && strcmp(function[commandExecutingIndex], "exit") == 0){
            //printf("%i", commandExecutingIndex);
            //done = 0;
            //exit(EXIT_SUCCESS);
        //}
        //if(done == 1 && strcmp(function[commandExecutingIndex], "exit") == 1){
            //commandExecutingIndex ++;
            //printf("%i", commandExecutingIndex);
            //done = 0;
            //printf("%i", done);
        //}
    }
    //printf("%i", totalTime);
    //get total time
    //calculate cpu percentage
}

//  ----------------------------------------------------------------------

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
    printf("measurements  %i  %i\n", totalTime, 0);

    exit(EXIT_SUCCESS);
}

//  vim: ts=8 sw=4
