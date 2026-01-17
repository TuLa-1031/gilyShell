#include "shell.h"

static Job jobs[GLSH_MAX_JOBS];

void glsh_jobs_init(void) {
  for (int i = 0; i < GLSH_MAX_JOBS; i++) {
    jobs[i].pid = 0;
    jobs[i].id = 0;
    jobs[i].status = JOB_DONE;
    jobs[i].command = NULL;
  }
}

int glsh_job_next_id(void) {
  int max_id = 0;
  for (int i = 0; i < GLSH_MAX_JOBS; i++) {
    if (jobs[i].pid != 0 && jobs[i].id > max_id) {
      max_id = jobs[i].id;
    }
  }
  return max_id + 1;
}

void glsh_job_add(pid_t pid, JobStatus status, const char *cmd) {
  for (int i = 0; i < GLSH_MAX_JOBS; i++) {
    if (jobs[i].pid == 0) {
      jobs[i].pid = pid;
      jobs[i].status = status;
      jobs[i].command = strdup(cmd);
      jobs[i].id = glsh_job_next_id();
      printf("[%d] %d\n", jobs[i].id, pid);
      return;
    }
  }
  fprintf(stderr, "glsh: job list is full\n");
}

void glsh_job_delete(pid_t pid) {
  for (int i = 0; i < GLSH_MAX_JOBS; i++) {
    if (jobs[i].pid == pid) {
      jobs[i].pid = 0;
      jobs[i].id = 0;
      jobs[i].status = JOB_DONE;
      free(jobs[i].command);
      jobs[i].command = NULL;
      return;
    }
  }
}

Job *glsh_job_find(pid_t pid) {
  for (int i = 0; i < GLSH_MAX_JOBS; i++) {
    if (jobs[i].pid == pid) {
      return &jobs[i];
    }
  }
  return NULL;
}

Job *glsh_job_find_by_id(int id) {
  for (int i = 0; i < GLSH_MAX_JOBS; i++) {
    if (jobs[i].pid != 0 && jobs[i].id == id) {
      return &jobs[i];
    }
  }
  return NULL;
}

void glsh_jobs_print(void) {
  for (int i = 0; i < GLSH_MAX_JOBS; i++) {
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
