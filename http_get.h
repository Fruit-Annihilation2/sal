#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#define USERAGENT "HTMLGET 1.0"
#define PAGE "/"
#define PORT 80
#define HOST "coding.debuntu.org"

extern int myScore;
char *highScores;
char *get_ip(char *);
int create_tcp_socket();
void usage();
void sendData();
char *build_get_query(char *, char *);

