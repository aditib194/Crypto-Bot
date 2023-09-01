#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 8192

void decrypt(char *substitutionCipher, char *message, char *d) {
    for(int i = 0; i < strlen(message); i++) {
        d[i] = substitutionCipher[message[i] - 'a'];
    }
    d[strlen(message)] = '\0';
}

int main(int argc, char** argv) {
    if(argc != 4) {
        fprintf(stderr, "usage: %s", argv[0]);
        exit(0);
    }
    
    int s;
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

    char *hostname = argv[3];
    
    unsigned short p = htons(atoi(argv[2]));
    struct sockaddr_in serveraddr;

    struct hostent *hp;
    if ((hp = gethostbyname(hostname)) == NULL) return -2;
    bzero((char *)&serveraddr, sizeof(serveraddr));
    bcopy((char *)hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = p;

    
    printf("Echo Client is trying to connect to %s (%s:%d).\n", hostname, inet_ntoa(serveraddr.sin_addr), p);
    
    if(connect(s, (struct sockaddr *) &serveraddr, sizeof(struct sockaddr_in)) < 0) {
        printf("Fail");
        return -1;
    }
    
    char spire[MAXLINE];
    strcpy(spire, "cs230 HELLO ");
    strcat(spire, argv[1]);
    strcat(spire, "@umass.edu\n");
    
    
    send(s, spire, strlen(spire), 0);
    ssize_t n;
    while((n = recv(s, spire, MAXLINE, 0)) > 0) {
        printf("MSG Received: %s\n", spire);

        strtok(spire, " ");
        char *sf = strtok(NULL, " ");
        if(strcmp(sf, "STATUS") == 0) {
            char *s_cipher = strtok(NULL, " ");
            char *m = strtok(NULL, " ");
            m[strlen(m) - 1] = '\0';
            
            char d[strlen(m)];
            decrypt(s_cipher, m, d);
        
            char decrypted[MAXLINE];
            strcpy(decrypted, "cs230 ");
            strcat(decrypted, d);
            strcat(decrypted, "\n");
            send(s, decrypted, strlen(decrypted), 0);
        }

    }

    close(s);
    exit(0);
}