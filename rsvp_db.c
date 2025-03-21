#include "rsvp_db.h"
#include "timer-event.h"

struct session* sess = NULL;
struct session* head = NULL;
time_t now = 0;

	
struct session* insert_session(struct session* sess, char sender[], char receiver[]) {
        now = time(NULL);
        printf("insert session\n");
        if(sess == NULL) {
                struct session *temp = (struct session*)malloc(sizeof(struct session));
                if(temp < 0)
                         printf("cannot allocate dynamic memory]n");

                temp->last_path_time = now;
                strcpy(temp->sender, sender);
                strcpy(temp->receiver, receiver);
                temp->next = NULL;
                return temp;
        } else {
                while(sess->next != NULL) {
                        if((strcmp(sess->sender, sender) == 0) &&
                           (strcmp(sess->receiver, receiver) == 0)) {
                                return;
                        }
                        sess=sess->next;
                }

                struct session *temp = (struct session*)malloc(sizeof(struct session));
                if(sess < 0)
                         printf("cannot allocate dynamic memory\n");

                temp->last_path_time = now;
                strcpy(temp->sender, sender);
                strcpy(temp->receiver, receiver);
                temp->next = NULL;

                sess->next = temp;
        }
}


struct session* delete_session(struct session* sess, char sender[], char receiver[]) {

        struct session *temp = NULL;

        printf("delete session\n");
        while(sess != NULL) {
                if((head == sess) &&
                   (strcmp(sess->sender, sender) == 0) &&
                   (strcmp(sess->receiver, receiver) == 0)) {
                        temp = head;
                        head = head->next;
                        free(temp);
                        return head;
                } else {
                        if((strcmp(sess->sender, sender) == 0) &&
                           (strcmp(sess->receiver, receiver) == 0)) {
                                *sess = *sess->next;
                                temp = sess->next;
                                free(temp);
                        }else{
                                sess = sess->next;
                        }
                }
        }
}

