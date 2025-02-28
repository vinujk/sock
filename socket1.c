#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>

#define RSVP_PROTOCOL 46  // RSVP IP Protocol Number
#define PATH_MSG_TYPE 1   // RSVP-TE PATH Message Type
#define RESV_MSG_TYPE 2   // RSVP-TE RESV Message Type

#define SESSION 1
#define FILTER_SPEC 10
#define SENDER_TEMPLATE 11
#define RSVP_LABEL 16
#define LABEL_REQUEST 19
#define EXPLICIT_ROUTE 20
#define RECORD_ROUTE 21
#define HELLO 22
#define SESSION_ATTRIBUTE 207

// RSVP Common Header (Simplified)
struct rsvp_header {
    uint8_t version_flags;
    uint8_t msg_type;
    uint16_t checksum;
    uint8_t ttl;
    uint16_t length;
    uint8_t reserved;
    struct in_addr sender_ip;
    struct in_addr receiver_ip;
};

// Label Object for RESV Message
struct label_req_object {
    uint8_t class_num;
    uint8_t c_type;
    uint16_t length;
    uint16_t Reserved;
    uint16_t L3PID;
};

// Label Object for RESV Message
struct label_object {
    uint8_t class_num;
    uint8_t c_type;
    uint16_t length;
    uint32_t label;
};

// Function to send an RSVP-TE PATH message
void send_path_message(int sock, struct in_addr sender_ip, struct in_addr receiver_ip) {
    struct sockaddr_in dest_addr;
    char path_packet[64];

    struct rsvp_header *path = (struct rsvp_header*)path_packet;
    struct label_req_object *label_req_obj = (struct label_req_object*)(path_packet + sizeof(struct rsvp_header));

    // Populate RSVP PATH header
    path->version_flags = 0x10;  // RSVP v1
    path->msg_type = PATH_MSG_TYPE;
    path->length = htons(sizeof(path_packet));
    path->checksum = 0;
    path->ttl = 255;
    path->reserved = 0;
    path->sender_ip = sender_ip;
    path->receiver_ip = receiver_ip;

    // Populate Label Object                                                      
    label_req_obj->class_num = 19;  // Label Request class                                    
    label_req_obj->c_type = 1;  // Generic Label                                      
    label_req_obj->length = htons(sizeof(struct label_req_object));                       
    label_req_obj->L3PID = htonl(0x0800);  // Assigned Label (1001)

    // Set destination (egress router)
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = receiver_ip;
    dest_addr.sin_port = 0;

    // Send PATH message
    if (sendto(sock, path_packet, sizeof(path_packet), 0, 
               (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    } else {
        printf("Sent PATH message to %s\n", inet_ntoa(receiver_ip));
    }
}

// Function to receive an RSVP-TE RESV message
void receive_resv_message(int sock, struct label_object *label_obj, struct sockaddr_in sender_addr) {

    printf("Listening for RSVP-TE RESV messages...\n");

//    struct label_object *label_obj = (struct label_object*)(buffer + 20 + sizeof(struct rsvp_header));

	switch(label_obj->class_num) {
	
		case RSVP_LABEL: 
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
			label_obj = (struct label_object*)(buffer + 20 + sizeof(struct rsvp_header));
			receive_resv_message(sock,label_obj,sender_addr);
			break;

	}
	break;
    }

    close(sock);
    return 0;
}

