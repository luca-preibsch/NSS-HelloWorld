#include <iostream>

#include "../../include/nss.h"
#include "../../include/ssl.h"
#include "../../include/nspr.h"
#include "../../include/pk11func.h"

#define DB_DIR "./pki"
#define SERVER_PORT 12345

using namespace std;

void die(const string& error_msg) {
    cout << error_msg << endl;
    exit(EXIT_FAILURE);
}

void log(const string& msg) {
    cout << msg << endl;
}

void diePRError(const char* error_msg) {
    PRErrorCode errorCode = PR_GetError();
    const char *errString = PR_ErrorToString(errorCode, PR_LANGUAGE_I_DEFAULT);

    fprintf(stderr, "selfserv: %s returned error %d:\n%s\n",
            error_msg, errorCode, errString);
    exit(EXIT_FAILURE);
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
    PK11_SetPasswordFunc(passwd_callback); // now using a db without a password

    // set up NSS config; not idempotent, only call once
    NSS_Init(DB_DIR);

    // allow all ciphers permitted to export from the US
    NSS_SetExportPolicy();
//    enableAllCiphers();

    // create server session id cache, required if the application should handshake as a server
    // TODO: is this even applicable to a simple server? Does a simple server even handshake or does the client
    // handshake and the server is handshaken???
    SSL_ConfigServerSessionIDCache(NULL, 0, 10, NULL);

    log("NSS initialized");

    // create NSPR legal TCP socket
    PRFileDesc* listen_sock = PR_NewTCPSocket();
    if (!listen_sock) {
        die("Error creating TCP socket");
    }

    // bind listen sock to a specific port
    PRNetAddr listen_addr;
//    PR_InitializeNetAddr()
    listen_addr.inet.family = PR_AF_INET;
    listen_addr.inet.ip = PR_INADDR_ANY;
    listen_addr.inet.port = PR_htons(SERVER_PORT);

    // TODO: needed for listen socket?
    // create SSL socket from TCP socket
    listen_sock = SSL_ImportFD(nullptr, listen_sock);
    if (!listen_sock) {
        die("Error importing listen socket into SSL library");
    }

    if (PR_Bind(listen_sock, &listen_addr)) {
        die("Error binding listen socket");
        PR_Close(listen_sock);
    }

    // configure listen sock for handshakes, sockets created by PR_Accept on this socket inherit the configuration
    /*
        A pointer to application data for the password callback function. This pointer is
        set with SSL_SetPKCS11PinArg during SSL configuration. To retrieve its current value, use
        SSL_RevealPinArg. PK11_SetPasswordFunc
     */
    void *pwArg = SSL_RevealPinArg(listen_sock);
    CERTCertificate *cert = PK11_FindCertFromNickname("server", pwArg); // Nick: RootCA for testing purposes?
    if (cert == NULL)
        die("PK11_FindCertFromNickname");
    SECKEYPrivateKey *privKey = PK11_FindKeyByAnyCert(cert, pwArg);
    if (privKey == NULL)
        die("PK11_FindKeyByAnyCert");
    if (SECFailure == SSL_ConfigServerCert(listen_sock, cert, privKey, NULL, 0)) { // 505 selfserv.c
        diePRError("SSL_ConfigServerCert");
    }

    // start to listen on the socket
    if (PR_Listen(listen_sock, 1)) {
        die("Error starting to listen for listen socket");
        PR_Close(listen_sock);
    }

    log("started listening");

    for (;;) {
        PRNetAddr client_addr;
        PRFileDesc* tcp_sock = PR_Accept(listen_sock, &client_addr, PR_INTERVAL_NO_TIMEOUT);
        if (!tcp_sock) {
            die("Error accepting client connection");
            PR_Close(listen_sock);
        }

        log("accepted connection");

        // create SSL socket from TCP socket
        PRFileDesc* ssl_sock = SSL_ImportFD(nullptr, tcp_sock);
        if (!ssl_sock) {
            die("Error importing TCP socket into SSL library");
            PR_Close(listen_sock);
            PR_Close(tcp_sock);
        }

        // Read "Hello World!"
        int buf_len = strlen("Hello World!") + 1; // +1 for /0
        char buf[buf_len];
        memset(buf, 0, buf_len);
        int bytes_read = PR_Read(tcp_sock, buf, buf_len); // ssl_sock
        if (bytes_read == -1) {
            diePRError("Error receiving Hello World!");
        } else if (bytes_read == 0) {
            die("Error connection closed before receiving bytes");
        }
        cout << "Message: " << buf << endl;

        PR_Close(tcp_sock);
    }

    PR_Close(listen_sock);
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
