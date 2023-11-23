#include <iostream>

#include "../../include/nss.h"
#include "../../include/ssl.h"

int main() {
    // Initialisiere NSS
    NSS_NoDB_Init(NULL);

    // Initialisiere SSL
    PRFileDesc *socket = PR_NewTCPSocket();
    PR_SetSockOpt(socket, PR_SockOpt_Nonblocking, PR_TRUE);
    PRSocketOptionData opt = {.option = PR_SockOpt_Linger, .value.linger = {0, 0}};
    PR_SetSockOpt(socket, PR_SockOpt_Linger, &opt);

    // Erzeuge einen SSL-Socket
    PRFileDesc *sslSocket = SSL_ImportFD(NULL, socket);

    // Initialisiere den TLS-Handshake
    SSL_ResetHandshake(sslSocket, PR_TRUE);

    // Führe den TLS-Handshake durch
    if (SSL_ForceHandshake(sslSocket) == SECSuccess) {
        // TLS-Handshake erfolgreich
        // Hier kannst du mit dem sicheren Datenaustausch fortfahren
    }

    // Aufräumen
    SSL_Shutdown(sslSocket, PR_SHUTDOWN_BOTH);
    PR_Close(socket);

    return 0;
}