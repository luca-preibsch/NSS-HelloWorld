using namespace std;

void die(const string& error_msg);
void log(const string& msg);
void diePRError(const char* error_msg);
char *passwd_callback(PK11SlotInfo *slot, PRBool retry, void *arg);