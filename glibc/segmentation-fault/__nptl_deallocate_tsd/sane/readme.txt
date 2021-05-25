Bug 1065695 - Segmentation fault in __nptl_deallocate_tsd

https://bugzilla.redhat.com/show_bug.cgi?id=1065695

Keywords:	
Status:	CLOSED EOL
Alias:	None
Product:	Fedora
Component:	hplip  
Version:	20
Hardware:	Unspecified
OS:	Unspecified
Priority:	unspecified
Severity:	unspecified
Target Milestone:	---
Assignee:	Tim Waugh
QA Contact:	Fedora Extras Quality Assurance
Docs Contact:	
URL:	
Whiteboard:	
Depends On:	
Blocks:	
TreeView+	depends on / blocked	
 
Reported:	2014-02-15 22:30 UTC by Sandro Mani
Modified:	2015-06-29 15:14 UTC (History)
CC List:	3 users (show)
Fixed In Version:	
Doc Type: 	Bug Fix
Doc Text: 	
Clone Of:	
Environment:	
Last Closed:	2015-06-29 15:14:04 UTC
Type:	Bug
Dependent Products:	

Attachments	(Terms of Use)
Library to test (590.49 KB, application/x-troff-man)
2014-02-17 11:28 UTC, Jaromír Končický	no flags	Details
Patch (484 bytes, patch)
2014-05-02 18:03 UTC, Sandro Mani	no flags	Details | Diff
Add an attachment (proposed patch, testcase, etc.)


Links
System	ID	Private	Priority	Status	Summary	Last Updated
CUPS Bugs and Features	4410	0	None	None	None	Never
Launchpad	1315858	0	None	None	None	Never

DescriptionSandro Mani 2014-02-15 22:30:24 UTC
Description of problem:
Running sane_hpaio_get_devices (via sane_get_devices) in a thread with parameter local_only=false will result in a segmentation fault when joining the thread in which sane_get_devices was called. This happens only with the hplip sane driver with local_only=false.

Sample program and backtrace below.

Version-Release number of selected component (if applicable):
hplip-3.14.1-1.fc20

How reproducible:
Always

Steps to Reproduce:
1. Compile and run application below
2.
3.


Sample program:
--------------------------------------------------------
g++ -g -std=c++11 -o test test.cpp $(pkg-config --cflags --libs sane-backends)
--------------------------------------------------------
#include <cassert>
#include <iostream>
#include <thread>
#include <sane/sane.h>

void scan_thread() {
    SANE_Status status;

    status = sane_init(nullptr, nullptr);
    assert(status == SANE_STATUS_GOOD);

    const SANE_Device** device_list = nullptr;
    status = sane_get_devices(&device_list, false);
    assert(status == SANE_STATUS_GOOD);

    for(int i = 0; device_list[i] != nullptr; ++i){
        std::cout << device_list[i]->name << std::endl;
    }

    sane_exit();
}

int main() {
    std::thread t(scan_thread);
    t.join();
    return 0;
}
--------------------------------------------------------

Gdb output:
--------------------------------------------------------
Program received signal SIGSEGV, Segmentation fault.
[Switching to Thread 0x7ffff446c700 (LWP 5183)]
0x00007ffff2fce3e0 in ?? ()
(gdb) thread apply all bt

Thread 2 (Thread 0x7ffff446c700 (LWP 5183)):
#0  0x00007ffff2fce3e0 in ?? ()
#1  0x00007ffff59f0d32 in __nptl_deallocate_tsd () at pthread_create.c:157
#2  0x00007ffff59f0f46 in start_thread (arg=0x7ffff446c700) at pthread_create.c:322
#3  0x00007ffff70bfded in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:111

Thread 1 (Thread 0x7ffff7fd5840 (LWP 5179)):
#0  0x00007ffff59f2297 in pthread_join (threadid=140737291667200, thread_return=0x0) at pthread_join.c:92
#1  0x00007ffff795c077 in std::thread::join() () from /lib64/libstdc++.so.6
#2  0x0000000000400f89 in main () at test.cpp:25
--------------------------------------------------------

It looks like the program is crashing due to an invalid pointer when freeing up thread local storage:
pthread_create.c:157:
--------------------------------------------------------
/* Call the user-provided destructor.  */
__pthread_keys[idx].destr (data);
--------------------------------------------------------


Valgrind shows not memory issues.

Comment 1Jaromír Končický 2014-02-17 11:28:33 UTC
Created attachment 864062 [details]
Library to test

Please note that the component seems not not to be 'hplip' but rather 'sane-backends'.
I built a library 'libsane' from package version sane-backends-1.0.24-8.fc21. You probably have some different (older) version.
I tried to run the testing program with a library currently installed in my system (sane-backends-1.0.23-13.fc19.x86_64) and it segfaulted. However when I tried it with the new built library it did not segfault to me.
Please try running the testing program with attached library and tell me if it segfaults for you or not. Thanks.

Comment 2Sandro Mani 2014-02-17 12:20:43 UTC
Correct, it does not crash with the library you built.

Comment 3Sandro Mani 2014-02-17 12:24:45 UTC
The only difference between -7 (what is in f20) and -8 is http://pkgs.fedoraproject.org/cgit/sane-backends.git/tree/sane-backends-1.0.24-format-security.patch?id=c49ab916be4264b5fe8adaca570ac43472433d50 , I wonder how this could have fixed this without the cause of the error being memory corruption? Or otherwise valgrind failed to pick it up.

Comment 4Jaromír Končický 2014-02-17 13:06:33 UTC
I built the library also for version sane-backends-1.0.24-7.fc20 and it did not segfault (as I expected). Which version are you using?

Comment 5Sandro Mani 2014-02-17 13:09:20 UTC
On the affected machine I have
sane-backends-1.0.24-7.fc20.x86_64
libsane-hpaio-3.14.1-1.fc20.x86_64

Comment 6Sandro Mani 2014-02-17 13:29:19 UTC
Some more insight:
- sane-backends-1.0.24-7.fc20.x86_64 rebuilt against current F20 has the issue
- sane-backends-1.0.24-8.fc21.x86_64 rebuilt against current F20 has the issue

=> Broken glibc in F20?

Comment 7Jaromír Končický 2014-02-17 13:39:07 UTC
> => Broken glibc in F20?
It really seems like this. I've just made a scratch-build of sane-backends-1.0.24-7.fc20.x86_64, installed it and had the segfault.

Comment 8Sandro Mani 2014-02-17 13:46:49 UTC
I'll reassign to glibc. To summarize: compiling sane-backends against the glibc of F20 will cause the above sample program to crash. Compiling against the rawhide glibc, the program will run fine.

Comment 9Jaromír Končický 2014-02-17 14:37:35 UTC
To be correct, I built the working library (from comment 1) locally with 'fedpkg compile' on my Fedora 19 machine which I do not update often so I might have some different (older) version of glibc (2.17-20.fc19) or some other responsible package.

Comment 10Sandro Mani 2014-02-17 15:09:58 UTC
Latest F19 also seems affected. Latest glibc in F19 is glibc-2.17-20.fc19, so this is actually odd because with your setup it appeared to work :s What gcc are you using?

Comment 11Sandro Mani 2014-04-25 23:07:56 UTC
Seems fixed in rawhide now.

Comment 12Sandro Mani 2014-05-02 18:03:22 UTC
Scratch the comment about rawhide. But found the issue: in short, thread local storage is freed after the shared library containing the destructor is already dl-closed.

Specifics:
- libsane-hpaio.so.1 dlopened at
#0  __dlopen (file=file@entry=0x7ffff4d14d50 "/usr/lib64/sane/libsane-hpaio.so.1", mode=mode@entry=1)
    at dlopen.c:75
#1  0x00007ffff7bc6507 in load (be=be@entry=0x7ffff0008b80) at dll.c:499
#2  0x00007ffff7bc66e4 in init (be=be@entry=0x7ffff0008b80) at dll.c:608
#3  0x00007ffff7bc7021 in sane_dll_get_devices (device_list=0x7ffff4d15e60, local_only=0) at dll.c:1056
#4  0x00000000004011e1 in scan_thread () at test.cpp:13
[...]

- thread local storage allocated later on:
#0  __GI___pthread_key_create (key=0x7fffef9fae88 <cups_globals_key>, 
    destr=0x7fffef7af310 <cups_globals_free>) at pthread_key_create.c:28
#1  0x00007ffff59df3a0 in pthread_once () at ../nptl/sysdeps/unix/sysv/linux/x86_64/pthread_once.S:103
#2  0x00007fffef7af469 in _cupsGlobals () at globals.c:103
        cg = <optimized out>
#3  0x00007fffef7d9949 in cupsEncryption () at usersys.c:75
        cg = <optimized out>
#4  0x00007ffff42f41f3 in GetCupsPrinters (printer=<synthetic pointer>) at scan/sane/hpaio.c:224
#5  DevDiscovery (localOnly=localOnly@entry=0) at scan/sane/hpaio.c:305
#6  0x00007ffff42f45d9 in sane_hpaio_get_devices (deviceList=0x7ffff4d15e10, localOnly=0)
    at scan/sane/hpaio.c:359
#7  0x00007ffff7bc703a in sane_dll_get_devices (device_list=0x7ffff4d15e60, local_only=0) at dll.c:1059
#8  0x00000000004011e1 in scan_thread () at test.cpp:13
[...]
=> pthread_once calls cups_globals_init which registers global data with cups_globals_free as the thread local storage destructor

- libsane-hpaio.so.1 dlclosed at
#0  __dlclose (handle=0x7ffff0003c10) at dlclose.c:42
#1  0x00007ffff7bc6e7f in sane_dll_exit () at dll.c:966
#2  0x000000000040125f in scan_thread () at test.cpp:20
=> thread local storage destructor runs, attempts to call cups_globals_free, which is not mapped into memory anymore.


Question is how to fix this. The code should call cups_globals_free and unregister the TLS key when the dll is unloaded (as is done for the Windows dll, see cups/globals.c@DllMain). One possible approach (attached patch) involves declaring a "__attribute__((destructor))" decorated function which does the cleanup. The patch works, but I'm not sure this is the best way to do things.

Reassinging to CUPS.

Comment 13Sandro Mani 2014-05-02 18:03:46 UTC
Created attachment 891985 [details]
Patch

Comment 14Sandro Mani 2014-05-04 12:20:08 UTC
Cups upstream tells me that libcups is not dlopen/dlclose safe, taking back to hplip...

Comment 15Fedora End Of Life 2015-05-29 10:56:15 UTC
This message is a reminder that Fedora 20 is nearing its end of life.
Approximately 4 (four) weeks from now Fedora will stop maintaining
and issuing updates for Fedora 20. It is Fedora's policy to close all
bug reports from releases that are no longer maintained. At that time
this bug will be closed as EOL if it remains open with a Fedora  'version'
of '20'.

Package Maintainer: If you wish for this bug to remain open because you
plan to fix it in a currently maintained version, simply change the 'version' 
to a later Fedora version.

Thank you for reporting this issue and we are sorry that we were not 
able to fix it before Fedora 20 is end of life. If you would still like 
to see this bug fixed and are able to reproduce it against a later version 
of Fedora, you are encouraged  change the 'version' to a later Fedora 
version prior this bug is closed as described in the policy above.

Although we aim to fix as many bugs as possible during every release's 
lifetime, sometimes those efforts are overtaken by events. Often a 
more recent Fedora release includes newer upstream software that fixes 
bugs or makes them obsolete.

Comment 16Fedora End Of Life 2015-06-29 15:14:04 UTC
Fedora 20 changed to end-of-life (EOL) status on 2015-06-23. Fedora 20 is
no longer maintained, which means that it will not receive any further
security or bug fix updates. As a result we are closing this bug.

If you can reproduce this bug against a currently maintained version of
Fedora please feel free to reopen this bug against that version. If you
are unable to reopen this bug, please file a new report against the
current release. If you experience problems, please add a comment to this
bug.

Thank you for reporting this bug and we are sorry it could not be fixed.

Note
You need to log in before you can comment on or make changes to this bug.
     
