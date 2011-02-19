/*
 *   stunnel       Universal SSL tunnel
 *   Copyright (c) 1998-2006 Michal Trojnara <Michal.Trojnara@mirt.net>
 *                 All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include "common.h"

/**************************************** Network data structure */

#define MAX_HOSTS 16

typedef union sockaddr_union {
    struct sockaddr sa;
    struct sockaddr_in in;
#if defined(USE_IPv6)
    struct sockaddr_in6 in6;
#endif
} SOCKADDR_UNION;

typedef struct sockaddr_list {      /* list of addresses */
    SOCKADDR_UNION addr[MAX_HOSTS]; /* the list of addresses */
    u16 cur;                        /* current address for round-robin */
    u16 num;                        /* how many addresses are used */
} SOCKADDR_LIST;

/**************************************** Prototypes for stunnel.c */

extern volatile int num_clients;

void main_initialize(char *, char *);
void main_execute(void);
void stunnel_info(int);

/**************************************** Prototypes for log.c */

void log_open(void);
void log_close(void);
void s_log(int, const char *, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 2, 3)));
#else
    ;
#endif
void log_raw(const char *, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 1, 2)));
#else
    ;
#endif
void ioerror(const char *);
void sockerror(const char *);
void log_error(int, int, const char *);
char *my_strerror(int);
    
/**************************************** Prototypes for pty.c */
/* Based on Public Domain code by Tatu Ylonen <ylo@cs.hut.fi>  */

int pty_allocate(int *, int *, char *, int);
#if 0
void pty_release(char *);
void pty_make_controlling_tty(int *, char *);
#endif

/**************************************** Prototypes for ssl.c */

typedef enum {
    COMP_NONE, COMP_ZLIB, COMP_RLE
} COMP_TYPE;

extern int cli_index;

void ssl_init(void);
void ssl_configure(void);
#ifdef HAVE_OSSL_ENGINE_H
void open_engine(const char *);
void ctrl_engine(const char *, const char *);
void close_engine(void);
#endif

/**************************************** Prototypes for options.c */

typedef struct {
        /* some data for SSL initialization in ssl.c */
    COMP_TYPE compression;                               /* compression type */
    char *egd_sock;                       /* entropy gathering daemon socket */
    char *rand_file;                                /* file with random data */
    int random_bytes;                       /* how many random bytes to read */

        /* some global data for stunnel.c */
#ifndef USE_WIN32
#ifdef HAVE_CHROOT
    char *chroot_dir;
#endif
    unsigned long dpid;
    char *pidfile;
    char *setuid_user;
    char *setgid_group;
#endif

        /* Win32 specific data for gui.c */
#if defined(USE_WIN32) && !defined(_WIN32_WCE)
    char *win32_service;
#endif

        /* logging-support data for log.c */
    int debug_level;                               /* debug level for syslog */
#ifndef USE_WIN32
    int facility;                               /* debug facility for syslog */
#endif
    char *output_file;

        /* on/off switches */
    struct {
        unsigned int foreground:1;
        unsigned int syslog:1;                              /* log to syslog */
        unsigned int rand_write:1;                    /* overwrite rand_file */
#ifdef USE_WIN32
        unsigned int taskbar:1;                   /* enable the taskbar icon */
#endif
    } option;
} GLOBAL_OPTIONS;

extern GLOBAL_OPTIONS options;

typedef struct local_options {
    SSL_CTX *ctx; /*  SSL context */
    struct local_options *next; /* next node in the services list */
    char *servname; /* service name for logging & permission checking */
    SSL_SESSION *session; /* Recently used session */
    char local_address[IPLEN]; /* Dotted-decimal address to bind */

        /* service-specific data for ctx.c */
    char *ca_dir;                              /* directory for hashed certs */
    char *ca_file;                       /* file containing bunches of certs */
    char *crl_dir;                              /* directory for hashed CRLs */
    char *crl_file;                       /* file containing bunches of CRLs */
    char *cipher_list;
    char *cert;                                             /* cert filename */
    char *key;                               /* pem (priv key/cert) filename */
    long session_timeout;
    int verify_level;
    int verify_use_only_my;
    long ssl_options;

        /* service-specific data for client.c */
    int fd;        /* file descriptor accepting connections for this service */
    char *execname, **execargs; /* program name and arguments for local mode */
    SOCKADDR_LIST local_addr, remote_addr;
    SOCKADDR_LIST source_addr;
    char *username;
    char *remote_address;
    int timeout_busy; /* Maximum waiting for data time */
    int timeout_close; /* Maximum close_notify time */
    int timeout_connect; /* Maximum connect() time */
    int timeout_idle; /* Maximum idle connection time */

        /* protocol name for protocol.c */
    char *protocol;
    char *protocol_host;
    char *protocol_credentials;

        /* on/off switches */
    struct {
        unsigned int cert:1;
        unsigned int client:1;
        unsigned int delayed_lookup:1;
        unsigned int accept:1;
        unsigned int remote:1;
#ifndef USE_WIN32
        unsigned int program:1;
        unsigned int pty:1;
        unsigned int transparent:1;
#endif
    } option;
} LOCAL_OPTIONS;

extern LOCAL_OPTIONS local_options;

typedef enum {
    TYPE_NONE, TYPE_FLAG, TYPE_INT, TYPE_LINGER, TYPE_TIMEVAL, TYPE_STRING
} VAL_TYPE;

typedef union {
    int            i_val;
    long           l_val;
    char           c_val[16];
    struct linger  linger_val;
    struct timeval timeval_val;
} OPT_UNION;

typedef struct {
    char *opt_str;
    int  opt_level;
    int  opt_name;
    VAL_TYPE opt_type;
    OPT_UNION *opt_val[3];
} SOCK_OPT;

void parse_config(char *, char *);

/**************************************** Prototypes for ctx.c */

SSL_CTX *context_init(LOCAL_OPTIONS *);
void context_free(SSL_CTX *);
void sslerror(char *);

/**************************************** Prototypes for network.c */

#define MAX_FD 64

typedef struct {
#ifdef USE_POLL
    struct pollfd ufds[MAX_FD];
    unsigned int nfds;
#else
    fd_set irfds, iwfds, orfds, owfds;
    int max;
#endif
} s_poll_set;

void s_poll_zero(s_poll_set *);
void s_poll_add(s_poll_set *, int, int, int);
int s_poll_canread(s_poll_set *, int);
int s_poll_canwrite(s_poll_set *, int);
int s_poll_wait(s_poll_set *, int);

#ifndef USE_WIN32
int signal_pipe_init(void);
void child_status(void);  /* dead libwrap or 'exec' process detected */
#endif
int set_socket_options(int, int);
int alloc_fd(int);

/**************************************** Prototypes for client.c */

typedef struct {
    int fd; /* File descriptor */
    int rd; /* Open for read */
    int wr; /* Open for write */
    int is_socket; /* File descriptor is a socket */
} FD;

typedef struct {
    LOCAL_OPTIONS *opt;
    char accepting_address[IPLEN], connecting_address[IPLEN]; /* text */
    SOCKADDR_LIST peer_addr; /* Peer address */
    FD local_rfd, local_wfd; /* Read and write local descriptors */
    FD remote_fd; /* Remote file descriptor */
    SSL *ssl; /* SSL Connection */
    SOCKADDR_LIST bind_addr; /* IP for explicit local bind or transparent proxy */
    unsigned long pid; /* PID of local process */
    int fd; /* Temporary file descriptor */
    jmp_buf err;

    char sock_buff[BUFFSIZE]; /* Socket read buffer */
    char ssl_buff[BUFFSIZE]; /* SSL read buffer */
    int sock_ptr, ssl_ptr; /* Index of first unused byte in buffer */
    FD *sock_rfd, *sock_wfd; /* Read and write socket descriptors */
    FD *ssl_rfd, *ssl_wfd; /* Read and write SSL descriptors */
    int sock_bytes, ssl_bytes; /* Bytes written to socket and ssl */
    s_poll_set fds; /* File descriptors */
} CLI;

extern int max_clients;
#ifndef USE_WIN32
extern int max_fds;
#endif

void *alloc_client_session(LOCAL_OPTIONS *, int, int);
void *client(void *);

/**************************************** Prototypes for network.c */

void write_blocking(CLI *, int fd, u8 *, int);
void read_blocking(CLI *, int fd, u8 *, int);
void fdputline(CLI *, int, char *);
void fdgetline(CLI *, int, char *);
/* descriptor versions of fprintf/fscanf */
int fdprintf(CLI *, int, const char *, ...)
#ifdef __GNUC__
       __attribute__ ((format (printf, 3, 4)));
#else
       ;
#endif
int fdscanf(CLI *, int, const char *, char *)
#ifdef __GNUC__
       __attribute__ ((format (scanf, 3, 0)));
#else
       ;
#endif

/**************************************** Prototype for protocol.c */

void negotiate(CLI *c);

/**************************************** Prototypes for resolver.c */

int name2addrlist(SOCKADDR_LIST *, char *, char *);
int hostport2addrlist(SOCKADDR_LIST *, char *, char *);
char *s_ntop(char *, SOCKADDR_UNION *);

/**************************************** Prototypes for sthreads.c */

typedef enum {
    CRIT_KEYGEN, CRIT_INET, CRIT_CLIENTS, CRIT_WIN_LOG, CRIT_SESSION,
    CRIT_SECTIONS
} SECTION_CODE;

void enter_critical_section(SECTION_CODE);
void leave_critical_section(SECTION_CODE);
void sthreads_init(void);
unsigned long stunnel_process_id(void);
unsigned long stunnel_thread_id(void);
int create_client(int, int, void *, void *(*)(void *));
#ifdef USE_UCONTEXT
typedef struct CONTEXT_STRUCTURE {
    char stack[STACK_SIZE];
    unsigned long id;
    ucontext_t ctx;
    s_poll_set *fds;
    int ready; /* number of ready file descriptors */
    time_t finish; /* when to finish poll() for this context */
    struct CONTEXT_STRUCTURE *next; /* next context on a list */
} CONTEXT;
extern CONTEXT *ready_head, *ready_tail;
extern CONTEXT *waiting_head, *waiting_tail;
#endif
#ifdef _WIN32_WCE
int _beginthread(void (*)(void *), int, void *);
void _endthread(void);
#endif
#ifdef DEBUG_STACK_SIZE
void stack_info(int);
#endif

/**************************************** Prototypes for gui.c */

#ifdef USE_WIN32
void win_log(char *);
void exit_stunnel(int);
int pem_passwd_cb(char *, int, int, void *);

#ifndef _WIN32_WCE
typedef int (CALLBACK * GETADDRINFO) (const char *,
    const char *, const struct addrinfo *, struct addrinfo **);
typedef void (CALLBACK * FREEADDRINFO) (struct addrinfo FAR *);
typedef int (CALLBACK * GETNAMEINFO) (const struct sockaddr *, socklen_t,
    char *, size_t, char *, size_t, int);
extern GETADDRINFO s_getaddrinfo;
extern FREEADDRINFO s_freeaddrinfo;
extern GETNAMEINFO s_getnameinfo;
#endif /* ! _WIN32_WCE */
#endif /* USE_WIN32 */

/**************************************** Prototypes for file.c */

typedef struct disk_file {
#ifdef USE_WIN32
    HANDLE fh;
#else
    int fd;
#endif
    /* the inteface is prepared to easily implement buffering if needed */
} DISK_FILE;

#ifndef USE_WIN32
DISK_FILE *file_fdopen(int);
#endif
DISK_FILE *file_open(char *, int);
void file_close(DISK_FILE *);
int file_getline(DISK_FILE *, char *, int);
int file_putline(DISK_FILE *, char *);

#ifdef USE_WIN32
LPTSTR str2tstr(const LPSTR);
LPSTR tstr2str(const LPTSTR);
#endif

#endif /* defined PROTOTYPES_H */

/* End of prototypes.h */
