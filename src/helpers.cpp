#include <iostream>

#include "../include/nss.h"
#include "../include/ssl.h"
#include "../include/nspr.h"

#include "helpers.h"

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

    fprintf(stderr, "%s - error %d:\n%s\n",
            error_msg, errorCode, errString);
    exit(EXIT_FAILURE);
}

char *passwd_callback(PK11SlotInfo *slot, PRBool retry, void *arg) {
    string passwd;
    cout << "Please enter the password of your NSS certificate database: " << flush;
    getline(cin, passwd);
    return PL_strdup(passwd.c_str());
}