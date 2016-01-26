=========================================================
AMD FirePro SDK
=========================================================
WGL Framelock sample - 2012/06/19
---------------------------------------------------------
This sample demonstrates the capability of FirePro GPUs 
to synchronize the buffer swaps of a group of OpenGL 
windows. A swap group is created, and windows are added 
as members to the swap group. Buffer swaps to members of 
the swap group will then take place concurrently.

It shows the capability to sychronize the buffer swaps of 
different swap groups, which may reside on distributed 
systems on a network. For this purpose swap groups can be 
bound to a swap barrier.

To get this sample working correctly please make sure that
you configured Timing Server and Timing Clients in the 
Catalyst Control Center.
---------------------------------------------------------
1/ Create a Visual C++ solution using CMake
http://www.cmake.org/cmake/resources/software.html
2/ Run the sample and enjoy!
---------------------------------------------------------
