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
char commandName[MAX_COMMAND_NAME];
int waitTime[100];
char* function[100];
char* position[100];
int sleepTime = 0;
int amountOfB[100];
int fromSleep = 0;

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
    int commandNum = 0;
    //while(placeHolderC[i] != NULL){
    if(strcmp(placeHolderC[i], "#") == 13){
        commandNum++;
        i++;
        strcpy(commandName, placeHolderC[i]);
        i++;
        char* stringTemp;
        stringTemp = strtok(placeHolderC[i], " ");
        while(stringTemp != NULL){
            if(dataTypeNumber == 0){
                waitTime[commandNum-1] = atoi(stringTemp);
                //printf("%i \n", waitTime[commandNum-1]);
                //printf("%i", commandNum-1);
            }
            if(dataTypeNumber == 1){
                function[commandNum-1] = stringTemp;
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
                    sleepTime = atoi(stringTemp);
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
    printf("%s", commandName);
    //printf("%i", sleepTime);
}



//  ----------------------------------------------------------------------

void pushReadyFromBlocked(char commandName[]){
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(blockedQ[i], commandName) == 0){
            while(strcmp(blockedQ[i], "\0") != 0){
                strcpy(blockedQ[i], blockedQ[i+1]);
                i++;
            }
            break;
        }
    }
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(readyQ[i], "\0") == 0){
            strcpy(readyQ[i], commandName);
            break;
        }
    }
    totalTime += 10;
}

void pushReadyFromRunning(char commandName[]){
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(runningQ[i], commandName) == 0){
            while(strcmp(runningQ[i], "\0") != 0){
                strcpy(runningQ[i], runningQ[i+1]);
                i++;
            }
            break;
        }
    }
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(readyQ[i], "\0") == 0){
            strcpy(readyQ[i], commandName);
            break;
        }
    }
    totalTime += 10;
}

void pushBlocked(char commandName[]){
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(runningQ[i], commandName) == 0){
            while(strcmp(runningQ[i], "\0") != 0){
                strcpy(runningQ[i], runningQ[i+1]);
                i++;
            }
            break;
        }
    }
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(blockedQ[i], "\0") == 0){
            strcpy(blockedQ[i], commandName);
            break;
        }
    }
    printf("%i", fromSleep);
    if(fromSleep == 1){
        printf("%i", sleepTime);
        totalTime += sleepTime;
        fromSleep = 0;
    }
    totalTime += 10;
}

void pushRunning(char commandName[]){
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(readyQ[i], commandName) == 0){
            while(strcmp(readyQ[i], "\0") != 0){
                strcpy(readyQ[i], readyQ[i+1]);
                i++;
            }
            break;
        }
    }
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(runningQ[i], "\0") == 0){
            strcpy(runningQ[i], commandName);
            break;
        }
    }
    if(sleepTime != 0){
        fromSleep = 1;
        pushBlocked(commandName);
    }
    totalTime += 10;
}



void pushReadyFromNew(char commandName[]){
    for(int i = 0; i < MAX_COMMANDS; i++){
        if(strcmp(readyQ[i], "\0") == 0){
            strcpy(readyQ[i], commandName);
            break;
        }
    }
    pushRunning(commandName);
}

int execute_commands()
{
    pushReadyFromNew(commandName);
    //get total time
    //calculate cpu percentage
    //pushRunning(commandName);
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
