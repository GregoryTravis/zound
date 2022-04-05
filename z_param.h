// $Id: z_param.h,v 1.2 1998/07/12 15:56:00 mito Exp $

#ifndef _z_param_h_
#define _z_param_h_

#define MAX_PARAMS 20
typedef struct params {
  char *keys[MAX_PARAMS], *values[MAX_PARAMS];
  int num;
} params;

params *new_params();
void param_add( params *ps, char *fname, char *aname );
void param_add_comline( params *ps, int argc, char *argv[] );
int param_int( params *ps, char *key );
double param_double( params *ps, char *key );
char *param_string( params *ps, char *key );
int param_is_set( params *ps, char *key );

#endif /* _z_param_h_ */
