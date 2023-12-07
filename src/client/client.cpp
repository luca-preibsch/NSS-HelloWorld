#include <iostream>

#include "../../include/nss.h"
#include "../../include/ssl.h"
#include "../../include/nspr.h"

#define DB_DIR "./"
#define HOSTNAME "localhost"
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
    PRFileDesc* tcp_sock = PR_NewTCPSocket();
    if (!tcp_sock) {
        die("Error creating TCP socket");
    }

    log("created TCP socket");

    // create SSL socket from TCP socket
    PRFileDesc* ssl_sock = SSL_ImportFD(nullptr, tcp_sock);
    if (!ssl_sock) {
        die("Error importing ssl_sock socket into SSL library");
    }

    log("imported TCP Socket into NSS");

    char buf[PR_NETDB_BUF_SIZE];
    PRHostEnt host_ent;
    if (PR_FAILURE == PR_GetHostByName(HOSTNAME, buf, PR_NETDB_BUF_SIZE, &host_ent)) {
        die("Error for GetHostByName");
    }

    log("got host by name");

    PRNetAddr server_addr;
//    PR_InitializeNetAddr(PR_IpAddrAny, 0, &server_addr);
//    server_addr.inet.family = PR_AF_INET;
//    server_addr.inet.port = PR_htons(SERVER_PORT);

    int index = 0;
    do {
        index = PR_EnumerateHostEnt(index, &host_ent, SERVER_PORT, &server_addr);
    } while (index != 0);
    if (index == -1) {
        die("Error enumerating host entities");
    } else if (index == 0) {
        die("No valid host entity found");
    }

    log("found valid host entity");

    if (PR_FAILURE == PR_Connect(ssl_sock, &server_addr, PR_INTERVAL_NO_TIMEOUT)) {
        die("Error connecting to server");
    }

    log("connected to host entity");

    const char *msg_buf = "Hello World";
    int msg_buf_len = (int) strlen(msg_buf)+1;
    if (0 > PR_Send(ssl_sock, msg_buf, msg_buf_len, 0, PR_INTERVAL_NO_TIMEOUT)) {
        die("Error sending Hello World");
    }

    log("sent message");

    PR_Close(ssl_sock);
    PR_Close(tcp_sock);
    NSS_Shutdown();

    log("shutting down");

    return 0;
}

//int main() {
//    // NSS initialisieren
//    NSS_Init("");
//
//    // SSL-Client erstellen
//    PRFileDesc* clientSocket = SSL_ImportFD(nullptr, PR_NewTCPSocket());
//    if (!clientSocket) {
//        fprintf(stderr, "Fehler beim Erstellen des Client-Sockets\n");
//        return 1;
//    }
//
//    // Mit dem Server verbinden
//    PRNetAddr serverAddr;
//    PR_InitializeNetAddr(PR_IpAddrAny, 0, &serverAddr);
//    serverAddr.inet.family = PR_AF_INET;
//    serverAddr.inet.port = PR_htons(443); // Setze den richtigen Port ein
//
//    PRStatus status = PR_Connect(clientSocket, &serverAddr, PR_INTERVAL_NO_TIMEOUT);
//    if (status != PR_SUCCESS) {
//        fprintf(stderr, "Fehler beim Verbinden mit dem Server\n");
//        return 1;
//    }
//
//    // Daten über die TLS-Verbindung senden/empfangen
//    // Implementiere hier deine Logik für die Datenübertragung
//
//    // Aufräumen
//    PR_Close(clientSocket);
//    NSS_Shutdown();
//
//    return 0;
//}
