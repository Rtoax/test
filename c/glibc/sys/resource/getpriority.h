#include <sys/time.h>
#include <sys/resource.h>

//which是下面之一
/* Priority limits.  */
#define PRIO_MIN	-20	/* Minimum priority a process can have.  */
#define PRIO_MAX	20	/* Maximum priority a process can have.  */

/* The type of the WHICH argument to `getpriority' and `setpriority',
   indicating what flavor of entity the WHO argument specifies.  */
enum __priority_which
{
  PRIO_PROCESS = 0,		/* WHO is a process ID.  */
#define PRIO_PROCESS PRIO_PROCESS
  PRIO_PGRP = 1,		/* WHO is a process group ID.  */
#define PRIO_PGRP PRIO_PGRP
  PRIO_USER = 2			/* WHO is a user ID.  */
#define PRIO_USER PRIO_USER
};


//get/set program scheduling priority
int getpriority(int which, int who);
int setpriority(int which, int who, int prio);

