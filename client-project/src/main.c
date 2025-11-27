/*
 * main.c
 *
 * TCP Client - Weather request client
 *
 * Compatibile con Windows, Linux e macOS
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define closesocket close
#endif

#include "protocol.h"

void clearwinsock() {
#if defined(_WIN32)
    WSACleanup();
#endif
}

int main(int argc, char* argv[]) {

#if defined(_WIN32)
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) {
        printf("Errore WSAStartup\n");
        return -1;
    }
#endif

    int port = SERVER_PORT;
    char ip[32] = DEFAULT_IP;
    char request_string[128] = "";
    int richiesta = 0;

    // --- Parsing degli argomenti ---
    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            strncpy(ip, argv[++i], 31);
            ip[31] = '\0'; // assicura terminatore
        }

        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        }

        else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            strncpy(request_string, argv[++i], 127);
            request_string[127] = '\0'; // assicura terminatore
            richiesta = 1;
        }
    }

    if (!richiesta) {
        printf("Uso: %s [-s server] [-p port] -r \"tipo citta'\"\n", argv[0]);
        clearwinsock();
        return -1;
    }

    // --- Costruzione della struttura richiesta ---
    weather_request_t req;
    memset(&req, 0, sizeof(req));

    int i = 0;

    // Skip spazi iniziali
    while (request_string[i] == ' ' && request_string[i] != '\0')
        i++;

    // Tipo (T, H, W, P)
    req.type = request_string[i++];

    // Skip spazi dopo il tipo
    while (request_string[i] == ' ' && request_string[i] != '\0')
        i++;

    // Nome città
    strncpy(req.city, &request_string[i], 63);
    req.city[63] = '\0';

    // --- Creazione socket ---
    int c_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (c_socket < 0) {
        perror("socket");
        clearwinsock();
        return -1;
    }

    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));

    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr(ip);
    sad.sin_port = htons(port);

    // --- Connessione ---
    if (connect(c_socket, (struct sockaddr*)&sad, sizeof(sad)) < 0) {
        perror("connect");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }

    // --- Invio richiesta ---
    if (send(c_socket, (char*)&req, sizeof(req), 0) < 0) {
        perror("send");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }

    // --- Ricezione risposta ---
    weather_response_t resp;
    if (recv(c_socket, (char*)&resp, sizeof(resp), 0) <= 0) {
        perror("recv");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }

    // --- Elaborazione risposta ---
    printf("Risposta dal server (%s): ", ip);

    if (resp.status == STATUS_SUCCESS) {

        req.city[0] = (char)toupper((unsigned char)req.city[0]);

        switch (resp.type) {

            case TYPE_TEMPERATURE:
                printf("%s: Temperatura = %.1f°C\n", req.city, resp.value);
                break;

            case TYPE_HUMIDITY:
                printf("%s: Umidità = %.1f%%\n", req.city, resp.value);
                break;

            case TYPE_WIND:
                printf("%s: Vento = %.1f km/h\n", req.city, resp.value);
                break;

            case TYPE_PRESSURE:
                printf("%s: Pressione = %.1f hPa\n", req.city, resp.value);
                break;
        }
    }
    else if (resp.status == STATUS_CITY_UNAVAILABLE) {
        printf("Città non disponibile\n");
    }
    else if (resp.status == STATUS_INVALID_REQUEST) {
        printf("Richiesta non valida\n");
    }
    else {
        printf("Errore sconosciuto\n");
    }

    closesocket(c_socket);
    clearwinsock();
    return 0;
}
