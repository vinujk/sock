#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include "socket.h"


// Function to send an RSVP-TE PATH message
void send_path_message(int sock, struct in_addr sender_ip, struct in_addr receiver_ip) {
    struct sockaddr_in dest_addr;
    char path_packet[256];

    struct rsvp_header *path = (struct rsvp_header*)path_packet;
    //struct class_obj *class_obj = (struct class_obj*)(path_packet + START_SENT_CLASS_OBJ); 
    struct session_object *session_obj = (struct session_object*)(path_packet + START_SENT_SESSION_OBJ);
    struct hop_object *hop_obj = (struct hop_object*)(path_packet + STARt_SENT_HOP_OBJ);
    struct time_object *time_obj = (struct time_object*)(path_packet + START_SENT_TIME_OBJ);
    struct label_req_object *label_req_obj = (struct label_req_object*)(path_packet + START_SENT_LABEL_REQ); 
    struct session_attr_object *session_attr_obj = (struct session_attr_object*)(path_packet + START_SENT_SESSION_ATTR_OBJ); 
    struct sender_temp_object *sender_temp_obj = (struct sender_temp_object*)(path_packet + START_SENT_SENDER_TEMP_OBJ);

    // Populate RSVP PATH header
    path->version_flags = 0x10;  // RSVP v1
    path->msg_type = PATH_MSG_TYPE;
    path->length = htons(sizeof(path_packet));
    path->checksum = 0;
    path->ttl = 255;
    path->reserved = 0;
    path->sender_ip = sender_ip;
    path->receiver_ip = receiver_ip;

    //session object for PATH msg
    session_obj->class_obj.class_num = 1;
    session_obj->class_obj.c_type = 7;
    session_obj->class_obj.length = htons(sizeof(struct session_object));
    session_obj->dst_ip = receiver_ip;
    //inet_pton(AF_INET, receiver_ip, &session_obj->dst_ip); 
    session_obj->tunnel_id = 1;
    session_obj->src_ip = sender_ip;
    //inet_pton(AF_INET, sender_ip, &session_obj->src_ip);

    //hop object for PATH?RESV msg
    hop_obj->class_obj.class_num = 3;
    hop_obj->class_obj.c_type = 1;
    hop_obj->class_obj.length = htons(sizeof(struct hop_object));
    hop_obj->next_hop = 1.1.1.1; //dummy
    hop_obj->IFH = 123; //dummy

    time_obj->class_obj.class_num = 5;
    time_obj->class_obj.c_type = 1;
    time_obj->class_obj.length = htons(sizeof(struct time_object));
    time_obj->interval = 123 //dummy

    // Populate Label Object                                        
    label_req_obj->class_obj.class_num = 19;  // Label Request class
    label_req_obj->class_obj.c_type = 1;  // Generic Label                   
    label_req_obj->class_obj.length = htons(sizeof(struct label_req_object));
    label_req_obj->L3PID = htonl(0x0800);  // Assigned Label (1001)

    //session attribute object for PATH msg
    session_attr_obj->class_obj.class_num = 207;
    session_attr_obj->class_obj.c_type = 1;
    session_attr_obj->class_obj.length = htons(sizeof(struct session_attr_object));
    session_attr_obj->setup_prio = 7;
    session_attr_obj->hold_prio = 7;
    session_attr_obj->flags = 0;
    session_attr_obj->name_len = sizeof("PE1");
    //strcpy("PE1", session_attr_obj->Name);
    
    //Sender template object for PATH msg
    sender_temp_obj->class_obj.class_num = 11;
    sender_temp_obj->class_obj.c_type = 7;
    sender_temp_obj->class_obj.length = htons(sizeof(struct sender_temp_object));    
    //inet_pton(AF_INET, sender_ip, &sender_temp_obj->src_ip);
    sender_temp_obj->src_ip = sender_ip;
    sender_temp_obj->Reserved = 0;
    sender_temp_obj->LSP_ID = 2;

    // Set destination (egress router)
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = receiver_ip;
    dest_addr.sin_port = 0;

     printf(" sending message1\n");
    // Send PATH message
    if (sendto(sock, path_packet, sizeof(path_packet), 0, 
               (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    } else {
        printf("Sent PATH message to %s\n", inet_ntoa(receiver_ip));
    }
}

// Function to receive an RSVP-TE RESV message
void receive_resv_message(int sock, char buffer[], struct sockaddr_in sender_addr) {

    printf("Listening for RSVP-TE RESV messages...\n");

    struct label_object *label_obj;
    struct class_obj *class_obj = (struct class_obj*)(buffer + 20 + sizeof(struct rsvp_header));
     
    switch(class_obj->class_num) {
	
		case RSVP_LABEL: 
			label_obj = (struct label_object*)(buffer + 20 + sizeof(struct rsvp_header)+sizeof(struct class_obj));
            		printf("Received RESV message from %s with Label %d\n", 
				inet_ntoa(sender_addr.sin_addr), ntohl(label_obj->label));	
			break;
        }
}

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
    inet_pton(AF_INET, "192.168.1.240", &sender_ip);  // Ingress Router
    inet_pton(AF_INET, "192.168.1.244", &receiver_ip);  // Egress Router

    // Send RSVP-TE PATH Message
    send_path_message(sock, sender_ip, receiver_ip);

 
    while(1) {
   	memset(buffer, 0, sizeof(buffer));
	int bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
       				(struct sockaddr*)&sender_addr, &addr_len);
       	if (bytes_received < 0) {
	        perror("Receive failed");
       		continue;
	}

       	struct rsvp_header *rsvp = (struct rsvp_header*)(buffer+20);
	struct label_object *label_obj;

	switch(rsvp->msg_type) {

		case PATH_MSG_TYPE: 
			//Receive PATH Message
			//receive_path_message(sock);
			break;

		case RESV_MSG_TYPE: 
    			// Receive RSVP-TE RESV Message
			receive_resv_message(sock,buffer,sender_addr);
			break;

	}
	break;
    }

    close(sock);
    return 0;
}

