/*******************************
tcp_client.c: the source file of the client in tcp transmission 
 ********************************/

#include "headsock.h"

float str_cli(FILE *fp, int sockfd, long *len, uint8_t error_probability); //transmission function
void tv_sub(struct timeval *out, struct timeval *in); //calcu the time interval between out and in
void set_receive_timeout(int *sockfd); // set receive timeout
uint8_t get_error_probability(char *arg);

// 3rd param : simulates error of sending partial frame (damaged)

int main(int argc, char **argv) {
    uint8_t error_probability = 0;

    int sockfd, ret; // sockfd => socket description from socket(), ret = return msg from connect()
    float ti, rt;
    long len;
    struct sockaddr_in ser_addr;
    
    
    char ** pptr;
    struct hostent *sh;
    struct in_addr **addrs;
    FILE *fp;



    if (argc < 2 && argc > 3) {
        printf("parameters not match");
    }

    sh = gethostbyname(argv[1]); //get host's information
    if (sh == NULL) {
        printf("error when gethostby name");
        exit(1);
    }

    if (argc == 3) {
        error_probability = get_error_probability(argv[2]);
    }

    printf("Error Probability chosen is : %d\n", error_probability);


    printf("canonical name: %s\n", sh->h_name); //print the remote host's information
    for (pptr = sh->h_aliases; *pptr != NULL; pptr++)
        printf("the aliases name is: %s\n", *pptr);
    switch (sh->h_addrtype) {
        case AF_INET:
            printf("AF_INET\n");
            break;
        default:
            printf("unknown addrtype\n");
            break;
    }


    addrs = (struct in_addr **) sh->h_addr_list;
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the socket
    if (sockfd < 0) {
        printf("error in socket");
        exit(1);
    }
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(MYTCP_PORT);
    memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof (struct in_addr));
    bzero(&(ser_addr.sin_zero), 8);
    ret = connect(sockfd, (struct sockaddr *) &ser_addr, sizeof (struct sockaddr)); //connect the socket with the host
    if (ret != 0) {
        printf("connection failed\n");
        close(sockfd);
        exit(1);
    }

    if ((fp = fopen("myfile.txt", "r+t")) == NULL) {
        printf("File doesn't exit\n");
        exit(0);
    }


    //
    printf("sockfd now is %d", sockfd);
    set_receive_timeout(&sockfd);
    //

    ti = str_cli(fp, sockfd, &len, error_probability); //perform the transmission and receiving
    rt = (len / (float) ti); //caculate the average transmission rate
    printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", ti, (int) len, rt);

    close(sockfd);
    fclose(fp);
    //}
    exit(0);
}

float str_cli(FILE *fp, int sockfd, long *len, uint8_t error_probability) {
    char *buf;
    char packet_ack;
    uint32_t packet_send = 0;
    uint32_t error_count = 0;
    uint8_t timeout_count = 0;
    long fileLength, charIndex;
    float error_percentage = 0;


    struct pack_so packet;
    struct ack_so ack;


    int bytesSent, bytesReceived, packetSize;
    float time_inv = 0.0;
    struct timeval sendt, recvt;
    charIndex = 0; // character index

    srand(time(NULL));

    fseek(fp, 0, SEEK_END); // set the file pointer to end of file
    fileLength = ftell(fp); // get the file pointer position  
    rewind(fp); // set file pointer back to the start

    printf("The file length is %d bytes\n", (int) fileLength);
    printf("the packet length is %d bytes\n", DATALEN);

    // allocate memory to contain the whole file.
    buf = (char *) malloc(fileLength);
    if (buf == NULL) exit(2);

    // copy the file into the buffer.
    fread(buf, 1, fileLength, fp);

    /*** the whole file is loaded in the buffer. ***/
    buf[fileLength] = '\a'; //append the end byte
    gettimeofday(&sendt, NULL); //get the current time
    while (charIndex <= fileLength) {
        // split message into packets
        if ((fileLength + 1 - charIndex) <= DATALEN)
            packet.len = fileLength + 1 - charIndex;
        else
            packet.len = DATALEN;

        memcpy(packet.data, (buf + charIndex), packet.len); // copy the buffer
        //packetSize = packet.len + HEADLEN; // set packet size to be dataSize + 5 bytes for ack and seq number
        packet.seq_num = !packet.seq_num; // set packet seq_num
        packet_ack = 0;
        // end of packets splitting

        printf("\n New unique packet \n");
        

        while (!packet_ack) // keep retransmit if not acknowledged
        {
            // pack.......
            // Start of send

            packet_send++;

            printf("Packet # sent %d \n", packet_send);

            printf("Packet seq number is %d \n", packet.seq_num);

            printf("Packet length is %d \n", packet.len);

            int random = (rand() % 100 + 1); // ..100 >= error_probability
            if (random > error_probability) {
                packetSize = packet.len + HEADLEN;

            } else // set fake damaged frame
            {
                printf("Set damaged frame\n");

                if (packet.len > 10)
                    packetSize = (packet.len + HEADLEN) - (rand() % 10 + 1);
                else if (packet.len > 0)
                    packetSize = (packet.len - 1) + HEADLEN;
                else
                    packetSize = packet.len + HEADLEN;

                if (PACK_SIZE < 0) {

                    printf("Unable to set error probability when packet size is <= 0");
                    exit(1);
                }

            }


            bytesSent = send(sockfd, &packet, packetSize, 0); // send the packet..


            // return number of bytes sent out or -1 on error
            if (bytesSent == -1) // error....
            {
                printf("send error!\n");
                exit(1);
            } else if (bytesSent != packetSize) {
                printf("Expected bytes sent is %d but bytes sent was %d\n", packetSize, bytesSent);
            }                // Sent successful
            else {

             
                bytesReceived = recv(sockfd, &ack, HEADLEN, 0);

                if (bytesReceived == -1) {
                    if (errno == EAGAIN) {
                        printf("Timeout occurred before data is received\n");
                    } else {
                        perror("Received Error: ");
                    }

                    error_count++;
                    printf("Time out #%d\n", ++timeout_count);

                    if (timeout_count >= MAX_TIMEOUT) {
                        printf("Max timeout retries reached :%d):\n", MAX_TIMEOUT);
                        exit(1);
                    }
                }
                    // received without timeout
                else {
                    if (ack.len != packet.len) {
                        error_count++;
                        printf("Acknowledge length mismatched, ack should be %d but was %d\n", packet.len, ack.len);

                    } else if (ack.seq_num == packet.seq_num) {
                        error_count++;
                        printf("Expected sequence mismatched\n");

                    } else {
                        packet_ack = 1;
                        printf("Packet sent successfully\n");
                    }

                }

            }

        }

        timeout_count = 0; // reset timeout_count
        charIndex += packet.len;
        // End of send  
    }


    gettimeofday(&recvt, NULL);
    printf("Error count is %d  and packet send is %d\n", error_count, packet_send);
    error_percentage = ((float) error_count / packet_send * 100);
    printf("Error percentage is %.2f %%\n", error_percentage);
    *len = charIndex; //get current time
    tv_sub(&recvt, &sendt); // get the whole trans time
    time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec) / 1000.0;
    return (time_inv);
}

void set_receive_timeout(int *sockfd) {
    printf("sockfd then is %d", *sockfd);

    struct timeval timeout;
    int ret; // return value
    timeout.tv_sec = TIMEOUT_SEC; /* 0.5 sec Timeout */
    timeout.tv_usec = TIMEOUT_USEC;
    ret = setsockopt(*sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof (timeout));
    if (ret < 0) {
        perror("Set timeout Error: ");
        exit(1);
    } else {

    }
}

void tv_sub(struct timeval *out, struct timeval *in) {
    if ((out->tv_usec -= in->tv_usec) < 0) {
        --out ->tv_sec;
        out ->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

uint8_t get_error_probability(char *arg) {
    int error_probability;

    if (!(sscanf(arg, "%d", &error_probability))) {
        perror("Unable to parse Error Probability : ");
        exit(1);
    }
    if (error_probability > 100 || error_probability < 0) {
        printf("Invalid range for Error Probability");
        exit(1);
    }

    return (uint8_t)error_probability;
}