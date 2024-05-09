#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

void handle_connection(int client_fd);

int main() {
  	setbuf(stdout, NULL);
  
  	int server_fd, client_addr_len;
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd ==-1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
	 	return 1;
	}
	
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
	 	printf("SO_REUSEPORT failed: %s \n", strerror(errno));
	 	return 1;
	}
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
	 								 .sin_port = htons(4221),
	 								 .sin_addr = { htonl(INADDR_ANY) },
	 								};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
	 	return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
	 	return 1;
	}

	while (1) {
		printf("Waiting for clients to connect...\n");
	  	struct sockaddr_in client_addr;
		int client_addr_len=sizeof(client_addr);

		int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);

		if (client_fd==-1) {
			printf("Failed to connect:%s\n",strerror(errno));
		}

		printf("Client connected");

		if(!fork()){
			close(server_fd);
			handle_connection(client_fd);
			close(client_fd);
			exit(0);
		}
		close(client_fd);
	}
	return 0;
}

void handle_connection(int client_fd) {
	printf("Handling connections");

	char buffer[BUFFER_SIZE];

	ssize_t size = recv(client_fd, buffer, BUFFER_SIZE, 0);
 	printf("size of data received:%zd\n",size);
	
	char* reqPath=strtok(buffer," ");
	reqPath=strtok(NULL," ");
	
	int bytesSend;

	if(strcmp(reqPath,"/")==0){
		char *res="HTTP/1.1 200 OK\r\n\r\n";
		bytesSend=send(client_fd,res,strlen(res),0);
	} else if(strncmp(reqPath,"/echo/",6)==0) {
		reqPath=strtok(reqPath,"/");
		reqPath=strtok(NULL,"");
		int contentLength=strlen(reqPath);

		char response[512];
		sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",contentLength,reqPath);
		printf("Sending response:%s\n",response);
		bytesSend=send(client_fd,response,strlen(response),0);
	} else if(strcmp(reqPath,"/user-agent")==0) {
		reqPath=strtok(NULL,"\r\n");
		reqPath=strtok(NULL,"\r\n");
		reqPath=strtok(NULL,"\r\n");

		char* body=strtok(reqPath," ");
		body=strtok(NULL," ");
		int contentLength=strlen(body);

		char response[512];
		sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",contentLength,body);
		printf("Sending response:%s\n",response);
		bytesSend=send(client_fd,response,strlen(response),0);
	} else {
		char *res="HTTP/1.1 404 Not Found\r\n\r\n";
		bytesSend=send(client_fd,res,strlen(res),0);
	}

	if(bytesSend<0){
		printf("send failed\n");
	}

}
