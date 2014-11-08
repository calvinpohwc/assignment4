/**********************************
tcp_ser.c: the source file of the server in tcp transmission
***********************************/


#include "headsock.h"

#define BACKLOG 10

void str_ser(int sockfd);                                                        // transmitting and receiving function

int main(void)
{
	int sockfd, con_fd, ret; // sockfd => socket description from socket(), ret = return msg from connect(), con_fd => connection description from accept()
	struct sockaddr_in my_addr; 
	struct sockaddr_in their_addr;
	int sin_size; 

	pid_t pid; // process ID

	sockfd = socket(AF_INET, SOCK_STREAM, 0);          //create socket
	if (sockfd <0)
	{
		printf("error in socket!");
		exit(1);
	}
	

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYTCP_PORT);				//port number
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // to long ...32 bit, any ip address
	bzero(&(my_addr.sin_zero), 8); // set 0

	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket
	// return 0 if successful, -1 if failed..
	// Parameters:
	//sockfd: is a socket descriptor returned by the socket function.
	//my_addr is a pointer to struct sockaddr that contains local IP address and port.
	//addrlen set it to sizeof(struct sockaddr).
	if (ret <0)
	{
		printf("error in binding");
		exit(1);
	}
	
	ret = listen(sockfd, BACKLOG);       // Enable connection request for the socket, 
	// return 0 if successful, -1 if failed..
	// Parameters:
	//sockfd: is a socket descriptor returned by the socket function.
	//backlog is the number of allowed connections.

	if (ret <0) {
		printf("error in listening");
		exit(1);
	}

	printf("receiving start\n");

	while (1)
	{
		sin_size = sizeof (struct sockaddr_in);

		con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);            //accept the packet using blocking ...
		// Return non negative on success, -1 on error
		// Params:
		//	sockfd: is a socket descriptor returned by the socket function.
		//	cliaddr is a pointer to struct sockaddr that contains client IP address and port.
		//	addrlen set it to sizeof(struct sockaddr).


		if (con_fd <0)
		{
			printf("error in accept\n");
			exit(1);
		}

		if ((pid = fork())==0)                                         // create acception process, new process by duplicating existing (Successful return 0 at child, then return child PID at parent)
		{
			close(sockfd);								// child process, close server socket
			str_ser(con_fd);                                          //receive packet and response
			close(con_fd);							// close connector socket
			exit(0);
		}
		else close(con_fd);                       //close connector socket parent process
	}
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd)
{
	char recvs[MAXSIZE];
	int n = 0;

	if ((n= recv(sockfd, &recvs, MAXSIZE, 0))==-1)                                   //receive the packet
		// return -1 if error, return number of byte if successful
		//sockfd: is a socket descriptor returned by the socket function.
		//buf is the buffer to read the information into.
		//len is the maximum length of the buffer.
		//flags is set to 0.

	{
		printf("receiving error!\n");
		return;
	}

	recvs[n] = '\0';
	printf("the received string:\n%s\n", recvs);
}
