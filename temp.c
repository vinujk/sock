unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    char packet[1024];
    struct iphdr *ip = (struct iphdr *)packet;
    struct icmphdr *icmp = (struct icmphdr *)(packet + sizeof(struct iphdr));

    memset(packet, 0, sizeof(packet));

    // Fill in the IP Header
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
    ip->id = htons(54321);
    ip->ttl = 64;
    ip->protocol = IPPROTO_ICMP;
    ip->saddr = inet_addr("172.168.10.2"); // Set custom source IP
    ip->daddr = inet_addr("172.168.30.2"); // Destination IP
    ip->check = checksum(ip, sizeof(struct iphdr));

    // Fill in the ICMP Header
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(1234);
    icmp->un.echo.sequence = htons(1);
    icmp->checksum = 0;
    icmp->checksum = checksum(icmp, sizeof(struct icmphdr));

    // Set socket option to include IP header
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        return 1;
    }

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = ip->daddr;

    if (sendto(sockfd, packet, sizeof(struct iphdr) + sizeof(struct icmphdr), 0,
               (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("sendto");
        return 1;
    }

    printf("ICMP Echo Request sent from 172.168.10.2 to 172.168.30.2\n");
    close(sockfd);
    return 0;
}
