#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "timer-event.h"
#include "rsvp_msg.h"
#include "rsvp_db.h"

extern int sock;
extern struct in_addr sender_ip, receiver_ip;

extern struct session* sess;
extern struct session* head;

#define TIMEOUT 90
#define INTERVAL 30


// Timer event handler for Path
void path_timer_handler(union sigval sv) {
    inet_pton(AF_INET, "192.168.11.11", &sender_ip);  // Ingress Router
    inet_pton(AF_INET, "192.168.11.12", &receiver_ip);  // Egress Router
    // Send RSVP-TE PATH Message
    send_path_message(sock, sender_ip, receiver_ip);
}

// Timer event handler for Resv
void resv_timer_handler(union sigval sv) {
    time_t now = time(NULL);

        sess = head;
        printf("timer handler \n");
        while(sess != NULL) {
                if((now - sess->last_path_time) > TIMEOUT) {
                        printf("RSVP session expired: %s->%s\n",sess->sender, sess->receiver);
                        head = delete_session(head, sess->sender, sess->receiver);
                } else if((now - sess->last_path_time) < INTERVAL) {
                        printf(" less than 30 sec\n");
                        sess = sess->next;
                        continue;
                } else {
                        printf("--------sebding resv message\n");

                        inet_pton(AF_INET, sess->sender, &sender_ip);
                        inet_pton(AF_INET, sess->receiver, &receiver_ip);
                        send_resv_message(sock, sender_ip, receiver_ip);
                }
                sess = sess->next;
        }
}

// Function to create a timer that triggers every 30 seconds
timer_t create_timer(void (*handler)(union sigval)) {
    struct sigevent sev;
    timer_t timerid;

    memset(&sev, 0, sizeof(struct sigevent));
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = handler;

    if (timer_create(CLOCK_REALTIME, &sev, &timerid) < 0) {
        perror("Timer creation failed");
        exit(EXIT_FAILURE);
    }
    return timerid;
}


// Function to start a timer with a 30-second interval
void start_timer(timer_t timerid) {
    struct itimerspec its;
    its.it_value.tv_sec = 30;   // Initial delay
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 30; // Repeating interval
    its.it_interval.tv_nsec = 0;

    if (timer_settime(timerid, 0, &its, NULL) < 0) {
        perror("Timer start failed");
        exit(EXIT_FAILURE);
    }
}

void path_event_handler() {
        timer_t path_timer = create_timer(path_timer_handler);
        start_timer(path_timer);
}

void resv_event_handler() {
        timer_t resv_timer = create_timer(resv_timer_handler);
        start_timer(resv_timer);
}

