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

#define IP 20
#define START_SENT_CLASS_OBJ (sizeof(struct rsvp_header))
#define START_RECV_CLASS_OBJ (IP + START_SENT_CLASS_OBJ)

#define START_SENT_SESSION_OBJ (sizeof(struct rsvp_header))// + sizeof(struct class_obj))
#define START_RECV_SESSION_OBJ (IP + START_SENT_SESSION_OBJ)

#define START_SENT_LABEL_REQ (START_SENT_SESSION_OBJ + sizeof(struct session_object)) 
#define START_RECV_LABEL_REQ (IP +  START_SENT_LABEL_REQ)

#define START_SENT_SESSION_ATTR_OBJ (START_SENT_LABEL_REQ + sizeof(struct label_req_object))
#define START_RECV_SESSION_ATTR_OBJ (IP + START_SENT_SESSION_ATTR_OBJ)

#define START_SENT_SENDER_TEMP_OBJ (START_SENT_SESSION_ATTR_OBJ + sizeof(struct session_attr_object))
#define START_RECV_SENDER_TEMP_OBJ (IP + START_SENT_SENDER_TEMP_OBJ)

#define START_SENT_LABEL (sizeof(struct rsvp_header) + sizeof(struct class_obj)
#define START_RECV_LABEL (IP + START_SENT_LABEL) 



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

// common class Object
struct class_obj {
    uint8_t class_num;
    uint8_t c_type;
    uint16_t length;
};

// Label Object for PATH Message
struct label_req_object {
    struct class_obj class_obj;
    uint16_t Reserved;
    uint16_t L3PID;
};

//  Session Object for PATH and RESV MessagE
struct session_object {
    struct class_obj class_obj;
    struct in_addr dst_ip;
    uint16_t Reserved;
    uint16_t tunnel_id;
    struct in_addr src_ip;
};

// Session Attribute Object for PATH Message
struct session_attr_object {
    struct class_obj class_obj;
    uint8_t setup_prio;
    uint8_t hold_prio;
    uint8_t flags;
    uint8_t name_len;
    char Name[32];
};

// Sender Template  Object for PATH Message
struct sender_temp_object {
    struct class_obj class_obj;
    struct in_addr src_ip;
    uint16_t Reserved;
    uint16_t LSP_ID;
};


// Label Object for RESV Message
struct label_object {
    struct class_obj class_obj;
    uint32_t label;
};

//Filter Spec Object for RESV Message
struct Filter_spec_object {
    struct class_obj class_obj;
    struct in_addr src_ip;
    uint16_t Reserved;
    uint16_t LSP_ID;
};
