#include <vector>
#include <algorithm>
using std::swap;

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "cube.h"

int main(int argc, char **argv)
{
    //Generate key pair: genkey <seed>
    if(argc == 2)
    {
        vector<char> privkey, pubkey;
        genprivkey(argv[1], privkey, pubkey);
        printf("private key: %s\n", privkey.data());
        printf("public key: %s\n", pubkey.data());
        return EXIT_SUCCESS;
    }
    //Print yes/no to match pubkey with privkey: genkey <pubkey> <privkey>
    else if(argc == 3)
    {
        vector<char> pubkey;
        genpubkey(argv[2], pubkey);
        printf("%s\n", !strcmp(argv[1], pubkey.data()) ? "yes" : "no");
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

