// RUN: %clangxx_tsan -O1 %s -o %t && not %t 2>&1 | FileCheck %s
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int Global;
pthread_mutex_t mtx1;
pthread_spinlock_t mtx2;
pthread_rwlock_t mtx3;

void *Thread1(void *x) {
  sleep(1);
  pthread_mutex_lock(&mtx1);
  Global++;
  pthread_mutex_unlock(&mtx1);
  return NULL;
}

void *Thread2(void *x) {
  pthread_mutex_lock(&mtx1);
  pthread_mutex_unlock(&mtx1);
  pthread_spin_lock(&mtx2);
  pthread_rwlock_rdlock(&mtx3);
  Global--;
  pthread_spin_unlock(&mtx2);
  pthread_rwlock_unlock(&mtx3);
  return NULL;
}

int main() {
  // CHECK: WARNING: ThreadSanitizer: data race
  // CHECK:   Write of size 4 at {{.*}} by thread T1
  // CHECK:                          (mutexes: write [[M1:M[0-9]+]]):
  // CHECK:   Previous write of size 4 at {{.*}} by thread T2
  // CHECK:               (mutexes: write [[M2:M[0-9]+]], read [[M3:M[0-9]+]]):
  // CHECK:   Mutex [[M1]] created at:
  // CHECK:     #1 main {{.*}}/mutexset6.cc:[[@LINE+5]]
  // CHECK:   Mutex [[M2]] created at:
  // CHECK:     #1 main {{.*}}/mutexset6.cc:[[@LINE+4]]
  // CHECK:   Mutex [[M3]] created at:
  // CHECK:     #1 main {{.*}}/mutexset6.cc:[[@LINE+3]]
  pthread_mutex_init(&mtx1, 0);
  pthread_spin_init(&mtx2, 0);
  pthread_rwlock_init(&mtx3, 0);
  pthread_t t[2];
  pthread_create(&t[0], NULL, Thread1, NULL);
  pthread_create(&t[1], NULL, Thread2, NULL);
  pthread_join(t[0], NULL);
  pthread_join(t[1], NULL);
  pthread_mutex_destroy(&mtx1);
  pthread_spin_destroy(&mtx2);
  pthread_rwlock_destroy(&mtx3);
}
