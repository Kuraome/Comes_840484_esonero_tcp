#ifndef PROTOCOL_H
#define PROTOCOL_H

#define SERVER_PORT 9000
#define DEFAULT_IP "127.0.0.1"

#define TYPE_TEMPERATURE 'T'
#define TYPE_HUMIDITY    'H'
#define TYPE_WIND        'W'
#define TYPE_PRESSURE    'P'

#define STATUS_SUCCESS            0
#define STATUS_INVALID_REQUEST    1
#define STATUS_CITY_UNAVAILABLE   2

typedef struct {
    char type;
    char city[64];
} weather_request_t;

typedef struct {
    int status;
    char type;
    float value;
} weather_response_t;

#endif
