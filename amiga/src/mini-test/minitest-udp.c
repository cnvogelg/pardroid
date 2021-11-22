#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <libraries/bsdsocket.h>
#include <proto/bsdsocket.h>

#include "autoconf.h"
#include "debug.h"
#include "timer.h"

struct Library *SocketBase;

#define MYPORT 1234

int dosmain(void)
{
	SocketBase = OpenLibrary("bsdsocket.library",0);
    if(SocketBase != NULL) {

        /* resolve host */
        struct hostent *he;
        he = gethostbyname("localhost");
        if(he != NULL) {

            struct sockaddr_in their_addr;

            their_addr.sin_family = AF_INET;      /* host byte order */
            their_addr.sin_port = htons(MYPORT);  /* short, network byte order */
            their_addr.sin_addr = *((struct in_addr *)he->h_addr);
            memset(&(their_addr.sin_zero), 0, 8);     /* zero the rest of the struct */

            // open socket
            int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if(sockfd < 0) {
                PutStr("Error in 'socket'\n");
            } else {

                PutStr("sending...\n");

                char *msg = "hello, world!\n";
                int num = sendto(sockfd, msg, strlen(msg), MSG_WAITALL,
                    (struct sockaddr *)&their_addr, sizeof(their_addr));
                Printf("result: %ld\n", num);

                PutStr("receiving...\n");
                char buffer[256];
                socklen_t peer_len;
                struct sockaddr_in peer_addr;
                num = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *)&peer_addr, &peer_len);
                Printf("result: %ld\n", num);

                PutStr("OK!\n");

            }

            // close socket
            CloseSocket(sockfd);
        } else {
            PutStr("can't resolve host!\n");
        }
    } else {
        PutStr("Error opening 'bsdsocket.library'!\n");
    }

    return 0;
}
