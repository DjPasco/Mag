/*
 *  Copyright (C) 2002 - 2007 Tomasz Kojm <tkojm@clamav.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */
#ifdef	_MSC_VER
#include <winsock.h>
#endif


#if HAVE_CONFIG_H
#include "clamav-config.h"
#endif

#include <stdio.h>
#ifdef	HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef	C_WINDOWS
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <utime.h>
#endif
#include <errno.h>

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#include "others.h"
#include "defaults.h"
#include "options.h"
#include "cfgparser.h"
#include "output.h"
#include "misc.h"
#include "str.h"
#include "client.h"

#ifdef PF_INET
# define SOCKET_INET	PF_INET
#else
# define SOCKET_INET	AF_INET
#endif

#ifndef	C_WINDOWS
#define	closesocket(s)	close(s)
#endif

/* #define ENABLE_FD_PASSING	    FIXME: Doesn't work yet */

void move_infected(const char *filename, const struct optstruct *opt);
int notremoved = 0, notmoved = 0;

#ifdef	C_WINDOWS
static  int     get_a_line(int sockd, char *buf, size_t len);

static int
dsresult(int sockd, const struct optstruct *opt)
{
	char buff[BUFSIZ], *pt;
	int infected = 0, waserror = 0;

	while(get_a_line(sockd, buff, sizeof(buff))) {
		if(strstr(buff, "FOUND\n")) {
			infected++;
			logg("%s", buff);
			if(opt_check(opt, "move") || opt_check(opt, "copy")) {
				/* filename: Virus FOUND */
				if((pt = strrchr(buff, ':'))) {
					*pt = 0;
					move_infected(buff, opt);
				} else
					mprintf("@Broken data format. File not %s.\n", opt_check(opt, "move") ? "moved" : "copied");
			} else if(opt_check(opt, "remove")) {
				if(!(pt = strrchr(buff, ':')))
					mprintf("@Broken data format. File not removed.\n");
				else {
					*pt = 0;
					if(unlink(buff)) {
						mprintf("%s: Can't remove.\n", buff);
						logg("%s: Can't remove.\n", buff);
						notremoved++;
					} else {
						mprintf("%s: Removed.\n", buff);
						logg("%s: Removed.\n", buff);
					}
				}
			}
		}

		if(strstr(buff, "ERROR\n")) {
			logg("%s", buff);
			waserror = 1;
		}
	}

	return infected ? infected : (waserror ? -1 : 0);
}

static int
get_a_line(int sockd, char *buf, size_t len)
{
	char *ptr;

	for(ptr = buf; ptr < &buf[len]; ptr++) {
		/* FIXME: very inefficient to call recv so many times */
		if(recv(sockd, ptr, sizeof(char), 0) <= 0)
			return 0;
		if(*ptr == '\n') {
			*++ptr = '\0';
			return 1;
		}
	}
	return 1;
}
#else
static int dsresult(int sockd, const struct optstruct *opt)
{
	int infected = 0, waserror = 0;
	char buff[4096], *pt;
	FILE *fd;


#ifndef C_OS2
    if((fd = fdopen(dup(sockd), "r")) == NULL) {
#else /* FIXME: according to YD OS/2 does not support dup() for sockets */
    if((fd = fdopen(sockd, "rb")) == NULL) {
#endif
	logg("^Can't open descriptor for reading.\n");
	return -1;
    }

    while(fgets(buff, sizeof(buff), fd)) {
	if(strstr(buff, "FOUND\n")) {
	    infected++;
	    logg("%s", buff);
	    if(opt_check(opt, "move") || opt_check(opt, "copy")) {
		/* filename: Virus FOUND */
		if((pt = strrchr(buff, ':'))) {
		    *pt = 0;
		    move_infected(buff, opt);
		} else {
		    mprintf("@Broken data format. File not %s.\n", opt_check(opt, "move") ? "moved" : "copied");
		}

	    } else if(opt_check(opt, "remove")) {
		if(!(pt = strrchr(buff, ':'))) {
		    mprintf("@Broken data format. File not removed.\n");
		} else {
		    *pt = 0;
		    if(unlink(buff)) {
			mprintf("%s: Can't remove.\n", buff);
			logg("%s: Can't remove.\n", buff);
			notremoved++;
		    } else {
			mprintf("%s: Removed.\n", buff);
			logg("%s: Removed.\n", buff);
		    }
		}
	    }
	}

	if(strstr(buff, "ERROR\n")) {
	    logg("%s", buff);
	    waserror = 1;
	}
    }

#ifndef C_OS2 /* Small memory leak under OS/2 (see above) */
    fclose(fd);
#endif

    return infected ? infected : (waserror ? -1 : 0);
}
#endif	/*C_WINDOWS*/

static int dsfile(int sockd, const char *scantype, const char *filename, const struct optstruct *opt)
{
	int ret;
	char *scancmd;


    scancmd = malloc(strlen(filename) + 20);
    sprintf(scancmd, "%s %s", scantype, filename);

    if(send(sockd, scancmd, strlen(scancmd), 0) <= 0) {
	logg("^Can't write to the socket.\n");
	free(scancmd);
	return -1;
    }

    free(scancmd);

    ret = dsresult(sockd, opt);

    if(!ret)
	logg("%s: OK\n", filename);

    return ret;
}

#if defined(ENABLE_FD_PASSING) && defined(HAVE_SENDMSG) && (defined(HAVE_ACCRIGHTS_IN_MSGHDR) || defined(HAVE_CONTROL_IN_MSGHDR)) && !defined(C_CYGWIN)

/* Submitted by Richard Lyons <frob-clamav*webcentral.com.au> */
static int dsfd(int sockfd, int fd, const struct optstruct *opt)
{
	struct iovec iov[1];
	struct msghdr msg;
#ifdef HAVE_CONTROL_IN_MSGHDR
#ifndef CMSG_SPACE
#define CMSG_SPACE(len)	    (_CMSG_ALIGN(sizeof(struct cmsghdr)) + _CMSG_ALIGN(len))
#endif
#ifndef CMSG_LEN
#define CMSG_LEN(len)	    (_CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))
#endif
	struct cmsghdr *cmsg;
	char tmp[CMSG_SPACE(sizeof(fd))];
#endif

    iov[0].iov_base = "";
    iov[0].iov_len = 1;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
#ifdef HAVE_CONTROL_IN_MSGHDR
    msg.msg_control = tmp;
    msg.msg_controllen = sizeof(tmp);
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
    *(int *)CMSG_DATA(cmsg) = fd;
#endif
#ifdef HAVE_ACCRIGHTS_IN_MSGHDR
    msg.msg_accrights = (caddr_t)&fd;
    msg.msg_accrightslen = sizeof(fd);
#endif
    if (sendmsg(sockfd, &msg, 0) != iov[0].iov_len) {
	logg("^Can't write to the socket.\n");
	return -1;
    }
    return dsresult(sockfd, opt);
}
#endif

static int dsstream(int sockd, const struct optstruct *opt)
{
	int wsockd, loopw = 60, bread, port, infected = 0;
	struct sockaddr_in server;
	struct sockaddr_in peer;
	socklen_t peer_size;
	char buff[4096], *pt;


    if(send(sockd, "STREAM", 6, 0) <= 0) {
	logg("^Can't write to the socket.\n");
	return 2;
    }

    while(loopw) {
	memset(buff, 0, sizeof(buff));
	if(recv(sockd, buff, sizeof(buff), 0) > 0) {
	    if((pt = strstr(buff, "PORT"))) {
		pt += 5;
		sscanf(pt, "%d", &port);
		break;
	    }
	}
	loopw--;
    }

    if(!loopw) {
	logg("^Daemon not ready for stream scanning.\n");
	return -1;
    }

    /* connect to clamd */

    if((wsockd = socket(SOCKET_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket()");
	logg("^Can't create the socket.\n");
	return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    peer_size = sizeof(peer);
    if(getpeername(sockd, (struct sockaddr *) &peer, &peer_size) < 0) {
	perror("getpeername()");
	logg("^Can't get socket peer name.\n");
	return -1;
    }

    switch (peer.sin_family) {
	case AF_UNIX:
	    server.sin_addr.s_addr = inet_addr("127.0.0.1");
	    break;
	case AF_INET:
	    server.sin_addr.s_addr = peer.sin_addr.s_addr;
	    break;
	default:
	    mprintf("^Unexpected socket type: %d.\n", peer.sin_family);
	    return -1;
    }

    if(connect(wsockd, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) < 0) {
	closesocket(wsockd);
	perror("connect()");
	logg("^Can't connect to clamd [port: %d].\n", port);
	return -1;
    }

    while((bread = read(0, buff, sizeof(buff))) > 0) {
	if(write(wsockd, buff, bread) <= 0) {
	    logg("^Can't write to the socket.\n");
	    closesocket(wsockd);
	    return -1;
	}
    }
    closesocket(wsockd);

    memset(buff, 0, sizeof(buff));
    while((bread = read(sockd, buff, sizeof(buff))) > 0) {
	logg("%s", buff);
	if(strstr(buff, "FOUND\n")) {
	    infected++;

	} else if(strstr(buff, "ERROR\n")) {
	    logg("%s", buff);
	    return -1;
	}
	memset(buff, 0, sizeof(buff));
    }

    return infected;
}

static char *abpath(const char *filename)
{
	struct stat foo;
	char *fullpath, cwd[200];

    if(stat(filename, &foo) == -1) {
	logg("^Can't access file %s\n", filename);
	perror(filename);
	return NULL;
    } else {
	fullpath = malloc(200 + strlen(filename) + 10);
#ifdef C_CYGWIN
	sprintf(fullpath, "%s", filename);
#else
	if(!getcwd(cwd, 200)) {
	    logg("^Can't get absolute pathname of current working directory.\n");
	    return NULL;
	}
	sprintf(fullpath, "%s/%s", cwd, filename);
#endif
    }

    return fullpath;
}

static int dconnect(const struct optstruct *opt)
{
#ifndef	C_WINDOWS
	struct sockaddr_un server;
#endif
	struct sockaddr_in server2;
	struct hostent *he;
	struct cfgstruct *copt;
	const struct cfgstruct *cpt;
	const char *clamav_conf = opt_arg(opt, "config-file");
	int sockd;


    if(!clamav_conf)
	clamav_conf = DEFAULT_CFG;

    if((copt = getcfg(clamav_conf, 1)) == NULL) {
	logg("^Can't parse the configuration file.\n");
	return -1;
    }

#ifndef	C_WINDOWS
    memset((char *) &server, 0, sizeof(server));
#endif
    memset((char *) &server2, 0, sizeof(server2));

    /* Set default address to connect to */
    server2.sin_addr.s_addr = inet_addr("127.0.0.1");    

    if((cpt = cfgopt(copt, "LocalSocket"))->enabled) {
#ifdef	C_WINDOWS
	logg("^LocalSocket is not supported under Windows");
#else

	server.sun_family = AF_UNIX;
	strncpy(server.sun_path, cpt->strarg, sizeof(server.sun_path));

	if((sockd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	    perror("socket()");
	    logg("^Can't create the socket.\n");
	    freecfg(copt);
	    return -1;
	}

	if(connect(sockd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
	    close(sockd);
	    perror("connect()");
	    logg("^Can't connect to clamd.\n");
	    freecfg(copt);
	    return -1;
	}
#endif
  
	} else if((cpt = cfgopt(copt, "TCPSocket"))->enabled) {
#ifdef  C_WINDOWS
	WSADATA wsaData;

	if(WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR) {
		logg("!Error at WSAStartup(): %d\n", WSAGetLastError());
		return -1;
	}
#endif
  

	if((sockd = socket(SOCKET_INET, SOCK_STREAM, 0)) < 0) {
	    perror("socket()");
	    logg("^Can't create the socket.\n");
	    freecfg(copt);
	    return -1;
	}

	server2.sin_family = AF_INET;
	server2.sin_port = htons(cpt->numarg);

	if((cpt = cfgopt(copt, "TCPAddr"))->enabled) {
	    if ((he = gethostbyname(cpt->strarg)) == 0) {
		closesocket(sockd);
		perror("gethostbyname()");
		logg("^Can't lookup clamd hostname.\n");
		freecfg(copt);
		return -1;
	    }
	    server2.sin_addr = *(struct in_addr *) he->h_addr_list[0];
	}

	if(connect(sockd, (struct sockaddr *) &server2, sizeof(struct sockaddr_in)) < 0) {
	    closesocket(sockd);
	    perror("connect()");
	    logg("^Can't connect to clamd.\n");
	    freecfg(copt);
	    return -1;
	}

    } else {
	logg("^Clamd is not configured properly.\n");
	freecfg(copt);
	return -1;
    }

    freecfg(copt);

    return sockd;
}

int client(const struct optstruct *opt, int *infected)
{
	char cwd[200], *fullpath;
	int sockd, ret, errors = 0;
	struct stat sb;
	const char *scantype = "CONTSCAN";


    *infected = 0;

    if(opt_check(opt, "multiscan"))
	scantype = "MULTISCAN";

    /* parse argument list */
    if(opt->filename == NULL || strlen(opt->filename) == 0) {
	/* scan current directory */
	if(!getcwd(cwd, 200)) {
	    logg("^Can't get absolute pathname of current working directory.\n");
	    return 2;
	}

	if((sockd = dconnect(opt)) < 0)
	    return 2;

	if((ret = dsfile(sockd, scantype, cwd, opt)) >= 0)
	    *infected += ret;
	else
	    errors++;

	closesocket(sockd);

#if defined(ENABLE_FD_PASSING) && defined(HAVE_SENDMSG) && (defined(HAVE_ACCRIGHTS_IN_MSGHDR) || defined(HAVE_CONTROL_IN_MSGHDR)) && !defined(C_CYGWIN)
    } else if(!strcmp(opt->filename, "-")) { /* scan data from stdin */
	if((sockd = dconnect(opt)) < 0)
	    return 2;

	if((ret = dsfd(sockd, 0, opt)) >= 0)
	    *infected += ret;
	else
	    errors++;

	close(sockd);
#else
    } else if(!strcmp(opt->filename, "-")) { /* scan data from stdin */
	if((sockd = dconnect(opt)) < 0)
	    return 2;

	if((ret = dsstream(sockd, opt)) >= 0)
	    *infected += ret;
	else
	    errors++;

	close(sockd);
#endif

    } else {
	int x;
	char *thefilename;
	for (x = 0; (thefilename = cli_strtok(opt->filename, x, "\t")) != NULL; x++) {
	    fullpath = thefilename;

	    if(stat(fullpath, &sb) == -1) {
		logg("^Can't access file %s\n", fullpath);
		perror(fullpath);
		errors++;
	    } else {
		if(strcmp(fullpath, "/") && (strlen(fullpath) < 2 || (fullpath[0] != '/' && fullpath[0] != '\\' && fullpath[1] != ':'))) {
		    fullpath = abpath(thefilename);
		    free(thefilename);

		    if(!fullpath) {
			logg("^Can't determine absolute path.\n");
			return 2;
		    }
		}

		switch(sb.st_mode & S_IFMT) {
		    case S_IFREG:
		    case S_IFDIR:
			if((sockd = dconnect(opt)) < 0)
			    return 2;

			if((ret = dsfile(sockd, scantype, fullpath, opt)) >= 0)
			    *infected += ret;
			else
			    errors++;

			close(sockd);
			break;

		    default:
			logg("^Not supported file type (%s)\n", fullpath);
			errors++;
		}
	    }

	    free(fullpath);
	}
    }

    return *infected ? 1 : (errors ? 2 : 0);
}

void move_infected(const char *filename, const struct optstruct *opt)
{
	char *movedir, *movefilename, numext[4 + 1];
	const char *tmp;
	struct stat ofstat, mfstat;
	int n, len, movefilename_size;
	int moveflag = opt_check(opt, "move");
#ifndef	C_WINDOWS
	struct utimbuf ubuf;
#endif


    if((moveflag && !(movedir = opt_arg(opt, "move"))) ||
        (!moveflag && !(movedir = opt_arg(opt, "copy")))) {
        /* Should never reach here */
        logg("^opt_arg() returned NULL\n");
        notmoved++;
        return;
    }

    if(access(movedir, W_OK|X_OK) == -1) {
        logg("^error %s file '%s': cannot write to '%s': %s\n", (moveflag) ? "moving" : "copying", filename, movedir, strerror(errno));
        notmoved++;
        return;
    }

    if(stat(filename, &ofstat) == -1) {
        logg("^Can't stat file %s\n", filename);
	logg("Try to run clamdscan with clamd privileges\n");
        notmoved++;
	return;
    }

    if(!(tmp = strrchr(filename, '/')))
	tmp = filename;

    movefilename_size = sizeof(char) * (strlen(movedir) + strlen(tmp) + sizeof(numext) + 2);

    if(!(movefilename = malloc(movefilename_size))) {
        logg("^Memory allocation error\n");
	exit(2);
    }

    if(!(cli_strrcpy(movefilename, movedir))) {
        logg("^cli_strrcpy() returned NULL\n");
        notmoved++;
        free(movefilename);
        return;
    }

    strcat(movefilename, "/");

    if(!(strcat(movefilename, tmp))) {
        logg("^strcat() returned NULL\n");
        notmoved++;
        free(movefilename);
        return;
    }

    if(!stat(movefilename, &mfstat)) {
        if((ofstat.st_dev == mfstat.st_dev) && (ofstat.st_ino == mfstat.st_ino)) { /* It's the same file*/
            logg("File excluded '%s'\n", filename);
            notmoved++;
            free(movefilename);
            return;
        } else {
            /* file exists - try to append an ordinal number to the
	     * quranatined file in an attempt not to overwrite existing
	     * files in quarantine  
	     */
            len = strlen(movefilename);
            n = 0;        		        		
            do {
                /* reset the movefilename to it's initial value by
		 * truncating to the original filename length
		 */
                movefilename[len] = 0;
                /* append .XXX */
                sprintf(numext, ".%03d", n++);
                strcat(movefilename, numext);            	
            } while(!stat(movefilename, &mfstat) && (n < 1000));
       }
    }

    if(!moveflag || rename(filename, movefilename) == -1) {
	if(filecopy(filename, movefilename) == -1) {
	    logg("^cannot %s '%s' to '%s': %s\n", (moveflag) ? "move" : "copy", filename, movefilename, strerror(errno));
	    notmoved++;
	    free(movefilename);
	    return;
	}

	chmod(movefilename, ofstat.st_mode);
	if(chown(movefilename, ofstat.st_uid, ofstat.st_gid) == -1)
	    logg("^chown() failed for %s: %s\n", movefilename, strerror(errno));


#ifndef	C_WINDOWS
	ubuf.actime = ofstat.st_atime;
	ubuf.modtime = ofstat.st_mtime;
	utime(movefilename, &ubuf);
#endif

	if(moveflag && unlink(filename)) {
	    logg("^cannot unlink '%s': %s\n", filename, strerror(errno));
	    notremoved++;            
	    free(movefilename);
	    return;
	}
    }

    logg("%s: %s to '%s'\n", (moveflag)?"moved":"copied", filename, movefilename);

    free(movefilename);
}
