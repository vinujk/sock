#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include "socket.h"

int main() {
    int sock = socket(AF_INET, SOCK_RAW, RSVP_PROTOCOL);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

     char buffer[1024];

    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    struct in_addr sender_ip, receiver_ip;
 
    while(1) {
   	memset(buffer, 0, sizeof(buffer));
	int bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
       				(struct sockaddr*)&sender_addr, &addr_len);
       	if (bytes_received < 0) {
	        perror("Receive failed");
       		continue;
	}

       	struct rsvp_header *rsvp = (struct rsvp_header*)(buffer+20);

	printf(" received msg-type = %d\n", rsvp->msg_type);
	switch(rsvp->msg_type) {

		case PATH_MSG_TYPE: 
			//Receive PATH Message
			receive_path_message(sock,buffer,sender_addr);
			break;

		case RESV_MSG_TYPE: 
    			// Receive RSVP-TE RESV Message
			//receive_resv_message(sock,label_obj,sender_addr);
			break;

	}
	break;
    }
    close(sock);
    return 0;
}

