#include <iostream>

#include "../include/nss.h"
#include "../include/ssl.h"
#include "../include/nspr.h"
#include "../include/pk11func.h"
#include "../include/sslexp.h"

#include "helpers.h"

#define DB_DIR "./pki"
#define SERVER_PORT 443

using namespace std;

PRBool my_SSLExtensionWriter(
        PRFileDesc *fd,
        SSLHandshakeType message,
        PRUint8 *data,
        unsigned int *len,
        unsigned int maxLen,
        void *arg
) {
    cout << "my_SSLExtensionWriter" << endl;

    if (message != ssl_hs_server_hello)
        return PR_FALSE;

    cout << "Attach extension" << endl;

    string hello_msg = "Hello from Server-Extension";
    if (hello_msg.length() > maxLen)
        die("Extension message too long.");
    strcpy((char *) data, hello_msg.c_str());
    *len = hello_msg.length();

    return PR_TRUE;
}

SECStatus my_SSLExtensionHandler(
        PRFileDesc *fd,
        SSLHandshakeType message,
        const PRUint8 *data,
        unsigned int len,
        SSLAlertDescription *alert,
        void *arg
) {
    printf("Received data: %.*s\n", len, data);
    return SECSuccess;
}

int main() {
    // must be called before any other NSS function
    PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

    // set callback retrieving the password
    PK11_SetPasswordFunc(passwd_callback);

    // set up NSS config; not idempotent, only call once
    NSS_Init(DB_DIR);

    // allow all ciphers permitted to export from the US
    NSS_SetExportPolicy();

    // create server session id cache, required if the application should handshake as a server
    // TODO: is this even applicable to a simple server? Does a simple server even handshake or does the client handshake and the server is being handshook???
    SSL_ConfigServerSessionIDCache(0, 0, 10, nullptr);

    log("NSS initialized");

    // create NSPR legal TCP socket
    PRFileDesc* listen_sock = PR_NewTCPSocket();
    if (!listen_sock) {
        die("Error creating TCP socket");
    }

    // bind listen sock to a specific port
    PRNetAddr listen_addr;
    listen_addr.inet.family = PR_AF_INET;
    listen_addr.inet.ip = PR_htonl(PR_INADDR_ANY);
    listen_addr.inet.port = PR_htons(SERVER_PORT);

    // create SSL socket from TCP socket
    listen_sock = SSL_ImportFD(nullptr, listen_sock);
    if (!listen_sock) {
        die("Error importing listen socket into SSL library");
    }

    // RATLS
    // check if the extension can be used by custom hooks
    unsigned int extension = 420; // SSLExtensionType

    SSLExtensionSupport sslExtensionSupport = ssl_ext_native_only;
    SSL_GetExtensionSupport(extension, &sslExtensionSupport);
    if (sslExtensionSupport != ssl_ext_none && sslExtensionSupport != ssl_ext_native)
        die("SSL extension number not permitted by NSS, is 'native only'");

    SSL_InstallExtensionHooks(listen_sock, extension, my_SSLExtensionWriter, nullptr, my_SSLExtensionHandler, nullptr);

    if (PR_Bind(listen_sock, &listen_addr)) {
        die("Error binding listen socket");
        PR_Close(listen_sock);
    }

    // configure listen sock with the server cert for handshakes
    // sockets created by PR_Accept on this socket inherit the configuration
    void *pwArg = SSL_RevealPinArg(listen_sock);
    CERTCertificate *cert = PK11_FindCertFromNickname("server", pwArg); // Nick: RootCA for testing purposes?
    if (cert == nullptr)
        die("PK11_FindCertFromNickname");
    SECKEYPrivateKey *privKey = PK11_FindKeyByAnyCert(cert, pwArg);
    if (privKey == nullptr)
        die("PK11_FindKeyByAnyCert");
    if (SECFailure == SSL_ConfigServerCert(listen_sock, cert, privKey, nullptr, 0)) { // 505 selfserv.c
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
        // the socket returned by PR_Accept is already a ssl socket and inherits from the listen socket
        PRFileDesc* ssl_sock = PR_Accept(listen_sock, &client_addr, PR_INTERVAL_NO_TIMEOUT);
        if (!ssl_sock) {
            die("Error accepting client connection");
            PR_Close(listen_sock);
        }

        log("accepted connection");

        // send hello world
        const string msg = "Hello from the server!\n";
        switch (PR_Send(ssl_sock, msg.c_str(), (int) msg.length(), 0, PR_INTERVAL_NO_TIMEOUT)) {
            case -1:
                diePRError("PR_Send");
                break;
            case 0:
                die("Error sending, network connection is closed");
                break;
        }

        // Read hello from client
        char buf[1024];
        memset(buf, 0, 1024);
        int bytes_read = PR_Recv(ssl_sock, buf, 1024, 0, PR_INTERVAL_NO_TIMEOUT);
        if (bytes_read == -1) {
            diePRError("Error receiving message");
        } else if (bytes_read == 0) {
            die("Error connection closed before receiving any bytes");
        }
        buf[bytes_read] = '\0';
        cout << "Message: " << buf << endl;

        PR_Close(ssl_sock);
    }

    PR_Close(listen_sock);
    NSS_Shutdown();

    log("shutting down");

    return 0;
}
