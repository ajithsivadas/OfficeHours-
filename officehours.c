/*
Name: Ajiith Sivadas
ID: 1001829098

Name: Purvang Thakore
ID: 1001722459
*/
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1
#define CLASSC 2
#define CLASSD 3
#define CLASSE 4
pthread_mutex_t lock;
pthread_mutex_t lock1;
pthread_mutex_t lock2;
pthread_mutex_t lockA; // mutex for locking Class A
pthread_mutex_t lockB; // mutex for locking class B
pthread_mutex_t break1; // mutex for locking break functionality

pthread_cond_t cond;
pthread_cond_t cond1;
pthread_cond_t cond2;
pthread_cond_t condA;  // condition for Class A
pthread_cond_t condB;  // condition for class B
sem_t classA;   //Semaphore for Class A
sem_t classB;   // Semaphore for Class B

/* Addingsynchronization variables here */

/* Basic information about simulation are printed/checked at the end
 * and in assert statements during execution.
 */

static int students_in_office;   /* Total numbers of students currently in the office */
static int classa_inoffice;      /* Total numbers of students from class A currently in the office */
static int classb_inoffice;      /* Total numbers of students from class B in the office */
static int students_since_break; /* Total number of students in the class since the professor break*/

static int students_A; // Consecutive 5 Class A students
static int students_B;  // Consecutive 5 Class B students

typedef struct
{
  int arrival_time;  /* time between the arrival of this student and the previous student*/
  int question_time; /* time the student needs to spend with the professor*/
  int student_id;
  int class;
} student_info;


/* Called at beginning of simulation.
 * Initializing all synchronization
 * variables and other global variables here
 */
static int initialize(student_info *si, char *filename)
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0;
    
  students_B=0;
  students_A=0;
    
  sem_init(&classA, 0, 3);
  sem_init(&classB, 0, 3);
  /* Initialize your synchronization variables here */

  /* Read in the data file and initialize the student array */
  FILE *fp;

  if((fp=fopen(filename, "r")) == NULL)
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  int i = 0;
  while ( (fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time), &(si[i].question_time))!=EOF) &&
           i < MAX_STUDENTS )
  {
    i++;
  }
 fclose(fp);
 return i;
}

/* Code executed by professor to simulate taking a break*/
static void take_break()
{
  printf("The professor is taking a break now.\n");
  sleep(5);
  assert( students_in_office == 0 );
  students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students.  See the comments within the function for details.
 */
void *professorthread(void *junk)
{
  printf("The professor arrived and is starting his office hours\n");
  /* Loop while waiting for students to arrive. */
  while (1)
  {
   pthread_mutex_lock(&break1);

    /* Code to handle the student's request.*/
    /* all students are admitted without regard of the number
       of available seats, which class a student is in,
       and whether the professor needs a break.
    */
   if(students_since_break == 10 && students_in_office == 0)
    {
      take_break();
      pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&break1);

    pthread_mutex_lock(&lockA);

    /*If there are 5 students from class A , then
     * the next set of students would be from
     * class B
     */

    if(students_A == 5 && students_B == 0 && students_in_office == 0)
    {
     pthread_cond_signal(&condA);
    }

    pthread_mutex_unlock(&lockA);

    pthread_mutex_lock(&lockB);

    /*If there are 5 students from class B , then
     * The next set of students would be from
     * class A
     */

    if(students_B == 5 && students_A == 0 && classb_inoffice == 0)
        {
         pthread_cond_signal(&condB);
        }
    pthread_mutex_unlock(&lockB);


    
  }
  pthread_exit(NULL);
}


/* Request permission to enter the office.*/
/* Adding synchronization for the simulations variables below */

/* Code executed by a class A student to enter the office.*/
 
int condition_A()
{
    int a=1;
    if(a)
{

  pthread_mutex_lock(&lock);
  pthread_mutex_lock(&break1);
// condition for checking the break statements for 10 students
if(students_since_break == professor_LIMIT && students_in_office!=0)
{
 pthread_cond_wait(&cond, &break1);
}
pthread_mutex_unlock(&break1);
pthread_mutex_lock(&lock1);
if(classb_inoffice != 0)
{
 pthread_cond_wait(&cond1, &lock1);
}
pthread_mutex_unlock(&lock1);
pthread_mutex_lock(&lockA);
if(students_A == 5 && students_B == 0)
{
 pthread_cond_wait(&condA, &lockA);
 students_A = 0;
}
pthread_mutex_unlock(&lockA);
  students_in_office += 1;
  students_since_break += 1;
  classa_inoffice += 1;
  students_A += 1;
 pthread_mutex_unlock(&lock);
  return 1;
}

}
/* enter office */
void classa_enter()
{
 sem_wait(&classA);
 while(!condition_A())
{

}
 
}

/* Request permission to enter the office.*/
/* Adding synchronization for the simulations variables*/

/* Code executed by a class B student to enter the office.*/
int condition_B()
{int a=1;
if(a)
{

pthread_mutex_lock(&lock);
pthread_mutex_lock(&break1);
// condition for checking the break statements for 10 students
if(students_since_break == professor_LIMIT && students_in_office!=0)
{
 pthread_cond_wait(&cond, &break1);
}
pthread_mutex_unlock(&break1);
pthread_mutex_lock(&lock2);
if(classa_inoffice != 0)
{
 pthread_cond_wait(&cond2, &lock2);
}
pthread_mutex_unlock(&lock2);
pthread_mutex_lock(&lockB);
if(students_B == 5 && students_A == 0)
{
 pthread_cond_wait(&condB, &lockB);
 students_B = 0;
}
pthread_mutex_unlock(&lockB);
  students_in_office += 1;
  students_since_break += 1;
  classb_inoffice += 1;
  students_B += 1;
 pthread_mutex_unlock(&lock);
  return 1;
}

}
/* enter office */
void classb_enter()
{
  sem_wait(&classB);
  while(!condition_B())
{
}

}

static void ask_questions(int t)
{
  sleep(t);
}


/* Code executed by a class A student when leaving the office.*/
static void classa_leave()
{
  sem_post(&classA);
  students_in_office -= 1;
  classa_inoffice -= 1;
  pthread_mutex_lock(&lock2);
  if(classa_inoffice == 0)
  {
  pthread_cond_signal(&cond2);
  }
  pthread_mutex_unlock(&lock2);
}

/* Code executed by a class B student when leaving the office.*/
static void classb_leave()
{
  sem_post(&classB);
  students_in_office -= 1;
  classb_inoffice -= 1;
  pthread_mutex_lock(&lock1);
  if(classb_inoffice == 0)
  {
  pthread_cond_signal(&cond1);
  }
  pthread_mutex_unlock(&lock1);
}


/* Main code for class A student threads.*/
void* classa_student(void *si)
{
  student_info *s_info = (student_info*)si;
  classa_enter();
  printf("Student %d from class A enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classb_inoffice == 0 );

  /* code for asking questions by student of Class A */
  printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);
  printf("Student %d from class A leaves the office\n", s_info->student_id);
  /* leave office */
  classa_leave();

  printf("Student %d from class A leaves the office\n", s_info->student_id);
  printf("Students in office : %d\n",students_in_office);
  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  pthread_exit(NULL);
}

/* Main code for class B student threads.*/
void* classb_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classb_enter();

  printf("Student %d from class B enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0 );
  /* code for asking questions by student of Class B */
  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classb_leave();

  printf("Student %d from class B leaves the office\n", s_info->student_id);
  printf("Students in office : %d\n",students_in_office);
  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 * defining the main() function
 */
int main(int nargs, char **args)
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];

  if (nargs != 2)
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);
  if (num_students > MAX_STUDENTS || num_students <= 0)
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students ...\n",
    num_students);

  result = pthread_create(&professor_tid, NULL, professorthread, NULL);

  if (result)
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }

  for (i=0; i < num_students; i++)
  {

    s_info[i].student_id = i;
    sleep(s_info[i].arrival_time);

    student_type = random() % 2;

    if (s_info[i].class == CLASSA)
    {
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
    }
    else
    {
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
    }

    if (result)
    {
      printf("officehour: thread_fork failed for student %d: %s\n",
            i, strerror(result));
      exit(1);
    }
  }

  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++)
  {
    pthread_join(student_tid[i], &status);
  }
  /* tell the professor to finish. */
  pthread_cancel(professor_tid);

  printf("Office hour simulation done.\n");

  return 0;
} /*end the main() function*/
