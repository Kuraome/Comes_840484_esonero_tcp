#ifndef PROTOCOL_H
#define PROTOCOL_H

#define SERVER_PORT 56700
#define DEFAULT_IP "127.0.0.1"

// Tipi di richiesta
#define TYPE_TEMPERATURE 't'
#define TYPE_HUMIDITY 'h'
#define TYPE_WIND 'w'
#define TYPE_PRESSURE 'p'

// Codici di stato
#define STATUS_SUCCESS 0
#define STATUS_CITY_UNAVAILABLE 1
#define STATUS_INVALID_REQUEST 2

// Strutture
typedef struct {
    char type;
    char city[64];
} weather_request_t;

typedef struct {
	unsigned int status;
    char type;
    float value;
} weather_response_t;



void errorhandler(char *errorMessage);

#endif
