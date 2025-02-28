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

struct class_obj {
    uint8_t class_num;
    uint8_t c_type;
    uint16_t length;
};

struct label_req_object {
    struct class_obj *class_obj;
    uint16_t Reserved;
    uint16_t L3PID;
};

// Label Object for RESV Message
struct label_object {
    struct class_obj *class_obj;
    uint32_t label;
};


// Function to send an RSVP-TE RESV message with label assignment
void send_resv_message(int sock, struct in_addr sender_ip, struct in_addr receiver_ip) {
    struct sockaddr_in dest_addr;
    char resv_packet[64];

    struct rsvp_header *resv = (struct rsvp_header*)resv_packet;
    struct class_obj *class_obj = (struct class_obj*)(resv_packet + sizeof(struct rsvp_header));
    struct label_object *label_obj = (struct label_object*)(resv_packet + sizeof(struct rsvp_header) +sizeof(struct class_obj));

    // Populate RSVP RESV header
    resv->version_flags = 0x10;  // RSVP v1
    resv->msg_type = RESV_MSG_TYPE;
    resv->length = htons(sizeof(resv_packet));
    resv->checksum = 0;
    resv->ttl = 255;
    resv->reserved = 0;
    resv->sender_ip = receiver_ip;
    resv->receiver_ip = sender_ip;

    // Populate Label Object
    class_obj->class_num = 16;  // Label class
    class_obj->c_type = 1;  // Generic Label
    class_obj->length = htons(sizeof(struct label_object));
    label_obj->label = htonl(1001);  // Assigned Label (1001)

    // Set destination (ingress router)
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = sender_ip;
    dest_addr.sin_port = 0;

    // Send RESV message
    if (sendto(sock, resv_packet, sizeof(resv_packet), 0, 
               (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    } else {
        printf("Sent RESV message to %s with Label 1001\n", inet_ntoa(sender_ip));
    }
}

// Function to receive RSVP-TE PATH messages
void receive_path_message(int sock, char buffer[], struct sockaddr_in sender_addr) {
    //char buffer[1024];

    printf("Listening for RSVP-TE PATH messages...\n");
	
	struct rsvp_header *rsvp = (struct rsvp_header*)(buffer+20);

	struct class_obj *class_obj = (struct class_obj*)(buffer + 20 + sizeof(struct rsvp_header));

        printf("Received PATH message from %s\n", inet_ntoa(sender_addr.sin_addr));

        struct in_addr sender_ip = rsvp->sender_ip;
        struct in_addr receiver_ip = rsvp->receiver_ip;

	switch(class_obj->class_num) {

		case LABEL_REQUEST: 
            		// Send a RESV message in response
		        send_resv_message(sock, sender_ip, receiver_ip);
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

    //receive_path_message(sock);

    close(sock);
    return 0;
}

