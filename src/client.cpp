#include <iostream>
#include <map>

#include "../include/nss.h"
#include "../include/ssl.h"
#include "../include/nspr.h"
#include "../include/pk11func.h"
#include "../include/sslexp.h"

#include "helpers.h"

#define DB_DIR "./pki"
#define HOSTNAME "helloworld.example.com"
#define SERVER_PORT 443

using namespace std;
//typedef map<SECItem*, string> NonceMap;
typedef map<PRFileDesc*, string> NonceMap; // for now try with the fd

NonceMap nonceMap;

PRBool my_SSLExtensionWriter(
        PRFileDesc *fd,
        SSLHandshakeType message,
        PRUint8 *data,
        unsigned int *len,
        unsigned int maxLen,
        void *arg
) {
    if (message != ssl_hs_client_hello)
        return PR_FALSE;

    log("my_SSLExtensionWriter");

    // Send RA request including a nonce
    string nonce = generateNonce(16);
    string hello_msg = "RA_REQ:" + nonce;

//    SECItem* sessionId = SSL_GetSessionID(fd);
//    nonceMap.insert(pair<SECItem*, string>(sessionId, nonce));
    nonceMap.insert(pair<PRFileDesc*, string>(fd, nonce));

    if (hello_msg.length() > maxLen)
        die("Extension message too long.");
    strcpy((char *) data, hello_msg.c_str());
    *len = hello_msg.length();

    cout << "Sent nonce: " << nonce << endl;

    return PR_TRUE;
}

SECStatus my_SSLExtensionHandler(
        PRFileDesc *fd,
        SSLHandshakeType messageType,
        const PRUint8 *data,
        unsigned int len,
        SSLAlertDescription *alert,
        void *arg
) {
    if (messageType != ssl_hs_certificate)
        return SECSuccess;

//    SECItem* sessionId = SSL_GetSessionID(fd);
    if (!nonceMap.contains(fd))
        return SECSuccess;

    string message((char*) data, len);  // TODO use C++ style cast
    const string raReqText = "RA_RES:";
    if (message.substr(0, raReqText.length()) != raReqText) // C++ String comparison
        return SECFailure;

    string nonce = message.substr(raReqText.length(), message.length());

    if (nonceMap[fd] != nonce)
        return SECFailure;

    return SECSuccess;
}

int main() {
    // must be called before any other NSS function
    PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

    // set callback retrieving the password
    PK11_SetPasswordFunc(passwd_callback); // now uses db without a password

    // set up NSS config; not idempotent, only call once
    NSS_Init(DB_DIR); // For HTTPS the client does not need any certs, only has to check the server cert?

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

    // RATLS
    // check if the extension can be used by custom hooks
    unsigned int extension = 420; // SSLExtensionType

    SSLExtensionSupport sslExtensionSupport = ssl_ext_native_only;
    SSL_GetExtensionSupport(extension, &sslExtensionSupport);
    if (sslExtensionSupport != ssl_ext_none && sslExtensionSupport != ssl_ext_native)
        die("SSL extension number not permitted by NSS, is 'native only'");

    SSL_InstallExtensionHooks(ssl_sock, extension, my_SSLExtensionWriter, nullptr, my_SSLExtensionHandler, nullptr);

    // set hostname
    SSL_SetURL(ssl_sock, HOSTNAME);

    log("imported TCP Socket into NSS");

    PRNetAddr srv_addr;
    srv_addr.inet.family = PR_AF_INET;
    srv_addr.inet.ip = inet_addr("127.0.0.1"); // TODO get ip through domain
    srv_addr.inet.port = PR_htons(SERVER_PORT);

    if (PR_SUCCESS != PR_Connect(ssl_sock, &srv_addr, PR_INTERVAL_NO_TIMEOUT)) {
        die("Error connecting to server");
    }

    log("connected to host entity");

    if (SSL_ForceHandshake(ssl_sock) != SECSuccess) {
        diePRError("SSL/TLS handshake failed");
    }

    log("forced handshake");

    // Read hello from server
    char buf[1024];
    memset(buf, 0, 1024);
    int bytes_read = PR_Recv(ssl_sock, buf, 1024, 0, PR_INTERVAL_NO_TIMEOUT);
    if (bytes_read == -1) {
        diePRError("Error receiving Hello World!");
    } else if (bytes_read == 0) {
        die("Error connection closed before receiving bytes");
    }
    cout << "Bytes read: " << bytes_read << " Message: " << buf << endl;

    // Send hello world
    const string msg = "Hello from the client!\n";
    switch (PR_Write(ssl_sock, msg.c_str(), (int) msg.length())) {
        case -1:
            diePRError("PR_Send");
            break;
        case 0:
            die("Error sending, network connection is closed");
            break;
    }

    log("sent message");

    PR_Close(ssl_sock);
    NSS_Shutdown();

    log("shutting down");

    return 0;
}
