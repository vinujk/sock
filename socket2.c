#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <sys/time.h>
#include "socket.h"
#include <time.h>
#include <signal.h>

int path_received = 0;
int sock = 0;

struct in_addr sender_ip, receiver_ip;

extern struct session* sess;
extern struct session* head;
		
int main() {
    sock = socket(AF_INET, SOCK_RAW, RSVP_PROTOCOL);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

     char buffer[1024];

    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    struct in_addr sender_ip, receiver_ip;

    resv_event_handler();
    while(1) {
   	memset(buffer, 0, sizeof(buffer));
	int bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
       				(struct sockaddr*)&sender_addr, &addr_len);
       	if (bytes_received < 0) {
	        perror("Receive failed");
       		continue;
	}

       	struct rsvp_header *rsvp = (struct rsvp_header*)(buffer+20);

	switch(rsvp->msg_type) {

		case PATH_MSG_TYPE: 
			//Receive PATH Message
		   	//next = time(NULL)s
			printf("insert_session\n");
			if(sess == NULL) {
				sess = insert_session(sess, "192.168.11.11", "192.168.11.12"); 
				head = sess;
				printf("------- inserted session when NULL\n");
			} else {
				insert_session(sess, "192.168.11.11", "192.168.11.12");
				printf("------- inserted session\n");
			}
	
			printf(" session = %s %s \n",sess->sender, sess->receiver);
			receive_path_message(sock,buffer,sender_addr);
			//now = next;
			break;

		case RESV_MSG_TYPE: 
    			// Receive RSVP-TE RESV Message
			//receive_resv_message(sock,label_obj,sender_addr);
			break;

	}
    }
    close(sock);
    return 0;
}

