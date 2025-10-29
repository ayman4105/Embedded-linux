#!/bin/bash
ayman@ayman-Lat5:~$ mkdir myDirectory
ayman@ayman-Lat5:~$ cd myDirectory/
ayman@ayman-Lat5:~/myDirectory$ mkdir secondDirectory
ayman@ayman-Lat5:~/myDirectory$ cd secondDirectory/
ayman@ayman-Lat5:~/myDirectory/secondDirectory$ touch myNotePaper
ayman@ayman-Lat5:~/myDirectory/secondDirectory$ cp ~/myDirectory/secondDirectory/myNotePaper ~/myDirectory/
ayman@ayman-Lat5:~/myDirectory/secondDirectory$ cd ..
ayman@ayman-Lat5:~/myDirectory$ ls
myNotePaper  secondDirectory
ayman@ayman-Lat5:~/myDirectory$ mv ~/myDirectory/myNotePaper ~/myDirectory/myOldNotePaper
ayman@ayman-Lat5:~/myDirectory$ ls
myOldNotePaper  secondDirectory


