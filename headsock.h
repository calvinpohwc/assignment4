// headfile for TCP program
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#define NEWFILE (O_WRONLY|O_CREAT|O_TRUNC)
#define MYTCP_PORT 4950
#define MYUDP_PORT 5350
#define BUFSIZE 60000
#define DATALEN 500
#define HEADLEN 5
#define PACK_SIZE DATALEN + HEADLEN
#define MAX_TIMEOUT 100
#define TIMEOUT_SEC 0
#define TIMEOUT_USEC 500000L
#define TIMEOUT_NSEC TIMEOUT_USEC * 1000
#define TIMEOUT_ERROR TIMEOUT_NSEC / 2
#define DATA_NULL_TERMINATED_OFFSET 1
#define TRUE 1
#define FALSE 0
#define NOT_SET 10

#pragma pack(1)
struct pack_so			//data packet structure
{
uint8_t seq_num;				// the sequence number
uint32_t len;					// the packet length
char data[DATALEN];	//the packet data
};
#pragma pack(0)

#pragma pack(1)
struct ack_so
{
uint8_t seq_num;
uint32_t len;
};
#pragma pack(0)
