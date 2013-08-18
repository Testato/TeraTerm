/////////////////////////////////////////////////////////////////////////////
// CygTerm - yet another Cygwin console
// Copyright (C) 2000-2004 NSym.
//---------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License (GPL) as published by
// the Free Software Foundation; either version 2 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CygTerm - yet another Cygwin console
//
//   Using Cygwin with a terminal emulator.
//   Written by NSym.
//                         *** Web Pages ***
//  (English) http://www.dd.iij4u.or.jp/~nsym/cygwin/cygterm/index-e.html
// (Japanese) http://www.dd.iij4u.or.jp/~nsym/cygwin/cygterm/index.html
//
/////////////////////////////////////////////////////////////////////////////
// patch level 01 - support for "~/.cygtermrc" and "/etc/cygterm.conf"
//   Written by BabyDaemon. (babydamons@yahoo.co.jp)
//
//                         *** Web Pages ***
// (Japanese) http://www.dd.iij4u.or.jp/~nsym/cygwin/cygterm/index.html
// Sorry, Japanese web pages only, but I will try read English e-mail,
// If I recieved it.

static char Program[] = "CygTerm";
static char Version[] = "version 1.06_01 (2006/02/08)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <windows.h>
#include <pwd.h>

// PTY device name
//----------------
#define  DEVPTY  "/dev/ptmx"

// TCP port for TELNET
//--------------------
int port_start = 20000;  // default lowest port number
int port_range = 40;     // default number of ports

// command lines of a terminal-emulator and a shell
//-------------------------------------------------
char cmd_term[256] = "";
char cmd_shell[128] = "";

// TCP port for connection to another terminal application
//--------------------------------------------------------
int cl_port = 0;

// dumb terminal flag
//-------------------
bool dumb = false;

// terminal type & size
//---------------------
char term_type[41] = "";
struct winsize win_size = {0,0,0,0};

// additional env vars given to a shell
//-------------------------------------
struct sh_env_t {
    struct sh_env_t* next;
    char env[1];
} sh_env = {NULL, ""};

sh_env_t* sh_envp = &sh_env;

//================//
// message output //
//----------------//
void msg_print(char* msg)
{
    MessageBox(NULL, msg, Program, MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
}

//=========================//
// Win32-API error message //
//-------------------------//
void api_error(char* string = "")
{
    char msg[1024];
    char *ptr = msg;
    if (string != NULL)
        ptr += sprintf(ptr, "%s\n\n", string);
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        ptr, 256, NULL
    );
    msg_print(msg);
}

//=========================//
// C-runtime error message //
//-------------------------//
void c_error(char* string = "")
{
    char msg[1024];
    char *ptr = msg;
    if (string != NULL)
        ptr += sprintf(ptr, "%s\n\n", string);
    sprintf(ptr, "%s\n", strerror(errno));
    msg_print(msg);
}

//==================================//
// parse line in configuration file //
//----------------------------------//
void parse_cfg_line(char *buf)
{
    // "KEY = VALUE" format in each line.
    // skip leading/trailing blanks. KEY is not case-sensitive.
    char* p1;
    for (p1 = buf; isspace(*p1); ++p1);
    if (!isalpha(*p1)) {
        return; // comment line with non-alphabet 1st char
    }
    char* name = p1;
    for (++p1; isalnum(*p1) || *p1 == '_'; ++p1);
    char* p2;
    for (p2 = p1; isspace(*p2); ++p2);
    if (*p2 != '=') {
        return; // igonore line without '='
    }
    for (++p2; isspace(*p2); ++p2);
    char* val = p2;
    for (p2 += strlen(p2); isspace(*(p2-1)); --p2);
    *p1 = *p2 = 0;

    if (!strcasecmp(name, "TERM")) {
        // terminal emulator command line (host:%s, port#:%d)
        strncpy(cmd_term, val, sizeof(cmd_term)-1);
        cmd_term[sizeof(cmd_term)-1] = 0;
    }
    else if (!strcasecmp(name, "SHELL")) {
        // shell command line
        strncpy(cmd_shell, val, sizeof(cmd_shell)-1);
        cmd_shell[sizeof(cmd_shell)-1] = 0;
    }
    else if (!strcasecmp(name, "PORT_START")) {
        // minimum port# for TELNET
        port_start = atoi(val);
    }
    else if (!strcasecmp(name, "PORT_RANGE")) {
        // number of ports for TELNET
        port_range = atoi(val);
    }
    else if (!strcasecmp(name, "TERM_TYPE")) {
        // terminal type name (maybe overridden by TELNET negotiation.)
        strncpy(term_type, val, sizeof(term_type)-1);
        term_type[sizeof(term_type)-1] = 0;
    }
    else if (!strncasecmp(name, "ENV_", 4)) {
        // additional env vars given to a shell
        sh_env_t* e = (sh_env_t*)malloc(sizeof(sh_env_t)+strlen(val));
        if (e != NULL) {
            strcpy(e->env, val);
            e->next = NULL;
            sh_envp = (sh_envp->next = e);
        }
    }
    return;
}

//====================//
// load configuration //
//--------------------//
void load_cfg()
{
    // Windows system configuration file (.cfg) path
    char win_conf[MAX_PATH];

    // get cfg path from exe path
    if (GetModuleFileName(NULL, win_conf, MAX_PATH) <= 0) {
        return;
    }
    char* bs = strrchr(win_conf, '\\');
    if (bs == NULL) {
        return;
    }
    char* dot = strrchr(bs, '.');
    if (dot == NULL) {
        strcat(bs, ".cfg");
    } else {
        strcpy(dot, ".cfg");
    }

    static char sys_conf[] = "/etc/cygterm.conf";

    // user configuration file (~/.*rc) path
    static char usr_conf[MAX_PATH] = "";

    // auto generated configuration file path
    static char tmp_conf[MAX_PATH] = "/tmp/cygtermrc.XXXXXX";

    // get user name form Windows ENVIRONMENT,
    // and get /etc/passwd information by getpwnam(3) with USERNAME,
    // and generate temporary configuration file by mktemp(3).
    const char* username = getenv("USERNAME");
    if (username != NULL) {
        struct passwd* pw_ent = getpwnam(username);
        if (pw_ent != NULL) {
            strcpy(usr_conf, pw_ent->pw_dir);
            strcat(usr_conf, "/.");
            strcat(usr_conf, bs + 1);
            char* dot = strrchr(usr_conf, '.');
            if (dot == NULL) {
                strcat(bs, "rc");
            } else {
                strcpy(dot, "rc");
            }
        }
        mktemp(tmp_conf);
        FILE* fp = fopen(tmp_conf, "w");
        if (fp != NULL) {
            if (pw_ent != NULL) {
                fprintf(fp, "ENV_1=HOME=%s\n",  pw_ent->pw_dir);
                fprintf(fp, "ENV_2=USER=%s\n",  pw_ent->pw_name);
                fprintf(fp, "ENV_3=SHELL=%s\n", pw_ent->pw_shell);
            } else {
                fprintf(fp, "ENV_1=HOME=/home/%s\n", username);
                fprintf(fp, "ENV_2=USER=%s\n",       username);
            }
            fclose(fp);
        }
    }

    if (strcmp(usr_conf, "") == 0) {
        strcpy(usr_conf, "");
        strcpy(tmp_conf, "");
    }

    char *conf_path[] = { win_conf, sys_conf, tmp_conf, usr_conf };
    for (int i = 0; i < 4; i++) {
        // ignore empty configuration file path
        if (strcmp(conf_path[i], "") == 0) {
            continue;
        }
        // read each setting parameter
        FILE* fp;
        if ((fp = fopen(conf_path[i], "r")) == NULL) {
            continue;
        }
        char buf[BUFSIZ];
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            parse_cfg_line(buf);
        }
        fclose(fp);
    }

    // remove temporary configuration file, if it was generated.
    if (strcmp(tmp_conf, "") != 0) {
        unlink(tmp_conf);
    }
}

//=======================//
// commandline arguments //
//-----------------------//
void get_args(int argc, char** argv)
{
    for (++argv; *argv != NULL; ++argv) {
        if (!strcmp(*argv, "-t")) {             // -t <terminal emulator>
            if (*(argv+1) != NULL) {
                ++argv, strcpy(cmd_term, *argv);
            }
        }
        else if (!strcmp(*argv, "-p")) {        // -p <port#>
            if (*(argv+1) != NULL) {
                ++argv, cl_port = atoi(*argv);
            }
        }
        else if (!strcmp(*argv, "-dumb")) {     // -dumb
            dumb = true;
            strcpy(term_type, "dumb");
        }
        else if (!strcmp(*argv, "-s")) {        // -s <shell>
            if (*(argv+1) != NULL) {
                ++argv, strcpy(cmd_shell, *argv);
            }
        }
        else if (!strcmp(*argv, "-v")) {        // -v <additional env var>
            if (*(argv+1) != NULL) {
                ++argv;
                sh_env_t* e = (sh_env_t*)malloc(sizeof(sh_env_t)+strlen(*argv));
                if (e != NULL) {
                    strcpy(e->env, *argv);
                    e->next = NULL;
                    sh_envp = (sh_envp->next = e);
                }
            }
        }
    }
}

//=============================//
// terminal emulator execution //
//-----------------------------//
DWORD WINAPI term_thread(LPVOID)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    FillMemory(&si, sizeof(si), 0);
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;
    DWORD flag = 0;
    if (!CreateProcess(
         NULL, cmd_term, NULL, NULL, FALSE, flag, NULL, NULL, &si, &pi))
    {
        api_error(cmd_term);
        return 0;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

//============================-==========//
// thread creation for terminal emulator //
//---------------------------------------//
HANDLE exec_term()
{
    DWORD id;
    return CreateThread(NULL, 0, term_thread, NULL, 0, &id);
}

//=======================================//
// listener socket for TELNET connection //
//---------------------------------------//
int listen_telnet(u_short* port)
{
    int lsock;
    if ((lsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    int i;
    for (i = 0; i < port_range; ++i) { // find an unused port#
        addr.sin_port = htons(port_start + i);
        if (bind(lsock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            break;
        }
    }
    if (i == port_range) {
        shutdown(lsock, 2);
        close(lsock);
        return -1;
    }
    if (listen(lsock, 1) != 0) {
        shutdown(lsock, 2);
        close(lsock);
        return -1;
    }
    *port = addr.sin_port;
    return lsock;
}

//=============================//
// accept of TELNET connection //
//-----------------------------//
int accept_telnet(int lsock)
{
    fd_set rbits;
    FD_ZERO(&rbits);
    FD_SET(lsock, &rbits);
    struct timeval tm;
    tm.tv_sec = 5; // timeout 5 sec
    tm.tv_usec = 0;
    if (select(FD_SETSIZE, &rbits, 0, 0, &tm) <= 0) {
        return -1;
    }
    if (!FD_ISSET(lsock, &rbits)) {
        return -1;
    }
    int asock;
    struct sockaddr_in addr;
    int len = sizeof(addr);
    if ((asock = accept(lsock, (struct sockaddr *)&addr, &len)) < 0) {
        return -1;
    }
    if (getpeername(asock, (struct sockaddr *)&addr, &len) != 0) {
        shutdown(asock, 2);
        close(asock);
        return -1;
    }
    if (addr.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
        // reject it except local connection
        shutdown(asock, 2);
        close(asock);
        return -1;
    }
    return asock;
}

//============================//
// connect to specified port# //
//----------------------------//
int connect_client()
{
    int csock;
    if ((csock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(cl_port);
    if (connect(csock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(csock);
        return -1;
    }
    return csock;
}

//========================================//
// setup *argv[] from a string for exec() //
//----------------------------------------//
void get_argv(char **argv, int maxc, char *s)
{
    int esc, sq, dq;  // recognize (\) (') (") and tokenize
    int c, argc;
    char *p;
    esc = sq = dq = 0;
    for (argc = 0; argc < maxc-1; ++argc) {
        for ( ; isascii(*s) && isspace(*s); ++s);
        if (*s == 0) {
            break;
        }
        argv[argc] = p = s;
        while ((c = *s) != 0) {
            ++s;
            if (isspace(c) && !esc && !sq && !dq) {
                break;
            }
            if (c == '\'' && !esc && !dq) {
                sq ^= 1;
            } else if (c == '"' && !esc && !sq) {
                dq ^= 1;
            } else if (c == '\\' && !esc) {
                esc = 1;
            } else {
                esc = 0;
                *p++ = c;
            }
        }
        *p = 0;
    }
    // not to judge syntax errors
    // if (dq || sq || esc) { syntax error }
    // if (argc == maxc) { overflow }
    argv[argc] = NULL;
}

//=================//
// shell execution //
//-----------------//
int exec_shell(int* sh_pid)
{
    // open pty master
    int master;
    if ((master = open(DEVPTY, O_RDWR)) < 0) {
        return -1;
    }
    int pid;
    if ((pid = fork()) < 0) {
        return -1;
    }
    if (pid == 0) {
        // detach from control tty
        setsid();
        // open pty slave
        int slave;
        if ((slave = open(ptsname(master), O_RDWR)) < 0) {
            exit(0);
        }
        // stdio redirection
        while (slave <= 2) {
            if ((slave = dup(slave)) < 0) {
                exit(0);
            }
        }
        int fd;
        for (fd = 0; fd < 3; ++fd) {
            close(fd);
            dup(slave);
            fcntl(fd, F_SETFD, 0);
        }
        for (fd = 3; fd < getdtablesize(); ++fd) {
            if (fcntl(fd, F_GETFD) == 0) {
                close(fd);
            }
        }
        // set env vars
        if (*term_type != 0) {
            // set terminal type to $TERM
            char env_term[64];
            sprintf(env_term, "TERM=%s", term_type);
            putenv(env_term);
        }
        // set other additional env vars
        sh_env_t* e;
        for (e = sh_env.next; e != NULL; e = e->next) {
            putenv(e->env);
        }
        // chdir to home directory
        const char *home_dir = getenv("HOME");
        if (home_dir != NULL) {
            // ignore chdir(2) system-call error.
            chdir(home_dir);
        }
        // execute a shell
        char *argv[32];
        get_argv(argv, 32, cmd_shell);
        execv(argv[0], argv);
        // no error, exec() doesn't return
        c_error(argv[0]);
        exit(0);
    }
    *sh_pid = pid;
    return master;
}

//==================//
// i/o buffer class //
//------------------//
class IOBuf
{
private:
    int fd;
    u_char i_buf[4096];
    u_char o_buf[4096];
    int i_pos, i_len, o_pos;
public:
    IOBuf(int channel) : fd(channel), i_pos(0), i_len(0), o_pos(0) {}
    operator int() { return fd; }
    void ungetc() { --i_pos; }
    bool flush_in();
    bool getc(u_char*);
    bool nextc(u_char*);
    bool putc(u_char);
    bool flush_out();
};

// read bytes into input buffer
//-----------------------------
bool IOBuf::flush_in()
{
    if ((i_len = read(fd, i_buf, sizeof(i_buf))) <= 0)
        return false;
    i_pos = 0;
    return true;
}

// get 1 char from input buffer
//-----------------------------
inline bool IOBuf::getc(u_char* c)
{
    if (i_pos == i_len) return false;
    *c = i_buf[i_pos++];
    return true;
}

// get next 1 char from input buffer
//----------------------------------
inline bool IOBuf::nextc(u_char* c)
{
    if (i_pos == i_len)
        if (!flush_in()) return false;
    *c = i_buf[i_pos++];
    return true;
}

// put 1 char to output buffer
//----------------------------
inline bool IOBuf::putc(u_char c)
{
    if (o_pos == sizeof(o_buf))
        if (!flush_out()) return false;
    o_buf[o_pos++] = c;
    return true;
}

// write bytes from output buffer
//-------------------------------
bool IOBuf::flush_out()
{
    int n;
    for (int i = 0; i < o_pos; i += n) {
        if ((n = write(fd, o_buf+i, o_pos-i)) <= 0) return false;
    }
    o_pos = 0;
    return true;
}

//=========================//
// TELNET command handling //  (see RFC854 TELNET PROTOCOL SPECIFICATION)
//-------------------------//
enum { nIAC=255, nWILL=251, nWONT=252, nDO=253, nDONT=254 };
enum { sSEND=1, sIS=0, sSB=250, sSE=240 };
enum { oTERM=24, oNAWS=31 };

bool c_will_term = false;
bool c_will_naws = false;

u_char telnet_cmd(IOBuf* te)
{
    u_char cmd, c;
    te->nextc(&cmd);
    if (cmd == sSB) {
        te->nextc(&c);
        // accept terminal type request
        if (c == oTERM) {                      // "SB TERM
            te->nextc(&c);                     //     IS
            u_char* p = (u_char*)term_type;
            te->nextc(p);                      //     TERMINAL-TYPE
            while (*p != nIAC) {
                if (isupper(*p)) *p = _tolower(*p);
                ++p; te->nextc(p);
            }
            *p = 0;
            te->nextc(&c);                     //     IAC SE"
            return (u_char)oTERM;
        }
        // accept terminal size request
        if (c == oNAWS) {                      // "SB NAWS
            u_short col, row;
            te->nextc((u_char*)&col);
            te->nextc((u_char*)&col+1);        //     00 00 (cols)
            te->nextc((u_char*)&row);
            te->nextc((u_char*)&row+1);        //     00 00 (rows)
            te->nextc(&c);
            te->nextc(&c);                     //     TAC SE"
            win_size.ws_col = ntohs(col);
            win_size.ws_row = ntohs(row);
            return (u_char)oNAWS;
        }
        while (c != nIAC) te->nextc(&c);       // "... IAC SE"
        te->nextc(&c);
    }
    else if (cmd == nWILL || cmd == nWONT || cmd == nDO || cmd == nDONT) {
        u_char c;
        te->nextc(&c);
        if (cmd == nWILL && c == oTERM)        // "WILL TERM"
            c_will_term = true;
        else if (cmd == nWILL && c == oNAWS)   // "WILL NAWS"
            c_will_naws = true;
    }
    return cmd;
}

//============================//
// TELNET initial negotiation //
//----------------------------//
void telnet_nego(int te_sock)
{
    IOBuf te = te_sock;
    u_char c;

    // start terminal type negotiation
    // IAC DO TERMINAL-TYPE
    te.putc(nIAC); te.putc(nDO); te.putc(oTERM);
    te.flush_out();
    te.nextc(&c);
    if (c != nIAC) {
        te.ungetc();
        return;
    }
    (void)telnet_cmd(&te);
    if (c_will_term) {
        // terminal type sub-negotiation
        // IAC SB TERMINAL-TYPE SEND IAC SE
        te.putc(nIAC); te.putc(sSB); te.putc(oTERM);
        te.putc(sSEND); te.putc(nIAC); te.putc(sSE);
        te.flush_out();
        // accept terminal type response
        te.nextc(&c);
        if (c != nIAC) {
            te.ungetc();
            return;
        }
        (void)telnet_cmd(&te);
    }

    // start terminal size negotiation
    // IAC DO WINDOW-SIZE
    te.putc(nIAC); te.putc(nDO); te.putc(oNAWS);
    te.flush_out();
    te.nextc(&c);
    if (c != nIAC) {
        te.ungetc();
        return;
    }
    (void)telnet_cmd(&te);
    if (c_will_naws) {
        // accept terminal size response
        te.nextc(&c);
        if (c != nIAC) {
            te.ungetc();
            return;
        }
        (void)telnet_cmd(&te);
    }
}

//=============================================//
// relaying of a terminal emulator and a shell //
//---------------------------------------------//
void telnet_session(int te_sock, int sh_pty)
{
    IOBuf te = te_sock;
    IOBuf sh = sh_pty;
    fd_set rtmp, rbits;
    FD_ZERO(&rtmp);
    FD_SET(te, &rtmp);
    FD_SET(sh, &rtmp);
    u_char c;
    int cr = 0;
    for (;;) {
        rbits = rtmp;
        if (select(FD_SETSIZE, &rbits, 0, 0, 0) <= 0) {
            break;
        }
        if (FD_ISSET(sh, &rbits)) {
            // send data from a shell to a terminal
            if (sh.flush_in() == false) {
                break;
            }
            while (sh.getc(&c) == true) {
                if (c == nIAC) {
                    // escape a TELNET IAC char
                    te.putc(c);
                }
                te.putc(c);
            }
            if (te.flush_out() == false) {
                break;
            }
            continue;  // give priority to data from a shell
        }
        if (FD_ISSET(te, &rbits)) {
            // send data from a terminal to a shell
            if (te.flush_in() == false) {
                break;
            }
            while (te.getc(&c) == true) {
                if (c == nIAC && !dumb) {
                    u_char cmd = telnet_cmd(&te) ;
                    if (cmd == oNAWS) {
                        // resize pty by terminal size change notice
                        ioctl(sh_pty, TIOCSWINSZ, &win_size);
                        continue;
                    }
                    if (cmd != nIAC) {
                        continue;
                    }
                } else if (c == '\r') {
                    cr = 1;
                } else if (c == '\n' || c == '\0') {
                    if (cr) {  // do not send LF or NUL just after CR
                        cr = 0;
                        continue;
                    }
                } else {
                    cr = 0;
                }
                sh.putc(c);
            }
            if (sh.flush_out() == false) {
                break;
            }
        }
    }
}

//=========================================================//
// connection of TELNET terminal emulator and Cygwin shell //
//---------------------------------------------------------//
int main(int argc, char** argv)
{
    int listen_sock = -1;
    u_short listen_port;
    int te_sock = -1;
    int sh_pty = -1;
    HANDLE hTerm = NULL;
    int sh_pid;

    // load configuration
    load_cfg();

    // read commandline arguments
    get_args(argc, argv);

    if (cmd_shell[0] == 0) {
        msg_print("missing shell");
        return 0;
    }
    if (cmd_term[0] == 0 && cl_port <= 0) {
        msg_print("missing terminal");
        return 0;
    }

    // terminal side connection
    if (cl_port > 0) {
        // connect to the specified TCP port
        if ((te_sock = connect_client()) < 0) {
            goto cleanup;
        }
    } else {
        // prepare a TELNET listener socket
        if ((listen_sock = listen_telnet(&listen_port)) < 0) {
            goto cleanup;
        }
        in_addr addr;
        addr.s_addr = htonl(INADDR_LOOPBACK);
        char tmp[128];
        sprintf(tmp, cmd_term, inet_ntoa(addr), (int)ntohs(listen_port));
        strcpy(cmd_term, tmp);

        // execute a terminal emulator
        if ((hTerm = exec_term()) == NULL) {
            goto cleanup;
        }
        // accept connection from the terminal emulator
        if ((te_sock = accept_telnet(listen_sock)) < 0) {
            goto cleanup;
        }
        shutdown(listen_sock, 2);
        close(listen_sock);
        listen_sock = -1;
    }
    // TELNET negotiation
    if (!dumb) {
        telnet_nego(te_sock);
    }
    // execute a shell
    if ((sh_pty = exec_shell(&sh_pid)) < 0) {
        goto cleanup;
    }
    // set initial pty window size
    if (!dumb && c_will_naws && win_size.ws_col != 0) {
        ioctl(sh_pty, TIOCSWINSZ, &win_size);
    }

    // relay the terminal emulator and the shell
    telnet_session(te_sock, sh_pty);

  cleanup:
    if (sh_pty >= 0) {
        close(sh_pty);
        kill(sh_pid, SIGKILL);
        wait((int*)NULL);
    }
    if (listen_sock >= 0) {
        shutdown(listen_sock, 2);
        close(listen_sock);
    }
    if (te_sock >= 0) {
        shutdown(te_sock, 2);
        close(te_sock);
    }
    if (hTerm != NULL) {
        WaitForSingleObject(hTerm, INFINITE);
        CloseHandle(hTerm);
    }
    return 0;
}

/**
 * To avoid bellow error, disable follow codes.
 * NOTE: To avoid display console window as cygwin application,
 *       you must build without "gcc -g"
 * -----------------------------------------------------------------------------
 * gcc -g -fno-exceptions -o cygterm.exe cygterm.cc  -mwindows -lc -ltk
 * /tmp/ccfWqjPN.o: In function `WinMainCRTStartup':
 * /build/cygterm106/cygterm.cc:877: multiple definition of `_WinMainCRTStartup'
 * /usr/lib/gcc/i686-pc-cygwin/3.4.4/../../../crt0.o:: first defined here
 * collect2: ld returned 1 exit status
 * make: *** [cygterm.exe] Error 1
 */
#if 0
// This program is a Win32 application but, start as Cygwin main().
//-----------------------------------------------------------------
extern "C" {
    void mainCRTStartup(void);
    void WinMainCRTStartup(void) { mainCRTStartup(); }
};
#endif
//EOF