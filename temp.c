#define MAX_HOPS 30
#define TIMEOUT 1
#define DEST_PORT 33434

// Calculate checksum for ICMP packet
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    for (; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <destination IP>\n", argv[0]);
        return 1;
    }

    char *dest_ip = argv[1];
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in dest_addr = {0};
    dest_addr.sin_family = AF_INET;
    inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr);

    for (int ttl = 1; ttl <= MAX_HOPS; ttl++) {
        // Set TTL
        if (setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
            perror("setsockopt");
            return 1;
        }

        // Set socket timeout
        struct timeval timeout = {TIMEOUT, 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        // Build ICMP Echo Request
        char sendbuf[64];
        struct icmp *icmp_hdr = (struct icmp *)sendbuf;
        memset(sendbuf, 0, sizeof(sendbuf));
        icmp_hdr->icmp_type = ICMP_ECHO;
        icmp_hdr->icmp_code = 0;
        icmp_hdr->icmp_id = getpid();
        icmp_hdr->icmp_seq = ttl;
        icmp_hdr->icmp_cksum = checksum(sendbuf, sizeof(sendbuf));

        struct timeval start, end;
        gettimeofday(&start, NULL);

        sendto(sock, sendbuf, sizeof(sendbuf), 0,
               (struct sockaddr *)&dest_addr, sizeof(dest_addr));

        // Receive ICMP reply
        char recvbuf[1024];
        struct sockaddr_in reply_addr;
        socklen_t addrlen = sizeof(reply_addr);

        int n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                         (struct sockaddr *)&reply_addr, &addrlen);

        gettimeofday(&end, NULL);

        double rtt = (end.tv_sec - start.tv_sec) * 1000.0 +
                     (end.tv_usec - start.tv_usec) / 1000.0;

        if (n < 0) {
            printf("%2d  *\n", ttl);
        } else {
            char addr_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &reply_addr.sin_addr, addr_str, sizeof(addr_str));

            struct ip *ip_hdr = (struct ip *)recvbuf;
            struct icmp *icmp_resp = (struct icmp *)(recvbuf + (ip_hdr->ip_hl << 2));

            printf("%2d  %-15s  %.2f ms\n", ttl, addr_str, rtt);

            // ICMP_ECHOREPLY means we reached the destination
            if (icmp_resp->icmp_type == ICMP_ECHOREPLY) {
                break;
            }
        }
    }

    close(sock);
    return 0;
}
