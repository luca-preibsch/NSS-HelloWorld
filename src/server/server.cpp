#include <iostream>

#include "../../include/nss.h"
#include "../../include/ssl.h"
#include "../../include/nspr.h"

#define DB_DIR "./"
#define SERVER_PORT 12345

using namespace std;

void die(const string& error_msg) {
    cout << error_msg << endl;
    exit(EXIT_FAILURE);
}

void log(const string& msg) {
    cout << msg << endl;
}

int main() {
    // must be called before any other NSS function
    PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

    // set up NSS config; not idempotent, only call once
    NSS_Init(DB_DIR);

    log("NSS initialized");

    // create NSPR legal TCP socket
    PRFileDesc* listen_sock = PR_NewTCPSocket();
    if (!listen_sock) {
        die("Error creating TCP socket");
    }

    // bind listen sock to a specific port
    PRNetAddr listen_addr;
    listen_addr.inet.family = PR_AF_INET;
    listen_addr.inet.ip = PR_INADDR_ANY;
    listen_addr.inet.port = PR_htons(SERVER_PORT);
    if (PR_Bind(listen_sock, &listen_addr)) {
        die("Error binding listen socket");
        PR_Close(listen_sock);
    }

    // TODO: needed for listen socket?
    // create SSL socket from TCP socket
    listen_sock = SSL_ImportFD(nullptr, listen_sock);
    if (!listen_sock) {
        die("Error importing listen socket into SSL library");
    }

    // start to listen on the socket
    if (PR_Listen(listen_sock, 0)) {
        die("Error starting to listen for listen socket");
        PR_Close(listen_sock);
    }

    log("started listening");

    PRNetAddr client_addr;
    PRFileDesc* tcp_sock = PR_Accept(listen_sock, &client_addr, PR_INTERVAL_NO_TIMEOUT);
    if (!tcp_sock) {
        die("Error accepting client connection");
        PR_Close(listen_sock);
    }

    log("accepted connection");

    // create SSL socket from TCP socket
    PRFileDesc* ssl_sock = SSL_ImportFD(nullptr, tcp_sock);
    if (!listen_sock) {
        die("Error importing TCP socket into SSL library");
        PR_Close(listen_sock);
        PR_Close(tcp_sock);
    }

    // Read "Hello World!"
    int buf_len = strlen("Hello World!") + 1; // +1 for /0
    string buf[buf_len];
    PR_Read(ssl_sock, &buf, buf_len);
    cout << "Message:/n" << buf << endl;

    PR_Close(listen_sock);
    PR_Close(ssl_sock);
    PR_Close(tcp_sock); // not needed because PR_Close(ssl_sock)?
    NSS_Shutdown();

    log("shutting down");

    return 0;
}

//int main() {
//    // NSS initialisieren
//    NSS_Init("");
//
//    // SSL-Server erstellen
//    PRFileDesc* serverSocket = SSL_CreateServerSocket(PR_AF_INET, PR_STREAM_LISTEN, 0, 443, nullptr);
//    if (!serverSocket) {
//        fprintf(stderr, "Fehler beim Erstellen des Server-Sockets\n");
//        return 1;
//    }
//
//    // Auf Verbindungen warten
//    PRFileDesc* sslSocket = SSL_ImportFD(nullptr, serverSocket);
//    PRStatus status = PR_Accept(sslSocket, nullptr, PR_INTERVAL_NO_TIMEOUT);
//    if (status != PR_SUCCESS) {
//        fprintf(stderr, "Fehler beim Akzeptieren der Verbindung\n");
//        return 1;
//    }
//
//    // Daten über die TLS-Verbindung senden/empfangen
//    // Implementiere hier deine Logik für die Datenübertragung
//
//    // Aufräumen
//    PR_Close(serverSocket);
//    NSS_Shutdown();
//
//    return 0;
//}
