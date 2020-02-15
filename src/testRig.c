
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "fort.h"

char *serverLocation;
char *clientLocation;
char *serverPort;

int serverPid = -1;

#define LOG(logfile, testName, i, j) \
    sprintf(logfile, "log/%s_group%d_user%03d.log", testName, i + 1, j + 1)


char *randomNames[] = {
    "Rishabh",  "Vaibhav",   "Shekhar",      "Simran",        "Sunny",
    "Aryan",    "Anisha",    "Gokul",        "N.Priyanka",    "Priya",
    "Nikita",   "Abigail",   "Archita",      "Suhani",        "Mitali",
    "Kavya",    "Rakesh",    "Sumit",        "Angel",         "Prince",
    "Ramya",    "Krishna",   "Deepro",       "Leah",          "Sadaf",
    "Ajay",     "Ayushi",    "Crowny",       "Vidhya",        "Avi",
    "Dilmini",  "Mayur",     "Ram",          "Niharika",      "Nishi",
    "Diya",     "Anushri",   "Dhruv",        "Paaus",         "Manoj",
    "Sanjana",  "Rishita",   "Radhika",      "Tushar",        "Anamika",
    "Vikas",    "Dawn",      "Sanjay",       "Anu",           "Siddharth",
    "Raghav",   "Tanu",      "Nishita",      "Indhumathi",    "Anurag",
    "Parth",    "Katherine", "Sanchit",      "Kamalika",      "Pranav",
    "Abdul",    "Rutuja",    "Rohit",        "Niti",          "Avinash",
    "Sneha",    "Arun",      "Anusha",       "Ankit",         "Siya",
    "Akshay",   "Raj",       "Ankur",        "Rahul",         "John",
    "Neha",     "Tanya",     "Gayatri",      "Nishant",       "Sam",
    "Abhishek", "Tanvi",     "Ashish",       "Arya",          "Shrinidhi",
    "Mahesh",   "Riya",      "Mary",         "Arusha",        "Nisha",
    "Prachi",   "Dinesh",    "Shashank",     "Vaishnavi",     "Diksha",
    "Arjun",    "Anirudh",   "Pratik",       "Shaan",         "Khushi",
    "Manu",     "Debbie",    "Ishika",       "Ajeet",         "Raju",
    "Aditya",   "Vedant",    "Divya",        "Jatin",         "Kartik",
    "Mahima",   "Rashi",     "Nikhil",       "Harish",        "Krish",
    "Lavanya",  "Sarah",     "Chandralekha", "Ajith",         "Akansha",
    "Abhinav",  "Sasashy",   "Varun",        "Manisha",       "Moii Chhangte",
    "Swati",    "Aswini",    "Prashant",     "Papuii Colney", "Rhea",
    "Neeraj",   "Deepak",    "Deepa",        "Natasha",       "Suresh",
    "Sara",     "Vivek",     "Aditi",        "Vani",          "Amit",
    "Shreya",   "Yash",      "Sunil",        "Ananya",        "Arti",
    "Priyanka", "Karan",     "Lily",         "Soham",         "Ramanan",
    "Ishita",   "Kalyani",   "Isha",         "Varsha",        "Jay",
    "Tisha",    "Shyam",     "Abhi",         "Aaditya",       "Juvina",
    "Mayank",   "Seema",     "Nitin",        "Shivani",       "Manish",
    "Shivangi", "Akash",     "Vishal",       "Rohan",         "Sam",
    "Naveen",   "Neelam",    "Anjana",       "Ira",           "Rajeev",
    "Aniket",   "Aishwarya", "Anubhav",      "Ria",           "Sakshi",
    "Anjali",   "Dia",       "Anil",         "Kumar",         "Pawan",
    "Shail",    "Mohit",     "Prateek",      "Alok",          "Aastha",
    "Shivam",   "Krithika",  "Pavithra",     "Girish",        "Aashna",
    "Anish",    "Vinay",     "Atul",         "Kunal",         "Deep"};

char *randomGroupNames[] = {"Emerald", "Ruby", "Topaz", "Saphire"};

void killServer() {
    printf("Are you sure you want to close the client ? (Y/N) \n");
    char response;
    scanf("%c", &response);
    if (response == 'Y' || response == 'y') {
        kill(serverPid, SIGQUIT);
        exit(EXIT_SUCCESS);
    }
}

void handle_my(int sig) {
    switch (sig) {
        case SIGINT:
            killServer();
            break;
    }
}

int createServer() {
    int pid;
    if ((pid = fork()))
        return pid;
    else {
        if (execlp(serverLocation, serverLocation, serverPort, NULL) == -1) {
            perror("execl()");
            exit(EXIT_FAILURE);
        }
    }
    return -1;
}

int createClient(char *user, char *group, char *logfile) {
    int pid;
    if ((pid = fork()))
        return pid;
    else {
        // close(0);
        // close(1);
        if (execlp(clientLocation, clientLocation, "127.0.0.1", serverPort,
                   user, group, "TEST", logfile, NULL) == -1) {
            perror("execlp()");
            exit(EXIT_FAILURE);
        }
    }
    return -1;
}

unsigned long performTest(char *testName, int groupCount, int userCount,
                 int simultaneousUsers) {
    if ((serverPid = createServer()) < 0) {
        perror("createServer");
        exit(EXIT_FAILURE);
    }

    int connections[groupCount][userCount];
    char groupNames[groupCount][20];

    for (int i = 0; i < groupCount; i++)
        for (int j = 0; j < userCount; j++) {
            usleep(20 * 1000);
            char logfile[100];
            LOG(logfile, testName, i, j);
            connections[i][j] = createClient(randomNames[i * userCount + j],
                                             randomGroupNames[i], logfile);
            if (connections[i][j] < 0) {
                perror("createClient");
                exit(EXIT_FAILURE);
            }
        }

    usleep(1000 * 1000);
    for (int i = 0; i < groupCount; i++) {
        printf("Running Test %s with group %d: \n", testName, i + 1);
        for (int j = 0; j < simultaneousUsers; j++) {
            kill(connections[i][j], SIGUSR1);
        }
    }

    // getchar();
    usleep(1 * 1000 * 1000);
    // signal(SIGQUIT, SIG_IGN);
    kill(serverPid, SIGQUIT);
    // usleep(1000 * 1000);
    for (int i = 0; i < groupCount; i++) {
        for (int j = 0; j < userCount; j++) {
            kill(connections[i][j], SIGQUIT);
        }
    }

    unsigned long delaySum = 0; 
    int count = 0;
    for (int i = 0; i < groupCount; i++)
        for (int j = 0; j < userCount; j++) {
            char logfile[100];
            LOG(logfile, testName, i, j);
            int fd = open(logfile, O_RDONLY);
            unsigned long temp;
            while(read(fd, &temp, sizeof(temp))) {
                delaySum += temp;
                count++;
            }
            // remove(logfile);
        }
    printf("Average Delay: %lu\n", delaySum / count);
    return delaySum / count;
}

#define MAX_CLIENTS 10
unsigned long delays[MAX_CLIENTS + 1][MAX_CLIENTS + 1];

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("%s <server path> <client path> <serverPort> \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, handle_my);
    serverLocation = argv[1];
    clientLocation = argv[2];
    serverPort = argv[3];

    // performTest("Test", 1, 100, 100);

    for(int clients = 2; clients <= MAX_CLIENTS; clients++) {
        for(int parallel = 1; parallel <= clients; parallel++) {
            char testName[100];
            sprintf(testName, "Test_%d_%d", clients, parallel);
            delays[clients][parallel] = performTest(testName, 1, clients, parallel);
        }
    }

     ft_table_t *table = ft_create_table();

    ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
    
    ft_write(table, "#clients \\ #parallel");

    for(int clients = 1; clients <= MAX_CLIENTS; clients++) {
        char header[20];
        sprintf(header, "%d", clients);
        ft_write(table, header);
    }
     
    for(int clients = 2; clients <= MAX_CLIENTS; clients++) {
        ft_ln(table);
        char cell[10];
            sprintf(cell, "%d", clients);
            ft_write(table, cell);
        for(int parallel = 1; parallel <= clients; parallel++) {
            // printf("%05lu ", delays[clients][parallel]);
            char cell[10];
            sprintf(cell, "%lu", delays[clients][parallel]);
            ft_write(table, cell);
        }
        // ft_ln(table);
        // printf("\n");
    }

    printf("%s\n", ft_to_string(table));
    ft_destroy_table(table);
    
}