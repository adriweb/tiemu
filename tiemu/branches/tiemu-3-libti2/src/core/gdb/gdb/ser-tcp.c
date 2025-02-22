/* Serial interface for raw TCP connections on Un*x like systems
   Copyright 1992, 1993, 1994, 1995, 1996, 1998, 1999, 2001
   Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "defs.h"
#include "serial.h"

#include <sys/types.h>

#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>  /* For FIONBIO. */
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>  /* For FIONBIO. */
#endif

#include <sys/time.h>

#if !defined (__MINGW32__)
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <signal.h>
#else /* !__MINGW32__ */
#include <winsock2.h>
#endif /* !__MINGW32__ */

#include "gdb_string.h"

/*
 * Winsock needs to be started manually.
 */

#if defined (__MINGW32__)
#define ECONNREFUSED WSAECONNREFUSED
#define EINPROGRESS  WSAEINPROGRESS
#define ETIMEDOUT    WSAETIMEDOUT

static int ws_started;
static int
net_open_winsock ()
{
  if (!ws_started) {
    WORD    wVersionRequested;
    WSADATA wsaData;
    int     err;

    wVersionRequested = MAKEWORD (2, 2);
    err = WSAStartup (wVersionRequested, &wsaData);
    if (err)
      return 0;
    ws_started = 1;
  }
  return 1;
}
#endif /* !__MINGW32__ */

extern void ser_platform_tcp_init (struct serial_ops *ops);

static int net_open (struct serial *scb, const char *name);
static void net_close (struct serial *scb);
void _initialize_ser_tcp (void);

/* seconds to wait for connect */
#define TIMEOUT 15
/* how many times per second to poll deprecated_ui_loop_hook */
#define POLL_INTERVAL 2

/* Open a tcp socket */

static int
net_open (struct serial *scb, const char *name)
{
  char *port_str, hostname[100];
  int n, port, tmp;
  int use_udp;
  struct hostent *hostent;
  struct sockaddr_in sockaddr;

  use_udp = 0;
  if (strncmp (name, "udp:", 4) == 0)
    {
      use_udp = 1;
      name = name + 4;
    }
  else if (strncmp (name, "tcp:", 4) == 0)
    name = name + 4;

  port_str = strchr (name, ':');

  if (!port_str)
    error ("net_open: No colon in host name!");	   /* Shouldn't ever happen */

  tmp = min (port_str - name, (int) sizeof hostname - 1);
  strncpy (hostname, name, tmp);	/* Don't want colon */
  hostname[tmp] = '\000';	/* Tie off host name */
  port = atoi (port_str + 1);

  /* default hostname is localhost */
  if (!hostname[0])
    strcpy (hostname, "localhost");

#if defined (__MINGW32__)
  if (!net_open_winsock ())
    {
      fprintf_unfiltered (gdb_stderr, "error WINSOCK startup\n");
      errno = ENOENT;
      return -1;
    }    
#endif /* __MINGW32__ */
  
  hostent = gethostbyname (hostname);
  if (!hostent)
    {
      fprintf_unfiltered (gdb_stderr, "%s: unknown host\n", hostname);
      errno = ENOENT;
      return -1;
    }

  if (use_udp)
    scb->fd = socket (PF_INET, SOCK_DGRAM, 0);
  else
    scb->fd = socket (PF_INET, SOCK_STREAM, 0);

  if (scb->fd < 0)
    return -1;
  
  sockaddr.sin_family = PF_INET;
  sockaddr.sin_port = htons (port);
  memcpy (&sockaddr.sin_addr.s_addr, hostent->h_addr,
	  sizeof (struct in_addr));

#if !defined (__MINGW32__)
  /* set socket nonblocking */
  tmp = 1;
  ioctl (scb->fd, FIONBIO, &tmp);
#endif /* !__MINGW32__ */

  /* Use Non-blocking connect.  connect() will return 0 if connected already. */
  n = connect (scb->fd, (struct sockaddr *) &sockaddr, sizeof (sockaddr));

  if (n < 0 && errno != EINPROGRESS)
    {
      net_close (scb);
      return -1;
    }

  if (n)
    {
      /* looks like we need to wait for the connect */
      struct timeval t;
      fd_set rset, wset;
      int polls = 0;
      FD_ZERO (&rset);

      do 
	{
	  /* While we wait for the connect to complete 
	     poll the UI so it can update or the user can 
	     interrupt. */
	  if (deprecated_ui_loop_hook)
	    {
	      if (deprecated_ui_loop_hook (0))
		{
		  errno = EINTR;
		  net_close (scb);
		  return -1;
		}
	    }
	  
	  FD_SET (scb->fd, &rset);
	  wset = rset;
	  t.tv_sec = 0;
	  t.tv_usec = 1000000 / POLL_INTERVAL;
	  
	  n = select (scb->fd + 1, &rset, &wset, NULL, &t);
	  polls++;
	} 
      while (n == 0 && polls <= TIMEOUT * POLL_INTERVAL);
      if (n < 0 || polls > TIMEOUT * POLL_INTERVAL)
	{
	  if (polls > TIMEOUT * POLL_INTERVAL)
	    errno = ETIMEDOUT;
	  net_close (scb);
	  return -1;
	}
    }

  /* Got something.  Is it an error? */
  {
    int res, err, len;
    len = sizeof(err);
    res = getsockopt (scb->fd, SOL_SOCKET, SO_ERROR, &err, &len);
    if (res < 0 || err)
      {
	if (err)
	  errno = err;
	net_close (scb);
	return -1;
      }
  } 

#if !defined (__MINGW32__)
  /* turn off nonblocking */
  tmp = 0;
  ioctl (scb->fd, FIONBIO, &tmp);
#endif /* !__MINGW32__ */
  
  if (use_udp == 0)
    {
      /* Disable Nagle algorithm. Needed in some cases. */
      tmp = 1;
      setsockopt (scb->fd, IPPROTO_TCP, TCP_NODELAY,
		  (char *)&tmp, sizeof (tmp));
    }

  /* If we don't do this, then GDB simply exits
     when the remote side dies.  */
#if !defined (__MINGW32__)
  signal (SIGPIPE, SIG_IGN);
#endif /* !__MINGW32__ */

  return 0;
}

static void
net_close (struct serial *scb)
{
  if (scb->fd < 0)
    return;

  close (scb->fd);
  scb->fd = -1;
}

void
_initialize_ser_tcp (void)
{
  struct serial_ops *ops = XMALLOC (struct serial_ops);
  memset (ops, 0, sizeof (struct serial_ops));
  ops->name = "tcp";
  ops->next = 0;
  ops->open = net_open;
  ops->close = net_close;
  ser_platform_tcp_init (ops);
  serial_add_interface (ops);
}
