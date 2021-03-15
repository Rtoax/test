#include <stdio.h>
#include <pthread.h>

#include <userlock.h>

#include "utils.h"

void spinlock_test() {

    user_spinlock_t spinlock = USER_SPINLOCK_INITIALIZER;
    
    user_spinlock_lock(&spinlock);
    printf("user_spinlock_trylock failed value is %d\n", user_spinlock_trylock(&spinlock));
    user_spinlock_unlock(&spinlock);
    printf("user_spinlock_trylock success value is %d\n", user_spinlock_trylock(&spinlock));
    user_spinlock_unlock(&spinlock);

    
    user_spinlock_lock(&spinlock);
    printf("user_spinlock_is_locked locked value is %d\n", user_spinlock_is_locked(&spinlock));
    user_spinlock_unlock(&spinlock);
    printf("user_spinlock_is_locked unlocked value is %d\n", user_spinlock_is_locked(&spinlock));
    

}

void rwlock_test() {
    user_rwlock_t rwlock = USER_RWLOCK_INITIALIZER;

    user_rwlock_read_lock(&rwlock);
    printf("user_rwlock_read_trylock failed value is %d\n", user_rwlock_read_trylock(&rwlock));
    user_rwlock_read_unlock(&rwlock);
    user_rwlock_read_unlock(&rwlock);
    user_rwlock_write_lock(&rwlock);
    printf("user_rwlock_read_trylock success value is %d\n", user_rwlock_read_trylock(&rwlock));
    user_rwlock_write_unlock(&rwlock);

    
    printf("user_rwlock_write_trylock success value is %d\n", user_rwlock_write_trylock(&rwlock));
    user_rwlock_write_unlock(&rwlock);
    
    user_rwlock_read_lock(&rwlock);
    printf("user_rwlock_write_trylock failed value is %d\n", user_rwlock_write_trylock(&rwlock));
    user_rwlock_read_unlock(&rwlock);

}


int main(int argc, char *argv[]) {

    spinlock_test();
    rwlock_test();
    return 0;
}

