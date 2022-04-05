// $Id: z_types.h,v 1.2 1998/06/23 02:36:10 mito Exp $

#ifndef _z_types_h_
#define _z_types_h_

#ifndef _PTRDIFF_T /* Hack to not conflict with linux types! */
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
#endif /* _PTRDIFF_T */

typedef unsigned char byte;

typedef short sample;

#endif /* _z_types_h_ */
