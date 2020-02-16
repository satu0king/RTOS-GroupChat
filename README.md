# Group Chat Application

## Basic Instructions to run
1. run `make`
1. In a terminal run `bin/server 6666 DEV` to start server on port 6666 in development mode
1. In another terminal run `bin/client 127.0.0.1 6666 raju testGroup DEV` to start a client with name raju with group name testGroup
1. Similarily create more clients 


## Detailed Usage 
### 1) Server 
`bin/server <port> [mode]`
* `<port>` - server port
* `<mode>` - settings - DEV | TEST | PROD (optional)

The different modes differ in the log level and how the resources are managed. 

### 2) client 
`bin/client <server IP> <server port> <user handle> <group handle> [mode [logfile]]`
* `<server IP>` - IP address to which server is bound to
* `<server port>` - port on which server is running
* `<user handle>` - name of the user
* `<group handle>` - name of the group
* `<mode>` - settings - DEV | TEST | PROD (optional)
* `<logfile>` - location to save the logs (for TEST mode only)

The different modes differ in the log level and how the resources are managed. Multiple clients with the same name can exist. Clients with the same group name are added to the same group chat. If the group doesn't exist a new group is created. 

### 3) Test Rig
`bin/testRig <server path> <client path> <serverPort> <groupCount> <clientCount> <parallel> `

* `<server path>` - location of server binary (usually bin/server)
* `<client path>` - location of client binary (usually bin/client)
* `<server port>` - port on which server is to be run
* `<group count>` - number of groups to create
* `<client count>` - number of clients to create
* `<parallel count>` - number of clients which talk in parallel

This test rig is used to automate the tests and generate the test results shown at the end of this document. The test rig creates the server process and all the client processes and triggers them to send the messages at the same time to simulate the simultaneous users. The clients processes then log the results into log files. The test rig then measures the delays taken to deliver the packets and also detects any dropped messages.

## Design 
There are 2 main executables. The server and the client. A third executable is the testRig which will run tests for us. All programs do appropriate error handling, thread synchronization and handles exits gracefully. 

## Server 
The server supports a maximum of 20 groups and each group supports a maximum of 200 clients. 

### Client Join and Group Creation
When a client wants to join a group chat, the client sends a join request with client name and group name. The server automatically creates a group if it doesn't exist and assigns ids accordinally. Mutex locking is used when assigning group and user id to ensure thread safety. A new thread is created for each client which blocks waiting for a message from the client. It would be more efficient to use asynchronous IO in a single thread by using `select()`, however the former design was adopted due to simplicity.

### Message Send - Producer Consumer Model
When a client sends a message to server, the server enqueues the message into a circular queue. A mutex is used to ensure thread safety. A background thread (consumer) keeps processing the queue and delivers messages to the different clients of the group. The producer - consumer model has been used so as to avoid race conditions and to avoid overloading the network. The producer - consumer model has been implemented using mutexes with conditional variable and signalling. 

## Client 
The client joins a single group by specifying the group name and the client name. The client can easily be modified to be part of multiple groups. The client is fairly simple with 2 threads. One thread listens to the server waiting for a message from someone. Another thread listens to standard input for messages from the client. The client encodes the message with user id, group id and timestamp of the message.

### Performance Metrics
In order to meassure the performance of the system, a message delay metric was adopted. For this the client encodes the message with the current timestamp before sending. When the target client receives the message, it can compute the delay by calculating the difference between its current timestamp and the message timestamp. However, this has to be done with care as we cannot assume that clock is synchronized between the clients, i.e. there is no global clock. A protocol like NTP can be used to syncrhonize the clocks between clients. 
> However, unfortunately NTP didn't work for our basic usecase because NTP has an accuracy upto a few milliseconds. This can be improved by using some sophisticated methods and reduce the error to the order of microseconds. But since we are running the tests on the same host machine, the network delay is in the order of microseconds as well. So not only was NTP unnecessary (as clock is global) but it also introduces errors. Therefore the purpose of the test, NTP was not used.

## Test Rig 
The test rig runs stress tests by creating multiple processes and clients. The rig is fully automated and creates and closes the different processes between tests. The rig forces messages to be sent by the client by sending a signal to the client processes which then triggers a message. The clients in turn log the delays computed on receiving a message on to a designated log file. The test rig finally processes all the log files and computes the average delay.  

Attempts were made to fully populate a 100 x 100 matrix of #clients and #parallelism. However, due to some undiagonized issues, the tests were failing sometimes by not successfully closing servers and clients. This was very disappointing. However, Smaller automated tests and independent large tests all ran successfully. 

## Results 
The results show a clear increase in the average delay with increase in #clients and #parallelism. However, all delays are in microseconds as they are on the same host. Another observation is that the same test varied significantly in delay between runs. This might be due to external factors such as variables like context switching overhead and thread synchronization overhead or simply because of differences scheduling by the host system influencing the delay. Note in a real test across processes on different hosts, the network delay exceeds all these variables. To mitigate the issue, delays were computed by averaging delay across multiple test runs.

### Single client test

| #Clients | Average Delay |
|----------|---------------|
| 10       | 311           |
| 20       | 314           |
| 30       | 709           |
| 40       | 919           |
| 50       | 886           |
| 60       | 1306          |
| 70       | 1470          |
| 80       | 1456          |
| 90       | 1483          |
| 100      | 1619          |


### Parallel client test 
| #Clients           | #parallel | Average Delay |
|--------------------|-----------|---------------|
| 10                 | 3         | 743           |
| 10                 | 4         | 986           |
| 10                 | 5         | 1063          |
| 20                 | 3         | 1279          |
| 20                 | 4         | 1281          |
| 20                 | 5         | 1668          |
| 30                 | 3         | 2489          |
| 30                 | 4         | 1844          |
| 30                 | 5         | 2364          |
| 40                 | 3         | 2826          |
| 40                 | 4         | 2997          |
| 40                 | 5         | 3332          |
| 50                 | 3         | 4341          |
| 50                 | 4         | 1852          |
| 50                 | 5         | 7505          |
| 60                 | 3         | 6444          |
| 60                 | 4         | 5923          |
| 60                 | 5         | 7479          |
| 70                 | 3         | 5728          |
| 70                 | 4         | 7653          |
| 70                 | 5         | 5835          |
| 80                 | 3         | 10445         |
| 80                 | 4         | 11801         |
| 80                 | 5         | 3753          |
| 90                 | 3         | 5423          |
| 90                 | 4         | 14632         |
| 90                 | 5         | 20440         |
| 100                | 3         | 10167         |
| 100                | 4         | 17278         |
| 100                | 5         | 18361         |
| 10                 | 10        | 915           |
| 20                 | 10        | 1097          |
| 30                 | 10        | 3741          |
| 40                 | 10        | 2489          |
| 50                 | 10        | 8789          |
| 60                 | 10        | 11822         |
| 70                 | 10        | 15446         |
| 80                 | 10        | 17307         |
| 90                 | 10        | 19442         |
| 100                | 10        | 24304         |

### Stress Tests
| #Clients           | #parallel | Average Delay |
|--------------------|-----------|---------------|
| 20                 | 20        | 3517          |
| 30                 | 30        | 4959          |
| 40                 | 40        | 8897          |
| 50                 | 50        | 11539         |
| 60                 | 60        | 14238         |
| 70                 | 70        | 27683         |
| 80                 | 80        | 32568         |
| 90                 | 90        | 38382         |
| 100                | 100       | 50112         |
| 110                | 110       | 56957         |
| 120                | 120       | 64408         |
| 130                | 130       | 77757         |
| 140                | 140       | 80928         |
| 150                | 150       | 88842         |
| 160                | 160       | 99322         |
| 170                | 170       | 100254        |
| 180                | 180       | 143425        |
| 190                | 190       | 190950        |
| 200                | 200       | 192155        |


### Automated Results 
> Note: Library Libfort was used for pretty printing the tables 
```
┌──────────────────────┬─────┬─────┬─────┬─────┬──────┬─────┬─────┬─────┬─────┬──────┐
│ #clients \ #parallel │ 1   │ 2   │ 3   │ 4   │ 5    │ 6   │ 7   │ 8   │ 9   │ 10   │
├──────────────────────┼─────┼─────┼─────┼─────┼──────┼─────┼─────┼─────┼─────┼──────┤
│ 2                    │ 130 │ 211 │     │     │      │     │     │     │     │      │
│ 3                    │ 233 │ 240 │ 349 │     │      │     │     │     │     │      │
│ 4                    │ 240 │ 287 │ 259 │ 190 │      │     │     │     │     │      │
│ 5                    │ 257 │ 203 │ 316 │ 553 │ 255  │     │     │     │     │      │
│ 6                    │ 123 │ 399 │ 570 │ 330 │ 533  │ 560 │     │     │     │      │
│ 7                    │ 328 │ 298 │ 504 │ 632 │ 603  │ 730 │ 650 │     │     │      │
│ 8                    │ 272 │ 371 │ 309 │ 533 │ 643  │ 801 │ 889 │ 849 │     │      │
│ 9                    │ 402 │ 359 │ 355 │ 376 │ 803  │ 757 │ 823 │ 926 │ 867 │      │
│ 10                   │ 288 │ 567 │ 725 │ 673 │ 1559 │ 948 │ 791 │ 955 │ 979 │ 1022 │
└──────────────────────┴─────┴─────┴─────┴─────┴──────┴─────┴─────┴─────┴─────┴──────┘
```

```
╭──────────────────────┬─────┬──────┬──────┬──────┬──────╮
│ #clients \ #parallel │ 1   │ 2    │ 3    │ 4    │ 5    │
├──────────────────────┼─────┼──────┼──────┼──────┼──────┤
│ 2                    │ 263 │ 187  │      │      │      │
│ 3                    │ 315 │ 235  │ 291  │      │      │
│ 4                    │ 221 │ 158  │ 229  │ 5911 │      │
│ 5                    │ 118 │ 185  │ 245  │ 371  │ 346  │
│ 6                    │ 93  │ 188  │ 355  │ 436  │ 446  │
│ 7                    │ 151 │ 363  │ 191  │ 598  │ 300  │
│ 8                    │ 190 │ 362  │ 337  │ 320  │ 580  │
│ 9                    │ 207 │ 325  │ 353  │ 431  │ 434  │
│ 10                   │ 165 │ 484  │ 297  │ 412  │ 623  │
│ 11                   │ 229 │ 374  │ 355  │ 358  │ 781  │
│ 12                   │ 187 │ 339  │ 751  │ 439  │ 624  │
│ 13                   │ 188 │ 623  │ 886  │ 563  │ 847  │
│ 14                   │ 199 │ 615  │ 958  │ 486  │ 574  │
│ 15                   │ 360 │ 496  │ 1012 │ 607  │ 658  │
│ 16                   │ 319 │ 571  │ 464  │ 1096 │ 672  │
│ 17                   │ 325 │ 449  │ 918  │ 743  │ 1361 │
│ 18                   │ 377 │ 467  │ 982  │ 910  │ 719  │
│ 19                   │ 514 │ 551  │ 969  │ 1146 │ 981  │
│ 20                   │ 452 │ 677  │ 1361 │ 884  │ 1047 │
│ 21                   │ 407 │ 580  │ 561  │ 1655 │ 722  │
│ 22                   │ 346 │ 561  │ 1239 │ 1318 │ 1326 │
│ 23                   │ 553 │ 1040 │ 1500 │ 1395 │ 1482 │
│ 24                   │ 548 │ 611  │ 1702 │ 1659 │ 1683 │
│ 25                   │ 396 │ 751  │ 984  │ 986  │ 917  │
╰──────────────────────┴─────┴──────┴──────┴──────┴──────╯
```

## Conclusion 
In this assignment we have successfully developed a group chat system. We analyzed the performance of the system as the number of users and messages increases. We have learnt and used multithreading, several form of IPC including mutexes, socket communication and signals. Some important newly learnt concepts include producer-consumer model and time synchronization via NTP. 

## References and Resources used 
* [ Network Time Protocol Version 4: Protocol and Algorithms Specification (RFC5905)](https://tools.ietf.org/html/rfc5905)
* [ NTP Reference Implementation in C](https://lettier.github.io/posts/2016-04-26-lets-make-a-ntp-client-in-c.html)
* [ Libfort -  Pretty Print Library for tables](https://github.com/seleznevae/libfort)
* [Producer Consumer Pthread Implementation](https://classroom.udacity.com/courses/ud923/lessons/3155139407/concepts/34558787160923)
### Man Page References 
* [pthreads](http://man7.org/linux/man-pages/man7/pthreads.7.html)
* [pthread create](http://man7.org/linux/man-pages/man3/pthread_create.3.html)
* [pthread mutex lock](https://linux.die.net/man/3/pthread_mutex_lock)
* [pthread cond wait](https://linux.die.net/man/3/pthread_cond_wait)
* [sockets](http://man7.org/linux/man-pages/man2/socket.2.html)
* [fork](http://man7.org/linux/man-pages/man2/fork.2.html)
* [execlp](https://linux.die.net/man/3/execlp)
* [gettimeofday](http://man7.org/linux/man-pages/man2/gettimeofday.2.html)
* [signal](http://man7.org/linux/man-pages/man7/signal.7.html)
