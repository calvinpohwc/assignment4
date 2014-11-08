/*******************************
tcp_client.c: the source file of the client in tcp transmission
********************************/

#include "headsock.h"

void str_cli(FILE *fp, int sockfd);        //used for socket transmission             

int main(int argc, char **argv) // argc = argument counts, argv[0] = Application name, argv[1] = 1st param
{
	int sockfd, ret;  // sockfd => socket description from socket(), ret = return msg from connect()
	struct sockaddr_in ser_addr;  // from netinet/in.h
	
	/*  The struct contains the following
	struct sockaddr_in {
        short   sin_family; // Address domain for the socket 
        					// (Param: AF_UNIX for common file system)
        					// (Param: AF_INET for Internet domain)
        					// (Other Params : AF_INET6, AF_FILE, AF_LOCAL .....)

        u_short sin_port;  	// (Param : 16 bit Port number in network port order) 
        					// (Requires htons function to convert uint to network short format[Store most significant byte 1st])

        struct  in_addr sin_addr; // The struct contains unsigned long s_addr
        						  // (Param : 32 bits IP address in network port order [4 bits = 1 digit => 9 digit])
       
        char    sin_zero[8]; // 8 bytes of not used (Not used => Set to null or memset to 0)
	};
	*/

	char ** pptr;
	struct hostent *sh;

	/* The struct contains the following
	struct hostent
	{
  		char  *h_name; // canonical name of the host 
  		char  **h_aliases;  // list of alternate host names 
  		int   h_addrtype;  // host address type (AF_INET, AF_INET6, AF_UNIX ...)
  		int   h_length;    // length in byte
  		char  **h_addr_list // Null terminated list [Used mostly for array of in_addr structs] (char* is universal pointer before void* therefore...)
		#define h_addr  h_addr_list[0] // used for backward compatability
	};
	*/

	struct in_addr **addrs; // List of IP address..

	if (argc != 2) {
		printf("parameters not match");
	}

	sh = gethostbyname(argv[1]);	//get host's information from the input argument
									// Return NULL(Failed) / struct hostent(success)

	if (sh == NULL) {
		printf("error when gethostby name");
		exit(0);
	}

	printf("canonical name: %s\n", sh->h_name);	// print name....
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)	
		printf("the aliases name is: %s\n", *pptr); 	// print all aliases name

	switch(sh->h_addrtype)			// print address type
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
        
	addrs = (struct in_addr **)sh->h_addr_list;                       //get the server(receiver)'s ip address

	sockfd = socket(AF_INET, SOCK_STREAM, 0);   // create the socket
	//(Params: int family, int type, int protocol) 
	// family => [AF_INET....] 
	// type => [SOCK_STREAM, SOCK_DGRAM, SOCK_SEQPACKET, SOCK_RAW] 
	// protocol => [IPPROTO_TCP, IPPROTO_UDP, IPPROTO_SCTP, 0 (System Default)]

	if (sockfd <0)
	{
		printf("error in socket");
		exit(1);
	}

	ser_addr.sin_family = AF_INET;          // use AF_INET                      
	ser_addr.sin_port = htons(MYTCP_PORT);	// set the port number (0 will choose random port)
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));	 // Copy address  to .s_addr from *addrs (INADDR_ANY will choose any address)
	bzero(&(ser_addr.sin_zero), 8);	// set all 0 to be safe

	ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));         //connect the socket with the server(receiver)
	// 	Return 0 if connection is successful, -1 if error
	//	(Params: int sockfd, struct sockaddr *serv_addr, int addrlen) 
	//	sockfd: is a socket descriptor returned by the socket function.
	//	serv_addr is a pointer to struct sockaddr that contains destination IP address and port.
	//	addrlen set it to sizeof(struct sockaddr).



	if (ret != 0) {
		printf ("connection failed\n"); 
		close(sockfd); 
		exit(1);
	}
	
	str_cli(stdin, sockfd);                       //perform the transmission

	close(sockfd); // close socket...
	exit(0);
}

void str_cli(FILE *fp, int sockfd)
{
	char sends[MAXSIZE];

	printf("Please input a string (less than 50 character):\n");
	if (fgets(sends, MAXSIZE, fp) == NULL) {
		printf("error input\n");
	}
	send(sockfd, sends, strlen(sends), 0);		//send the string to the server(receiver)
	// 	return numbers of bytes sent out, or -1 on error
	//	(Params: int sockfd, const void *msg, int len, int flags)  

	printf("send out!!\n");
}

