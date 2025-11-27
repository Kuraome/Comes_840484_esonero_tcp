#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#define strcasecmp _stricmp
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#define closesocket close
#endif
#define QUEUE_SIZE 6

#include "protocol.h"

// --- Funzioni di generazione valori casuali ---
float get_temperature(void) { return -10.0f + (rand() % 500) / 10.0f; }
float get_humidity(void)    { return 20.0f  + (rand() % 801) / 10.0f; }
float get_wind(void)        { return (rand() % 1001) / 10.0f; }
float get_pressure(void)    { return 950.0f + (rand() % 1001) / 10.0f; }

// --- Lista città supportate ---
const char* cities[] = {
    "bari","roma","milano","napoli","torino",
    "palermo","genova","bologna","firenze","venezia"
};

int is_supported_city(const char* city) {

    for (int i = 0; i < 10; i++)
        if (strcasecmp(city, cities[i]) == 0)
            return 1;
    return 0;
}

void errorhandler(char *errorMessage)
{
    printf("%s", errorMessage);
}

// ----------------------------------------------
//                MAIN SERVER
// ----------------------------------------------
int main() {

#if defined(_WIN32)
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        printf("Errore WSAStartup()\n");
        return -1;
    }
#endif

    srand(time(NULL));

    int s_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s_socket < 0) {
        errorhandler("socket");
        return -1;
    }

    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(SERVER_PORT);

    if (bind(s_socket, (struct sockaddr*)&sad, sizeof(sad)) < 0) {
        errorhandler("bind");
        return -1;
    }

    if (listen(s_socket, QUEUE_SIZE) < 0) {
    	errorhandler("listen");
        return -1;
    }

    while (1) {

        struct sockaddr_in cad;
        socklen_t cad_len = sizeof(cad);

        int c_socket = accept(s_socket, (struct sockaddr*)&cad, &cad_len);
        if (c_socket < 0) continue;

        weather_request_t req;
        if (recv(c_socket, (char*)&req, sizeof(req), 0) <= 0) {
            closesocket(c_socket);
            continue;
        }

        printf("Richiesta %c %s dal client ip %s\n",req.type,req.city, inet_ntoa(cad.sin_addr));

        weather_response_t resp;
        memset(&resp, 0, sizeof(resp));
        req.type=tolower(req.type);

        // Validazione della richiesta
        if (req.type != TYPE_TEMPERATURE &&
            req.type != TYPE_HUMIDITY &&
            req.type != TYPE_WIND &&
            req.type != TYPE_PRESSURE) {

            resp.status = STATUS_INVALID_REQUEST;
            resp.type = '\0';
            resp.value = 0.0f;
        }
        else if (!is_supported_city(req.city)) {
            resp.status = STATUS_CITY_UNAVAILABLE;
            resp.type = '\0';
            resp.value = 0.0f;
        }
        else {
            resp.status = STATUS_SUCCESS;
            resp.type = req.type;

            switch (req.type) {
                case TYPE_TEMPERATURE: resp.value = get_temperature(); break;
                case TYPE_HUMIDITY:    resp.value = get_humidity();    break;
                case TYPE_WIND:        resp.value = get_wind();        break;
                case TYPE_PRESSURE:    resp.value = get_pressure();    break;
            }
        }

        send(c_socket, (char*)&resp, sizeof(resp), 0);
        closesocket(c_socket);
    }

#if defined(_WIN32)
    WSACleanup();
#endif

    return 0;
}
