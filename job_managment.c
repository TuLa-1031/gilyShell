#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define STATUS_RUNNING 1
#define STATUS_STOPPED 2
#define STATUS_DONE 3

typedef struct _Job {
    int id;
    pid_t pid;
    int status;
    char *command;
    struct _Job *next;
}   Job;

Job *head_job = NULL;

void add_job(pid_t pid, int status, char *command) {
    Job *new_job = (Job*)malloc(sizeof(Job));

    new_job->pid = pid;
    new_job->status = status;
    new_job->command = strdup(command);
    new_job->next = NULL;

    if (head_job == NULL) {
        new_job->id = 1;
        head_job = new_job;
    }   else {
        Job *current_job = head_job;
        while (current_job->next != NULL) {
            current_job = current_job->next;
        }
        new_job->id = current_job->id + 1;
        current_job->next = new_job;
    }
}

void delete_job(pid_t pid) {
    Job *current_job = head_job;
    Job *prev = NULL;

    while(current_job->next!=NULL) {
        if (current_job->id == pid) {
            if (prev==NULL) {
                head_job = current_job->next;
            }   else {
                prev->next = current_job->next;
            }
            free(current_job->command);
            free(current_job);
            return;
        }
        prev= current_job;
        current_job = current_job->next;
    }
}

Job* find_job_process_by_id(int id) {
    Job *current_job = head_job;
    while (current_job!=NULL) {
        if (current_job->id==id) {
            return current_job;
        }
    }
    return NULL;
}