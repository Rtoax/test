#include <syslog.h>

void shared_library_function (void)
{
    openlog ("mylibrary", LOG_PERROR, LOG_MAIL);
    syslog (LOG_INFO, "shared library has been invoked: %s, %m", __FILE__);
    syslog (LOG_ERR, "shared library has been invoked: %s", __FILE__);
    syslog (LOG_WARNING, "shared library has been invoked: %s", __FILE__);
    closelog ();
    
    openlog ("mylibrary2", LOG_PERROR, LOG_MAIL);
    syslog (LOG_INFO, "shared library has been invoked: %s", __FILE__);
    syslog (LOG_ERR, "shared library has been invoked: %s", __FILE__);
    syslog (LOG_WARNING, "shared library has been invoked: %s", __FILE__);
    closelog ();
    
    setlogmask (LOG_UPTO (LOG_NOTICE));
    openlog ("exampleprog", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog (LOG_NOTICE, "Program started by User %d", getuid ());
    syslog (LOG_INFO, "A tree falls in a forest");
    closelog ();
}

int main()
{
    shared_library_function();
    
    return 0;
}
