#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>


#define MAXLINE 5120 /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/
#define MAX_ROOMS 5
#define MAX_SLOTS 8

struct classroom {
  int booktime;
  int available;
  int booked;
  sem_t access;
};

int parse_req(struct classroom (*time_slots)[5], char* req, int* parsed_slot, int* parsed_type, int* parsed_room) {
  // This function will perform santiy check of the request
  int req_time;
  char* req_type;
  int room;
  char* slot;

  req_time = atoi(strtok(req, ","));
  req_type = strtok(NULL, ",");
  size_t len2 = strlen(req_type);
  if (len2 > 0 && req_type[len2 - 1] == '\n') {
    req_type[len2 - 1] = '\0';
  }
  printf("req_type: '%s'\n", req_type);
  // printf("%d",strcmp(req_type,"BOOK"));
  if (strcmp(req_type,"GET") == 0){
    return 0;
  }
 
  room = atoi(strtok(NULL, ","));
  slot = strtok(NULL, ",");
  size_t len1 = strlen(slot);

  if (len1 > 0 && slot[len1 - 1] == '\n') {
      slot[len1 - 1] = '\0';
  }

  if (room < 1 || room > 5){
    return -3;
  }
  // printf("time: %s\n",slot);
  // for (int i = 0; i < len1; i++){
  //   printf("char %c\n",slot[i]);
  // }
  int time_slot;
  if (strcmp(slot, "8:00-9:30") == 0) {
    time_slot = 0;
  } else if (strcmp(slot, "9:30-11:00") == 0) {
    time_slot = 1;
  } else if (strcmp(slot, "11:00-12:30") == 0) {
    time_slot = 2;
  } else if (strcmp(slot, "12:30-14:00") == 0) {
    time_slot = 3;
  } else if (strcmp(slot, "14:00-15:30") == 0) {
    time_slot = 4;
  } else if (strcmp(slot, "15:30-17:00") == 0) {
    time_slot = 5;
  } else if (strcmp(slot, "17:00-18:30") == 0) {
    time_slot = 6;
  } else if (strcmp(slot, "18:30-20:00") == 0) {
    time_slot = 7;
  } else {
    printf("Hi\n");
    return -3;
  }

  if (strcmp(req_type,"BOOK") == 0){
    if (time_slots[time_slot][room-1].available == 1){
      *parsed_slot = time_slot;
      *parsed_type = 1;
      *parsed_room = room;
      return 1;
    }
    else{
      return -1;
    }
  }
  else if (strcmp(req_type,"CANCEL") == 0){
    if (time_slots[time_slot][room-1].booked != 1){
      return -3;
    }
    if ((req_time - time_slots[time_slot][room-1].booktime) < 20) {
      return -2;
    }
    else{
      *parsed_slot = time_slot;
      *parsed_type = 2;
      *parsed_room = room;
    }
  }
  else{

    return -3;
  }
  return 0;
}

int handle_req(struct classroom (*time_slots)[5], int slot, int type, int room, int connfd, int n, char* buf){
  sem_wait(&time_slots[slot][room-1].access);
  if (time_slots[slot][room-1].available == 0) {
    sem_post(&time_slots[slot][room-1].access);
    return -1;
  }

  switch(type) {
    case 1:
      time_slots[slot][room-1].available = 0;
      time_slots[slot][room-1].booked = 1;
      time_slots[slot][room-1].booktime = slot;
      sem_post(&time_slots[slot][room-1].access);
      strcpy(buf,"0");
      send(connfd, buf, n, 0);
      break;
    case 2:
      time_slots[slot][room-1].available = 1;
      time_slots[slot][room-1].booked = 0;
      time_slots[slot][room-1].booktime = -1;
      sem_post(&time_slots[slot][room-1].access);
      strcpy(buf,"0");
      send(connfd, buf, n, 0);
      break;
  }
  return 0;
}

int handle_GET_req(struct classroom (*time_slots)[5], int slot, int type, int connfd, int n, char* buf) {
    buf[0] = '\0'; // Ensure buf is initially an empty string

    // Add an opening bracket to the beginning of the string
    strcat(buf, "{");

    for (int i = 0; i < MAX_SLOTS; i++) {
        for (int j = 0; j < MAX_ROOMS; j++) {
            if (time_slots[i][j].booked == 1) {
                char slot_info[100]; // Adjust the size as needed
                sprintf(slot_info, "('%d','%s')", j + 1, (i == 0) ? "8:00-9:30" : (i == 1) ? "9:30-11:00" : (i == 2) ? "11:00-12:30" : (i == 3) ? "12:30-14:00" : (i == 4) ? "14:00-15:30" : (i == 5) ? "15:30-17:00" : (i == 6) ? "17:00-18:30" : "18:30-20:00");

                // Concatenate slot_info to buf
                strcat(buf, slot_info);
                strcat(buf, ",");
            }
        }
    }

    // Remove the trailing comma if there is at least one booked slot
    size_t len = strlen(buf);
    if (len > 1 && buf[len - 1] == ',') {
        buf[len - 1] = '\0';
    }

    // Add a closing bracket to the end of the string
    strcat(buf, "}");
    strcat(buf, "\0");
    // Output the final content of buf
    puts(buf);

    // Send the concatenated information to the client
    send(connfd, buf, strlen(buf), 0);

    return 0;
}


int main (int argc, char **argv)
{
  int shmid;
  shmid = shmget(IPC_PRIVATE, MAX_SLOTS * MAX_ROOMS * sizeof(struct classroom), IPC_CREAT | 0666);
  if (shmid == -1) {
    perror("shmget");
    return 1;
  }

  /* Attach the shared memory segment. */
  struct classroom (*time_slots)[MAX_ROOMS] = shmat(shmid, NULL, 0);
  if (time_slots == (struct classroom(*)[MAX_ROOMS])-1) {
    perror("shmat");
    return 1;
  }

  // Initialize the time_slots
  for (int j = 0; j < MAX_SLOTS; j++) {
    for (int i = 0; i < MAX_ROOMS; i++) {

      time_slots[j][i].available = 1;
      time_slots[j][i].booked = -1;
      time_slots[j][i].booktime = -1;
      sem_init(&time_slots[j][i].access, 1, 1);
    }
  }

  int listenfd, connfd, n;
  pid_t childpid;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  //Create a socket for the soclet
  //If sockfd<0 there was an error in the creation of the socket
  if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
    perror("Problem in creating the socket");
    exit(2);
  }


  //preparation of the socket address
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);

  //bind the socket
  bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  //listen to the socket by creating a connection queue, then wait for clients
  listen (listenfd, LISTENQ);

  printf("%s\n","Server running...waiting for connections.");

  for ( ; ; ) {

    clilen = sizeof(cliaddr);
    //accept a connection
    connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

    printf("%s\n","Received request...");

    if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process
      char buf[MAXLINE];
      printf ("%s\n","Child created for dealing with client requests");

      //close listening socket
      close (listenfd);

      n = recv(connfd, buf, MAXLINE,0);
      puts(buf);

      if (n < 0) {
        printf("%s\n", "Read error");
      }

      else {
          int p_res, h_res;
          int p_slot;
          int p_type;
          int p_room;       
          p_res = parse_req(time_slots,buf,&p_slot,&p_type,&p_room);
          printf("p_res %d\n",p_res);
          if (p_res < 0){
            sprintf(buf,"%d",p_res);
            send(connfd, buf, n, 0);
          }
          else {
            if (p_type == 0){
              h_res = handle_GET_req(time_slots,p_slot,p_type,connfd,n,buf);
            }
            else{
              h_res = handle_req(time_slots,p_slot,p_type,p_room,connfd,n,buf);
            }
            printf("h_res %d\n",h_res);
            if (h_res < 0){
              sprintf(buf,"%d",h_res);
              send(connfd, buf, n, 0);
            }
          }
      }
      close(connfd);
      printf("Finished serving a child\n");
      exit(0);
    }
    //close socket of the server
    close(connfd);
  }
  shmdt(time_slots);
  shmctl(shmid, IPC_RMID, NULL);
  return 0;

}