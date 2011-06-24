/******************************************************************************
***N***N**Y***Y**X***X*********************************************************
***NN**N***Y*Y****X*X**********************************************************
***N*N*N****Y******X***********************************************************
***N**NN****Y*****X*X************************ Make Date  : Jul 01, 2007 *******
***N***N****Y****X***X*********************** Last Update: Jul 01, 2007 *******
******************************************************************************/
#ifndef K_SOCKET
#define K_SOCKET


//#define FD_SETSIZE 100

/*************************************************
 Dependencies ************************************
*************************************************/
#if defined(linux)

#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define TIMEVAL timeval

#else

#include "winsock.h"

#define ioctl ioctlsocket

#endif

#include "nSTL.h"

/*************************************************
 Sockets Class ***********************************
*************************************************/

class k_socket {

protected:
public:
    unsigned short port;
    SOCKET sock;
    bool has_data_waiting;
    sockaddr_in addr;
    static slist<k_socket*,FD_SETSIZE> list;
    static SOCKET ndfs;
    static fd_set sockets;
public:
    k_socket();
    ~k_socket();
    virtual int clone(k_socket * remote);
    void close();
    virtual bool initialize(int param_port, int minbuffersize = 32 * 1024);
    virtual bool set_address(const char * cp, const u_short hostshort);
    virtual bool set_addr(sockaddr_in * arg_addr);
    virtual bool set_aport(int port);
    virtual bool send(char * buf, int len);
    virtual bool check_recv (char* buf, int * len, bool leave_in_queue, sockaddr_in* addrp);
    virtual bool has_data();
    virtual int get_port();
	char* to_string(char *buf);
    static bool check_sockets(int secs, int ms);
    static bool Initialize();
    static void Cleanup();
};

#endif
