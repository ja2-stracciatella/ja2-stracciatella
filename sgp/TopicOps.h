#ifndef _TopicOps_h
#define _TopicOps_h

// debug levels

#define DBG_LEVEL_0       0	// for registering and unregistering topics only
#define DBG_LEVEL_1       1	// for basic stuff
#define DBG_LEVEL_2       2	// for ordinary, I usually want to see them, messages
#define DBG_LEVEL_3       3	// nitty gritty detail

// from client

#define TOPIC_REGISTER    0
#define TOPIC_UNREGISTER  1
#define TOPIC_MESSAGE     2
#define CLIENT_REGISTER	  3
#define CLIENT_SHUTDOWN	  4

// from server

#define SYSTEM_SHUTDOWN   0
#define MODULE_RESET		  1
#define SET_DEBUG_LEVEL	  2

#endif
