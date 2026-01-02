#include "shell.h"

#define MAX_JOBS 64

Job jobs[MAX_JOBS];
int job_count = 0;

void init_jobs(void) {
  for (int i = 0; i < MAX_JOBS; i++) {
    jobs[i].pid = 0;
    jobs[i].id = 0;
    jobs[i].status = JOB_DONE;
    jobs[i].command = NULL;
  }
}

int get_next_job_id(void) {
  int max_id = 0;
  for (int i = 0; i < MAX_JOBS; i++) {
    if (jobs[i].pid != 0 && jobs[i].id > max_id) {
      max_id = jobs[i].id;
    }
  }
  return max_id + 1;
}

void add_job(pid_t pid, JobStatus status, const char *cmd) {
  int i;
  for (i = 0; i < MAX_JOBS; i++) {
    if (jobs[i].pid == 0) {
      jobs[i].pid = pid;
      jobs[i].status = status;
      jobs[i].command = strdup(cmd);
      jobs[i].id = get_next_job_id();
      printf("[%d] %d\n", jobs[i].id, pid);
      return;
    }
  }
  fprintf(stderr, "glsh: detailed job list is full!\n");
}

void delete_job(pid_t pid) {
  for (int i = 0; i < MAX_JOBS; i++) {
    if (jobs[i].pid == pid) {
      jobs[i].pid = 0;
      jobs[i].id = 0;
      jobs[i].status = JOB_DONE;
      if (jobs[i].command) {
        free(jobs[i].command);
        jobs[i].command = NULL;
      }
      return;
    }
  }
}

Job *find_job(pid_t pid) {
  for (int i = 0; i < MAX_JOBS; i++) {
    if (jobs[i].pid == pid) {
      return &jobs[i];
    }
  }
  return NULL;
}

Job *find_job_by_id(int id) {
  for (int i = 0; i < MAX_JOBS; i++) {
    if (jobs[i].pid != 0 && jobs[i].id == id) {
      return &jobs[i];
    }
  }
  return NULL;
}

void print_jobs(void) {
  for (int i = 0; i < MAX_JOBS; i++) {
    if (jobs[i].pid != 0) {
      const char *status_str;
      switch (jobs[i].status) {
      case JOB_RUNNING:
        status_str = "Running";
        break;
      case JOB_STOPPED:
        status_str = "Stopped";
        break;
      case JOB_DONE:
        status_str = "Done";
        break;
      default:
        status_str = "Unknown";
        break;
      }
      printf("[%d]  %s\t\t%s\n", jobs[i].id, status_str, jobs[i].command);
    }
  }
}
