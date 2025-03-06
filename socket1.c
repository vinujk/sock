#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include "socket.h"
#include <linux/if.h>
#include <sys/ioctl.h>

/*void ifreq_output() {
	struct ifreq ifreq_i;
    memset(&ifreq_i, 0, sizeof(ifreq_i));
    strncpy(ifreq_i.ifr_name, "ens3", IFNAMSIZ-1);

    if((ioctl(sock, SIOCGIFHWADDR, &ifreq_i)) < 0)
        printf("----------failed\n");
    else {
        unsigned char *mac = (unsigned char *)ifreq_i.ifr_hwaddr.sa_data;
        printf("mac addr = %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        struct sockaddr_in *addr = ((struct sockaddr_in*)&ifreq_i.ifr_hwaddr);
        printf("index = %d %s\n", ifreq_i.ifr_ifindex, inet_ntoa(addr->sin_addr));
    }

}*/

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

