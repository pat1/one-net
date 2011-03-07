// File : client_port_specific.h

// Dec. 27, 2010
/* Note : This is an empty file.  Prior to my creating this empty file, there
was no file anywhere called client_port_specific.h.  There is, however, a file
called client_port_specific.c located in the
processors/renesas/src/common directory, which is used to implement
several functions.  If you delete the file client_port_specific.c, several of
the Renesas projects will not compile.

client_port_specific.c has one_net_port_specific.h as a dependency.  It DOES NOT
client_port_specific.h as a dependency and it DOES NOT have
one_net_client_port_specific.h as a dependency.

Several Renesas projects list client_port_specific.h as a project dependency,
yet there is no such file.  It is unclear to me what the intent of the original
programmers was in this respect.  They either did not intend to have
client_port_specific.h as a dependency or they did and forgot to create this
file or this file existed at one time and no longer does.

It's fairly clear from looking at client_port_specific.c that
client_port_specific.h is supposed to exist and that client_port_specific.c
is supposed to #include it.  Note the comment below at the top of
client_port_specific.c.


\file client_port_specific.c
\brief Common implementation for ONE-NET CLIENT port specific functionality.

This file contains the common implementation (to the processor) for some of
the CLIENT port specific ONE-NET functionality.  Interfaces come from
client_port_specific.h and client_util.h.


Clearly client_port_specific.h and client_util.h are dependencies of
client_util.c and should be #included in that file.  Neither are.


Right now, I am simply creating this empty file and leaving
client_port_specific.c alone.  client_port_specific.c clearly needs to be
changed, but since it works correctly now, I am leaving it alone for now.
I am putting this empty file in the one_net/port_specific direectory because
the Renesas project dependency lists says that's where it should go.  I think,
however, that this is probably incorrect and the file should go in the
processors/renesas/src/common directory instead, or wherever the
client_port_specific.c file.

TO DO :  Find a real solution for this.

*/
