// #include <stdlib.h>
// #include <stdio.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include <sys/time.h>
// #include <string.h>
// #include <stdbool.h>

// #define MAXLINE 4096 /*max text line length*/
// #define SERV_PORT 3000 /*port*/
// #define MAX_ROW_LENGTH 40

// int
// main(int argc, char **argv) 
// {
//     // Opening the csv file
//     FILE* reqs;
//     char req[MAX_ROW_LENGTH];
//     char *cell_val;
    
//     reqs = fopen(argv[1],"r");
//     fgets(req, MAX_ROW_LENGTH, reqs); // To ignore the header
//     if (reqs == NULL) {
//         perror("Error opening file");
//         exit(5);
//     }

//     int sockfd;
//     struct sockaddr_in servaddr;
//     char sendline[MAXLINE], recvline[MAXLINE];

//     char Server_IP[10] = "127.0.0.1";
//     //Create a socket for the client
//     //If sockfd<0 there was an error in the creation of the socket
//     if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//         perror("Problem in creating the socket");
//         exit(6);
//     }

//     //Creation of the socket
//     memset(&servaddr, 0, sizeof(servaddr));
//     servaddr.sin_family = AF_INET;
//     servaddr.sin_addr.s_addr= inet_addr(Server_IP);
//     servaddr.sin_port =  htons(SERV_PORT); //convert to big-endian order
//     struct timeval start, end;
//     double elapsed;
//     //Connection of the client to the socket 
//     if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
//     perror("Problem in connecting to the server");
//     exit(3);
//     }
//     while (fgets(req, MAX_ROW_LENGTH, reqs) != NULL) {
//         strcpy(sendline, req);
//         send(sockfd, sendline, strlen(sendline), 0);

//         if (recv(sockfd, recvline, MAXLINE - 1, 0) == 0){
//             // error: server terminated prematurely
//             perror("The server terminated prematurely"); 
//             exit(4);
//         }
        
//         recvline[MAXLINE - 1] = '\0';
//         printf("%s", "String received from the server: ");
//         fputs(recvline, stdout);
//         fputs("\n",stdout);
//         printf("%s", req);
        
//         memset(recvline, 0, sizeof(recvline));  // Clear recvline for the next response
//     }
//     fclose(reqs);
//     close(sockfd);
//     exit(0);
//     return 0;
// }

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdbool.h>

#define MAXLINE 4096 /* max text line length */
#define SERV_PORT 3000 /* port */
#define MAX_ROW_LENGTH 40

int main(int argc, char **argv) {
    FILE* reqs;
    FILE* output;
    char req[MAX_ROW_LENGTH];
    char sendline[MAXLINE], recvline[MAXLINE];

    reqs = fopen(argv[1], "r");
    if (reqs == NULL) {
        perror("Error opening request file");
        exit(5);
    }
    output = fopen(argv[2], "w+");
    if (reqs == NULL) {
        perror("Error opening output file file");
        exit(5);
    }
    int sockfd;
    struct sockaddr_in servaddr;

    char Server_IP[10] = "127.0.0.1";
    fgets(req, MAX_ROW_LENGTH, reqs); //remove headers.
    size_t len = strlen(req);
    if (len > 0 && req[len - 1] == '\n') {
        req[len - 1] = '\0';
    }
    strcat(req, ",Status\n");

    // Write the modified line to the output file
    fprintf(output, "%s", req);
    // req[-1] = ",";
    // // strcat(req,",");
    // strcat(req,"Status\n");
    // fprintf(output,req);
    while (feof(reqs) != true) {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Problem in creating the socket");
            exit(6);
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(Server_IP);
        servaddr.sin_port = htons(SERV_PORT);
        if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            perror("Problem in connecting to the server");
            exit(3);
        }
        fgets(req, MAX_ROW_LENGTH, reqs);
        printf("Row: %s", req);
        strcpy(sendline, req);
        send(sockfd, sendline, strlen(sendline), 0);
        // sleep(1);
        recv(sockfd, recvline, MAXLINE, 0);
        size_t len2 = strlen(recvline);
        if (len2 > 2) {
            recvline[len2 - 1] = '\0';
        }
        printf("\nString received from the server: %s\n", recvline);



        if (len2 > 2) {
            size_t len = strlen(req);
            if (len > 0 && req[len - 1] == '\n') {
                req[len - 1] = '\0';
            }
            strcat(req, ",");
            strcat(req, "0");
            strcat(req, ",");
            strcat(req, recvline);
            strcat(req, "\n");
            fprintf(output, "%s", req);
        } 
        else {
            size_t len = strlen(req);
            if (len > 0 && req[len - 1] == '\n') {
                req[len - 1] = '\0';
            }
            strcat(req, ",");
            strcat(req, recvline);
            strcat(req, "\n");

            fprintf(output, "%s", req);
        }

        // memset(recvline, 0, sizeof(recvline));  
        close(sockfd);
    }
    fclose(reqs);
    fclose(output);
    exit(0);
    return 0;
}
