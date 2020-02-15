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

