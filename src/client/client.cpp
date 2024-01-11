#include <iostream>

#include "../../include/nss.h"
#include "../../include/ssl.h"
#include "../../include/nspr.h"
#include "../../include/pk11func.h"

#define DB_DIR "./client_db"
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

void diePRError(const string& error_msg) {
    // TODO receive error correctly
    PRErrorCode errorCode = PR_GetError();
    PRInt32 errorTextLength = PR_GetErrorTextLength();
    char error_buf[errorTextLength + 1];
    PR_GetErrorText(error_buf);
    cout << "ErrorCode " << errorCode << ": " << error_buf << endl;
    die(error_msg);
}

void enableAllCiphers() {
    const PRUint16 *cipherSuites = SSL_ImplementedCiphers;
    int i = SSL_NumImplementedCiphers;
    SECStatus rv;

    while (--i >= 0) {
        PRUint16 suite = cipherSuites[i];
        rv = SSL_CipherPrefSetDefault(suite, PR_TRUE);
        if (rv != SECSuccess) {
            printf("SSL_CipherPrefSetDefault rejected suite 0x%04x (i = %d)\n",
                   suite, i);
        }
    }
}

char *passwd_callback(PK11SlotInfo *slot, PRBool retry, void *arg) {
    char *passwd = "nss";
    return PL_strdup(passwd);
}

int main() {
    // must be called before any other NSS function
    PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

    // set callback retrieving the password
    PK11_SetPasswordFunc(passwd_callback);

    // set up NSS config; not idempotent, only call once
    NSS_Init(DB_DIR);

//    enableAllCiphers();
    // allow all ciphers permitted to export from the US
    NSS_SetExportPolicy();

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

    // TODO set hostname, is the hostname set in the certificate the one needed?
    SSL_SetURL(ssl_sock, "myco.mcom.org");

    log("imported TCP Socket into NSS");

    // TODO enumerate hosts not required?
//    char buf[PR_NETDB_BUF_SIZE];
//    PRHostEnt host_ent;
//    if (PR_FAILURE == PR_GetHostByName(HOSTNAME, buf, PR_NETDB_BUF_SIZE, &host_ent)) {
//        die("Error for GetHostByName");
//    }
//
//    log("got host by name");
//    PRNetAddr server_addr;
//    int index = 0;
//    for (;;) {
//        index = PR_EnumerateHostEnt(index, &host_ent, SERVER_PORT, &server_addr);
//        if (index == -1) {
//            die("Error enumerating host entities");
//        }
//        if (index == 0) {
//            die("Found no valid host entity");
//        }
//
//        if (PR_SUCCESS == PR_Connect(ssl_sock, &server_addr, PR_INTERVAL_NO_TIMEOUT)) {
//            break;
////            die("Error connecting to server");
//        }
//    }

    PRNetAddr srv_addr;
//    PR_InitializeNetAddr()
    srv_addr.inet.family = PR_AF_INET;
    srv_addr.inet.ip = inet_addr("127.0.0.1");
    srv_addr.inet.port = PR_htons(SERVER_PORT);

    if (PR_SUCCESS != PR_Connect(ssl_sock, &srv_addr, PR_INTERVAL_NO_TIMEOUT)) {
        die("Error connecting to server");
    }

    log("connected to host entity");

    const char *msg_buf = "Hello World";
    int msg_buf_len = (int) strlen(msg_buf)+1;
    switch (PR_Send(ssl_sock, msg_buf, msg_buf_len, 0, PR_INTERVAL_NO_TIMEOUT)) {
        case -1:
            diePRError("PR_Send");
            break;
        case 0:
            die("Error sending, network connection is closed");
            break;
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
