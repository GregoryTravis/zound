#!/usr/local/bin/perl -w

package Zound;

$verbose = 0;

$global_xlate = {};

$ct_params = {};

$libs = {};

open (NOXLATE, "noxlate.txt") || die $!;
@global_xlate_list = map { chop $_; $_; } <NOXLATE>;

local $poo;
foreach $poo (@global_xlate_list) {
  $global_xlate->{$poo} = $poo;
}

local $mono_edge_struct = <<EOF;
sample edge_rawm(ID)[BUFLEN];
EOF

local $mono_edge_init = <<EOF;
memset( edge_rawm(ID), 0, BUFLEN*sizeof(sample) );
EOF

local $score_edge_struct = <<EOF;
score_event scoreev(ID,SCOREBUFCAP);
int scorebuflen(ID);
EOF

local $score_edge_init = <<EOF;
scorebuflen(ID) = 0;
memset( scorebuf(ID), 0, SCOREBUFCAP*sizeof(score_event) );
EOF

$edge_structs =
{ mono=>$mono_edge_struct, score=>$score_edge_struct };

$edge_inits =
{ mono=>$mono_edge_init, score=>$score_edge_init };

$header = <<EOF;
/* Start Header */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "profile.h"
#include "z_ext.h"
#include "z_types.h"
#include "z_param.h"
#define SAMPMAX 32767
#define BUFSIZELOG (4)
#define BUFSIZE (1<<(BUFSIZELOG))
#define BUFLEN (BUFSIZE/sizeof(sample))
#define edge_rawm(e) edge_raw_##e
#define ain(e,i) edge_rawm(e)[i]
#define aout ain
#define DONE(ni) done(ni)
extern int node_dones[];
extern int notdone;
extern int allnodesdone;
extern void edge_inits();
int done_should_init = 0;
void done( int ni )
{
  if (!node_dones[ni]) {
    node_dones[ni] = 1;
    notdone--;
    done_should_init = 1;
    //edge_inits();
  }
  allnodesdone = notdone==0;
}
params *rt_params;
void init_rt_params( int argc, char *argv[] )
{
  rt_params = new_params();
  param_add_comline( rt_params, argc, argv );
}
#define PARAM_INT(n,s) param_int(node_params[n],s)
#define PARAM_DOUBLE(n,s) param_double(node_params[n],s)
#define PARAM_STRING(n,s) param_string(node_params[n],s)
#define rawbuf(e) edge_rawm(e)
#define SAMPRATE 44100
//#define CYCLEDURATION ((SAMPRATE*1000)/BUFLEN)
#define CYCLEDURATION ((BUFLEN*1000.0)/SAMPRATE)
#define SCOREBUFCAP 1024
long curtime=0; // time in samples processed
long curtime_ms=0; // time as from z_time (ftime)
long starttime_ms=0;
long ltime=0;
#define BPM 120
#define BEATNUM() (BPM*curtime)/(60*SAMPRATE)

int sudden_term = 0;
void break_handler( int n )
{
  sudden_term = 1;
}
/* End Header */

EOF

sub grab_ct_params {
  my $argv = shift;

  my $a;
  my ($fname, $aname);
  foreach $a (@$argv) {
    if ($a =~ /:/) {
      ($fname, $aname) = split( ":", $a );
      $ct_params->{$fname} = $aname;
    }
  }
}

sub add_lib {
  my $l = shift;
  $libs->{$l}++;
}

sub gen_lib_list {
  my $l;
  my $s = "";
  foreach $l (keys %$libs) {
    $s .= "$l".".o ";
  }
  return $s;
}

sub gen_lib_include_list {
  my $l;
  my $s = "";
  foreach $l (keys %$libs) {
    $s .= "#include \"$l".".h\"\n";
  }
  return $s;
}

1;
