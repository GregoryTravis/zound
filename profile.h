/* $Id: profile.h,v 1.1 1998/07/14 00:35:44 mito Exp $ */

#ifndef _profile_h_
#define _profile_h_

#define MAX_DATA (65536*2)
extern long profile_data[];
extern int profile_data_p;

void profile_init( void );
#define profile_store(l)                  \
  do {                                    \
    if (profile_data_p<MAX_DATA)          \
      profile_data[profile_data_p++]=(l); \
  } while (0)

#endif /* _profile_h_ */
