// $Id: z_time.h,v 1.2 1998/07/12 22:57:27 mito Exp $

#ifndef _z_time_h_
#define _z_time_h_

long z_time( void );
long z_wait( long ms );
int z_create_timer( long delta );
int z_timer( int tid );

#endif /* _z_time_h_ */
