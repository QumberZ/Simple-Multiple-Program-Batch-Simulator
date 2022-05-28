//
//  simulator.c
//  Phase1
//
//  Created by Qumber Zaidi on 3/27/22.
//
#include "configops.h"
#include "metadataops.h"
#include "simulator.h"
#include "StringUtils.h"
#include "simtimer.h"

#include <pthread.h>

char *memory_alloca = NULL;
process_t *root_process_list, *process_list;
int running_process_identifier = 0;


void output(char *message, ConfigDataType *configPtr){
    char *value = (char *) malloc(sizeof(char) * 100);
    accessTimer(LAP_TIMER, value);
    
    if (configPtr -> logToCode == LOGTO_FILE_CODE) {
        FILE *fp = fopen(configPtr-> logToFileName, "a+");
        fprintf(fp, "[ %s] %s\n", value, message);
        fclose(fp);
    }
    
    if (configPtr -> logToCode == LOGTO_BOTH_CODE) {
        FILE *fp = fopen(configPtr-> logToFileName, "a+");
        fprintf(fp, "[ %s] %s\n", value, message);
        fclose(fp);
        printf("[ %s] %s\n", value, message);
    }
    if (configPtr -> logToCode == LOGTO_MONITOR_CODE) {
        printf("[ %s] %s\n", value, message);
    }
    free(value);
}

void *msleep(void* args){
    double millisecond = *((double*) args);
    char dummy[100];
    double start_time = accessTimer(LAP_TIMER, dummy);
    while (1000 * (accessTimer(LAP_TIMER, dummy) - start_time) <= millisecond);
    return NULL;
}

void timer(double millisecond){
    if (millisecond <= 1e-3) return;
    pthread_t pid;
    double *ptr = (double*) malloc(sizeof(double));
    *ptr = millisecond;
    pthread_create(&pid, NULL, msleep, (void*) ptr);
    pthread_join(pid, NULL);
    free(ptr);
}


Boolean mem_alloca_func(int dummy1, int dummy2){
    return True;
}


Boolean mem_access_func(int dummy){
    return True;
}

Boolean runProcess(process_t *current_process, ConfigDataType *configDataType){
    if (current_process->end_flag == 1) {
        output("OS: System stop", configDataType);
        return False;
    }
    
    if (current_process->start_flag == 1) {
        return True;
    }
    int i;
    
    current_process->state = PROCESS_STATE_READY;
    for (i = 0; i < current_process->exe_size; ++i) {
        switch (current_process->execution_flow[i].command) {
            case APP:
            {
                if (current_process->execution_flow[i].strArgs1 == START) {
                    timer(current_process->execution_flow[i].intArg2 *
                          configDataType->quantumCycles);
                    
                    char *value = (char*) malloc(sizeof(char) * 1000);
                    sprintf(value, "OS: Process %d set to RUNNING state from READY state.",
                            running_process_identifier);
                    output(value, configDataType);
                    free(value);
                    current_process->state = PROCESS_STATE_RUN;
                } else if(current_process-> execution_flow[i].strArgs1 == END){
                    
                    timer(current_process->execution_flow[i].intArg2 *
                          configDataType-> quantumCycles);
                    
                    char *value = (char*) malloc(sizeof(char) * 1000);
                    sprintf(value, "OS: Process %d set to EXITED state from RUNNING state.",
                            running_process_identifier);
                    output(value, configDataType);
                    free(value);
                    running_process_identifier++;
                    current_process->state = PROCESS_STATE_RUN;
                }
            
                break;
            }
            case DEVIN:{
                char *value = (char*) malloc(sizeof(char) * 1000);
                sprintf(value, "Process: %d device input %s start, time: %d ms.",
                        running_process_identifier, current_process->execution_flow[i].origin->strArg1,
                        current_process->execution_flow[i].intArg2 * configDataType-> quantumCycles * configDataType-> ioCycleRate);
                output(value, configDataType);
                
                timer(current_process->execution_flow[i].intArg2 * configDataType-> quantumCycles *
                      configDataType-> ioCycleRate);
                sprintf(value, "Process: %d device input %s end.", running_process_identifier,
                        current_process->execution_flow[i].origin->strArg1);
                output(value, configDataType);
                free(value);
                break;
            }
                
            case DEVOUT:{
                char *value = (char*) malloc(sizeof(char) * 1000);
                sprintf(value, "Process: %d device output %s start, time: %d ms.",
                        running_process_identifier, current_process->execution_flow[i].origin->strArg1,
                        current_process->execution_flow[i].intArg2 * configDataType-> quantumCycles *
                        configDataType-> ioCycleRate);
                output(value, configDataType);
                
                timer(current_process->execution_flow[i].intArg2 * configDataType-> quantumCycles *
                      configDataType-> ioCycleRate);
                sprintf(value, "Process: %d device input %s end.", running_process_identifier,
                        current_process->execution_flow[i].origin->strArg1);
                output(value, configDataType);
                free(value);
                break;
                
            }
            case CPU:{
                char *value = (char*) malloc(sizeof(char) * 1000);
                sprintf(value, "Process: %d CPU Process, time: %d ms.", running_process_identifier,
                        current_process->execution_flow[i].intArg2 * configDataType-> quantumCycles);
                output(value, configDataType);
                
                timer(current_process->execution_flow[i].intArg2 * configDataType-> quantumCycles);
                sprintf(value, "Process: %d CPU Process end.", running_process_identifier);
                output(value, configDataType);
                free(value);
                break;
            }
            case MEM:{
                if (current_process->execution_flow[i].strArgs1 == ALLOCATE) {
                    char *value = (char*) malloc(sizeof(char) * 1000);
                    sprintf(value, "Process: %d Memory allocation from %d to %d.",
                            running_process_identifier, current_process->execution_flow[i].intArg2,
                            current_process->execution_flow[i].intArg3);
                    output(value, configDataType);
                    
                    Boolean result = mem_alloca_func(current_process->execution_flow[i].intArg2,
                                                     current_process->execution_flow[i].intArg3);
                    if(result) sprintf(value, "Process: %d Memory allocation from %d to %d success.",
                                        running_process_identifier, current_process->execution_flow[i].intArg2,
                                        current_process->execution_flow[i].intArg3);
                    else sprintf(value, "Process: %d Memory allocation from %d to %d failed.",
                                 running_process_identifier, current_process->execution_flow[i].intArg2,
                                 current_process->execution_flow[i].intArg3);
                    output(value, configDataType);
                    free(value);
                    break;
                } else if (current_process->execution_flow[i].strArgs1 == ACCESS){
                    char *value = (char*) malloc(sizeof(char) * 1000);
                    sprintf(value, "Process: %d Memory access to %d.",
                            running_process_identifier, current_process->execution_flow[i].intArg2);
                    output(value, configDataType);
                    
                    Boolean result = mem_access_func(current_process->execution_flow[i].intArg2);
                    if(result) sprintf(value, "Process: %d Memory access to %d success.",
                                        running_process_identifier, current_process->execution_flow[i].intArg2);
                    else sprintf(value, "Process: %d Memory access to %d failed.",
                                 running_process_identifier, current_process->execution_flow[i].intArg2);
                    output(value, configDataType);
                    free(value);
                    break;
                }
                
            }
            case SYS: break;
        }
        
    }
    return True;
}

void initial_process(int n, ConfigDataType *configPtr){
    char value[1007];
    int i;
    for (i = 0; i < n; ++i) {
        sprintf(value, "OS: Process %d set to READY state from NEW state.", i);
        output(value, configPtr);
    }
}


void memset_usr(char* ptr, int count, char value){
    int i;
    for (i = 0; i < count; ++i) ptr[i] = value;
}

void runSim(ConfigDataType *configPtr, OpCodeType *metaDataMsterPtr){
    
    runTimer(0);
    
    char *value = (char *) malloc(sizeof(char) * 100);
    accessTimer(ZERO_TIMER, value);
    output("OS: Simulation Start", configPtr);
    accessTimer(LAP_TIMER, value);
    
    Boolean simRun = True;
    process_list = (process_t *) malloc(sizeof(process_t));
    memset_usr((char*) process_list, sizeof(process_t) / sizeof(char), 0);
    
    root_process_list = process_list;
    
    OpCodeType *ptr = metaDataMsterPtr;
    process_t *prevProcess = NULL;
    OpCodeType *app_start, *app_end;
    int range = 0;
    int process_cnt = 0;
    
    while (ptr != NULL) {
        if (compareString(ptr->command, "sys") == 0) {
            if(compareString(ptr->strArg1, "start") == 0) process_list-> start_flag = 1;
            else process_list->end_flag = 1;
            
            if (prevProcess != NULL) prevProcess->next = process_list;
            
            prevProcess = process_list;
            process_list = (process_t *) malloc(sizeof(process_t));
            memset_usr((char*) process_list, sizeof(process_t) / sizeof(char), 0);
            process_cnt++;
                
            
        } else if (compareString(ptr->command, "app") == 0){
            if (compareString(ptr->strArg1, "start") == 0) {
                range = 1;
                app_start = ptr;
            } else if (compareString(ptr->strArg1, "end") == 0){
                range++;
                app_end = ptr;
                
                process_list->execution_flow = (executable_t *) malloc(sizeof(executable_t) *
                                                                       (range + 7));
                memset_usr((char*) process_list->execution_flow, (sizeof(executable_t) * (range + 7)) / sizeof(char), 0);
                
                process_list->exe_size = range;
                int command_idx = 0;
                struct OpCodeType *app_ptr;
                
                for (app_ptr = app_start; app_ptr != app_end; app_ptr = app_ptr->nextNode) {
                    if (compareString(app_ptr->command, "app") == 0) {
                        if (compareString(app_ptr->strArg1, "start") == 0) {
                            process_list-> execution_flow[command_idx].command = APP;
                            process_list-> execution_flow[command_idx].strArgs1 = START;
                        } else if(compareString(app_ptr->strArg1, "end" ) == 0) {
                            process_list-> execution_flow[command_idx].command = APP;
                            process_list-> execution_flow[command_idx].strArgs1 = END;
                        }
                    } else if (compareString(app_ptr->command, "cpu") == 0){
                        if(compareString(app_ptr->strArg1, "process" ) == 0) {
                            process_list-> execution_flow[command_idx].command = CPU;
                            process_list-> execution_flow[command_idx].strArgs1 = PROCESS;
                    }
                    } else if (compareString(app_ptr->command, "dev") == 0){
                        if(compareString(app_ptr->inOutArg, "in") == 0) {
                            process_list-> execution_flow[command_idx].command = DEVIN;
                            
                        } else if(compareString(app_ptr->inOutArg, "out") == 0) {
                            process_list-> execution_flow[command_idx].command = DEVOUT;
            }
                        if(compareString(app_ptr->strArg1, "monitor") == 0)
                            process_list-> execution_flow[command_idx].strArgs1 = MONITOR;
                        if(compareString(app_ptr->strArg1, "sound signal") == 0)
                            process_list-> execution_flow[command_idx].strArgs1 = SOUND_SIGNAL;
                        if(compareString(app_ptr->strArg1, "ethernet") == 0)
                            process_list-> execution_flow[command_idx].strArgs1 = ETHERNET;
                        if(compareString(app_ptr->strArg1, "hard drive") == 0)
                            process_list-> execution_flow[command_idx].strArgs1 = HDD;
                        if(compareString(app_ptr->strArg1, "keyboard") == 0)
                            process_list-> execution_flow[command_idx].strArgs1 = KEYBOARD;
                        if(compareString(app_ptr->strArg1, "serial") == 0)
                            process_list-> execution_flow[command_idx].strArgs1 = SERIAL;
                        if(compareString(app_ptr->strArg1, "video signal") == 0)
                            process_list-> execution_flow[command_idx].strArgs1 = VIDEO_SIGNAL;
                        if(compareString(app_ptr->strArg1, "usb") == 0)
                            process_list-> execution_flow[command_idx].strArgs1 = USB;
                        
    } else if (compareString(app_ptr->command, "mem") == 0) {
        process_list-> execution_flow[command_idx].command = MEM;
        
     if (compareString(app_ptr->strArg1, "access") == 0)
            process_list-> execution_flow[command_idx].strArgs1 = ACCESS;
        
        else if (compareString(app_ptr->strArg1, "allocate") == 0)
            process_list-> execution_flow[command_idx].strArgs1 = ALLOCATE;
             
}

                    process_list-> execution_flow[command_idx].intArg2 = app_ptr->intArg2;
                    process_list-> execution_flow[command_idx].intArg3 = app_ptr->intArg3;
                    process_list-> execution_flow[command_idx].origin = app_ptr;
                    process_list-> state = PROCESS_STATE_NEW;
                    command_idx += 1;
                }
                process_list-> execution_flow[command_idx].command = APP;
                process_list-> execution_flow[command_idx].strArgs1 = END;
                
                if(prevProcess != NULL) prevProcess->next = process_list;
                prevProcess = process_list;
                process_list = (process_t *) malloc(sizeof(process_t));
                memset_usr((char*) process_list, sizeof(process_t) / sizeof(char), 0);
                process_cnt++;
            }
        } else {
            range++;
        }
        
        ptr = ptr->nextNode;
        
    }
    output("OS: System start", configPtr);
    initial_process(process_cnt, configPtr);
    free(process_list);
    process_list = root_process_list;
    
    while (simRun) {
        simRun = runProcess(process_list, configPtr);
        if(!simRun) break;
        process_list = process_list->next;
    }
    output("OS: Simulation End", configPtr);
    
    while (root_process_list != NULL) {
        process_t *p = root_process_list->next;
        free(root_process_list->execution_flow);
        free(root_process_list);
        root_process_list = p;
    }
    free(value);
}

