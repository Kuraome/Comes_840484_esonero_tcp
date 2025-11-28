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
#include <time.h>

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

//cleanup per winsock
void clearwinsock() {
#if defined(_WIN32)
    WSACleanup();
#endif
}

void errorhandler(char *errorMessage)
{
    printf("%s", errorMessage);
}

int main(int argc, char* argv[]) {
//inizializzazione winsock
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

    // struttura da inviare al server
    weather_request_t request;
    memset(&request, 0, sizeof(request));

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
    	 printf("Uso: %s [-s server] [-p port] -r \"tipo città\"\n", argv[0]);
    	        clearwinsock();
            return -1;
        }

    //parsing della stringa richiesta
    int i = 0;
    while (request_string[i]== ' ' &&request_string[i] != '\0')i++;

    request.type = request_string[i];
    i++;

    while (request_string[i]== ' ' &&request_string[i] != '\0')i++;
    strncpy (request.city, &request_string[i], 63);
    request.city[63] = '\0';

    int c_socket =socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(c_socket < 0){
    	errorhandler ("Creazione del socket fallita.\n");
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
        errorhandler("connect");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }

    // --- Invio richiesta ---
    if (send(c_socket, (char*)&request, sizeof(request), 0) < 0) {
    	errorhandler("send");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }

    // --- Ricezione risposta ---
    weather_response_t response;
    if (recv(c_socket, (char*)&response, sizeof(response), 0) <= 0) {
    	errorhandler("recv() fallita o connessione chiusa prematuratamente");
        closesocket(c_socket);
        clearwinsock();
        return -1;
    }

    if (response.status == STATUS_SUCCESS) {

        request.city[0] = (char)toupper((unsigned char)request.city[0]);

        switch (response.type) {

            case TYPE_TEMPERATURE:
                printf("Ricevuto risultato dal server ip %s. %s: Temperatura = %.1f%cC\n", ip, request.city, response.value,248);
                break;

            case TYPE_HUMIDITY:
                printf("Ricevuto risultato dal server ip %s. %s: Umidita' = %.1f%%\n", ip, request.city, response.value);
                break;

            case TYPE_WIND:
                printf("Ricevuto risultato dal server ip %s. %s: Vento = %.1f km/h\n",ip ,request.city, response.value);
                break;

            case TYPE_PRESSURE:
                printf("Ricevuto risultato dal server ip %s. %s: Pressione = %.1f hPa\n", ip, request.city, response.value);
                break;
        }
    }
    else if (response.status == STATUS_CITY_UNAVAILABLE) {
        printf("Ricevuto risultato dal server ip %s. Citta' non disponibile\n", ip);
    } else if (response.status == STATUS_INVALID_REQUEST) {
        printf("Ricevuto risultato dal server ip %s. Richiesta non valida\n", ip);
    } else {
    	printf("Errore sconosciuto\n");
    }

//chiusura
    closesocket(c_socket);
    clearwinsock();
    return 0;
}
