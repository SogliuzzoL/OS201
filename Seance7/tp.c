#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define V1 0

void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(void) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");
  struct sockaddr_in serv_addr, cli_addr;
  bzero((char *)&serv_addr, sizeof(serv_addr));
  int portno = 8888;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");
  listen(sockfd, 5);
  int clilen = sizeof(cli_addr);

  int main_pid = getpid();
  printf("Server started with PID %d\n", main_pid);

  int i = 0;

  while (1) {
    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");

#if V1
    char buffer[256];
    bzero(buffer, 256);
    int n = read(newsockfd, buffer, 255);
    if (n < 0)
      error("ERROR reading from socket");
    printf("Here is the message: %s\n", buffer);
    n = write(newsockfd, "I got your message", 18);
    if (n < 0)
      error("ERROR writing to socket");
#else
    if (fork() == 0) {
      close(sockfd);
      dup2(newsockfd, STDIN_FILENO);
      dup2(newsockfd, STDOUT_FILENO);
      close(newsockfd);
      if (i % 2)
        execlp("./toupper", "./toupper", NULL);
      else
        system("echo -e \"test\" | nc localhost 8888");
      i++;
    } else {
      close(newsockfd);
    }
#endif
  }
  return 0;
}