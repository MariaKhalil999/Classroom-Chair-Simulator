#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//initialize the mutex
pthread_mutex_t taMutex;

//initialize the condition variable
pthread_cond_t chairBecomesFree;

//initialize the threads
pthread_t ta;
pthread_t *students;
void *ta_thread(void *studentID);
void *student_thread(void *studentID);

//initialize variables
int occupiedChairs = 0;
//queue is an array that will contain the student ids of all the students sitting in the hallway waiting for the ta
int queue[3];

//semaphore that represents the ta's sleeping behavior
sem_t sleeping;

int main(int argc, char **argv) {
  // assign the input parameter for the number of students that will require
  // help from the ta to the variable studentsNeedingHelp
  int studentsNeedingHelp = atoi(argv[1]);

  // initialize the condition variable chairBecomesFree
  pthread_cond_init(&chairBecomesFree, NULL);

  pthread_mutex_init(&taMutex, NULL);

  // initialize the sleeping semaphore
  sem_init(&sleeping, 0, 0);

  // create the ta_thread that will help each the students requiring help
  pthread_create(&ta, NULL, ta_thread, NULL);

  // create a student_thread for each of the students needing help from the ta
  students = (pthread_t *)malloc(sizeof(pthread_t) * studentsNeedingHelp);
  while (studentsNeedingHelp > 0) {
    pthread_create(&students[studentsNeedingHelp], NULL, student_thread,
                   (void *)(long)(studentsNeedingHelp));
    studentsNeedingHelp--;
  }
  pthread_exit(NULL);
}

void *ta_thread(void *args) {
  while (1) {
    //wait for the sleeping semaphore to be incremented (unlocked) by the sem_post statement in the student_thread so the ta can wake up
    sem_wait(&sleeping);

    //repeat while there is at least one chair in the queue that is occupied
    while (occupiedChairs != 0) {
      //lock the taMutex so no other student can access the ta
      pthread_mutex_lock(&taMutex);
      
      //assign the id of the next student in the queue to the variable nextStudent
      int nextStudent = queue[0];
      
      //print message stating that the student is receiving help from the ta
      printf("Student %d enters TA office and receives help\n", nextStudent);
      sleep(rand() % 8);

      // since the first student in the queue entered the ta's office, the other
      // students can move in the queue
      for (int i = 0; i < occupiedChairs; i++) {
        if (i == occupiedChairs - 1) {
          //assign value of 0 to the chair in the queue that was emptied by the students           moving up in the queue
          queue[occupiedChairs - 1] = 0;
        } 
        else {
          //move each student up to the next seat in the queue
          queue[i] = queue[i + 1];
        }
      }

      // decrement the number of occupied chairs by 1
      occupiedChairs--;

      // unlock the taMutex and print message stating that the student is left
      // the ta's office
      pthread_cond_signal(&chairBecomesFree);
      pthread_mutex_unlock(&taMutex);
      printf("Student %d received help and left TA office\n", nextStudent);

      // if there are no more student in the queue
      if (occupiedChairs == 0) {
        // print message stating that the ta is sleeping
        printf("TA is sleeping\n");
      }
    }
  }
}

void *student_thread(void *arg) {
  int studentID = (int)(long)arg;
  sleep(rand() % 15);

  while (1) {
    //change the state of the taMutex to locked
    pthread_mutex_lock(&taMutex);

    //if there are empty chairs in the queue
    if (occupiedChairs < 3) {
      //then the student will take a seat in the chair and wait
      queue[occupiedChairs] = studentID;
      occupiedChairs++;
      printf("Student %d arrived and sat in the queue\n", studentID);

      //increment the value of the sleeping semaphore to indicate that the ta is not sleeping
      sem_post(&sleeping);
      //change the state of the taMutex to unlocked
      pthread_mutex_unlock(&taMutex);
      break;
    } 
    else {
      printf("All chairs are taken, student %d will resume programming and "
             "seek help later\n",
             studentID);
      pthread_cond_wait(&chairBecomesFree, &taMutex);
      
      //change the state of the taMutex to unlocked
      pthread_mutex_unlock(&taMutex);
      sleep(rand() % 15);
    }
  }
}
