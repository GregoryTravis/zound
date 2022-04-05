#!/usr/local/bin/perl -w

package Patch;

use Graph;
use NodeInst;
use Node;

# PUBLIC

sub new {
  return bless
    { nodeinsts => [],
      text => "" };
}

sub read {
  my $class = shift;
  my $fname = shift;

  my $self = $class->new;

  $self->parse( $fname );
  $self->sort_nodeinsts;

  return $self;
}

sub add_options {
  my ($self,$options) = @_;
  foreach (@$options) {
    $self->{options}->{$_} = $_;
  }
}

sub gen {
  my $self = shift;

  #print "Options: " . (join( " ", sort keys %{$self->{options}} )) . "\n";

  my $s =
         $self->gen_header .
         $self->gen_lib_includes .
         $self->gen_node_param_structs .
         $self->gen_edge_data .
         $self->gen_edge_inits .
         $self->gen_node_data .
         $self->gen_node_inits .
         $self->gen_node_terms .
         $self->gen_node_dones .
         $self->gen_main;

  return $s;
}

sub get_edge_types {
  my $self = shift;
  my $ni;
  my $ets = {};
  foreach $ni (@{$self->{nodeinsts}}) {
    $ets = { %$ets, %{$ni->get_edge_types} };
  }
  return $ets;
}

# PRIVATE

sub sort_nodeinsts {
  my $self = shift;

  my ($i, $j);

  my $g;

  for ($i=0; $i<=$#{$self->{nodeinsts}}; ++$i) {
    my $in = $self->{nodeinsts}->[$i];
    for ($j=0; $j<=$#{$self->{nodeinsts}}; ++$j) {
      my $jn = $self->{nodeinsts}->[$j];
      next if ($in eq $jn);
      my $int = Util->intersectp( $in->get_outs, $jn->get_ins );
      #Util->dump( "HA", $in->get_outs, $jn->get_ins, $int );
      if ($int > 0) {
        push @{$g->{$in->{serial}}}, $jn->{serial};
      }
    }
  }

  #Util->dump( "GGG", $g );
  return if (!defined $g);

  my $graph = Graph->new( $g );
  my $sorted = $graph->sort_nodes;
  #Util->dump( "SORTED", $sorted );

  # sort the nodes

  my @newni = ();
  for ($i=0; $i<=$#{$sorted}; ++$i) {
    push @newni, $self->{nodeinsts}->[$sorted->[$i]];
  }

  #print "OLD @{$self->{nodeinsts}}\n";
  #print "NEW @newni\n";

  $self->{nodeinsts} = \@newni;
}

sub gen_lib_includes {
  my $self = shift;
  return Zound::gen_lib_include_list;
}

sub gen_header {
  my $self = shift;

  return $Zound::header;
}

sub gen_main {
  my $self = shift;

  my $s = "";

  my $pin = $self->{options}->{frame_profile} ? "profile_init();" : "";

  $s .= <<EOF;
void main( int argc, char *argv[] )
{
  struct sigaction sa;

  sa.sa_handler = &break_handler;
  sigemptyset( &sa.sa_mask );
  sa.sa_flags = SA_RESTART;
  sigaction( SIGINT, &sa, NULL );

  printf( "Frame len is %d samples\n", BUFLEN );

EOF

  if ($self->{buffering}) {
    $s .= <<EOF;
      card_buffering( $self->{buffering}->[0], $self->{buffering}->[1] );
EOF
  }

  $s .= <<EOF;
  $pin
  sync();
  curtime = 0;
  curtime_ms = 0;
  init_rt_params( argc, argv );
  edge_inits();
  init_node_params();
  node_inits();
  starttime_ms = z_time();

while (!allnodesdone && !sudden_term) {
EOF

  my $i;
  my $nis = $self->{nodeinsts};
  for ($i=0; $i<=$#$nis; ++$i) {
    my $ni = $nis->[$i];
    $s .= $ni->gen_loop( $self->{options} );
  }

  # Edge inits
  $s .= <<EOF;
if (done_should_init) {
  done_should_init = 0;
  edge_inits();
}
EOF

  # Frame profiling
  $s .= <<EOF if ($self->{options}->{frame_profile});
  {
    profile_store( z_time() );
  }
EOF

  # Advance time one frame
  $s .= <<EOF;
curtime += BUFLEN;
EOF

  # Wait until end of frame
  $s .= <<EOF if (!$self->{options}->{nonrealtime});
{
  long wait_until = (long)( 1000.0*((double)curtime/SAMPRATE) + starttime_ms);
  long now;

  do {
    now = z_time();
    if (now>=wait_until) break;
  } while (1);

  curtime_ms = now;
}
EOF

  $s .= <<EOF if ($self->{options}->{nonrealtime});
    curtime_ms = z_time();
EOF

  # Do every-second drift check
  $s .= <<EOF if ($self->{options}->{drift_everysecond});
{
  static int gount=0;
  long real_curtime =
    (long)(SAMPRATE * ((double)(curtime_ms-starttime_ms))/1000);
  if (gount++>2750) {
    printf( "sec drift %d\n", real_curtime-curtime );
    gount=0;
  }
}
EOF

  # Check for all large drifts
  $s .= <<EOF if ($self->{options}->{drift_all});
{
  static int bad_latch = 0;
  long real_curtime =
    (long)( (SAMPRATE * ((double)(curtime_ms-starttime_ms))) /1000);
  long dcurtime = real_curtime - curtime;
  if (dcurtime > 50 || dcurtime < -50) {
    if (!bad_latch) {
      printf( "DRIFT %d\n", dcurtime );
      bad_latch = 1;
    }
  } else bad_latch = 0;
}
EOF

  $s .= <<EOF;
} /* while */

if (sudden_term)
  printf( "^C\\n" );

node_terms();

} /* main */
EOF

  return $s;
}

sub gen_edge_data {
  my $self = shift;
  my $et = $self->get_edge_types;
  my $s = "";
  my ($edge,$type);
  while (($edge,$type) = each %$et) {
    my $text = $Zound::edge_structs->{$type};
    my $xtext = Util->xlate( $text, "", { ID=>$edge } );
    $s .= $xtext;
  }
  return $s;
}

sub gen_edge_inits {
  my $self = shift;
  my $et = $self->get_edge_types;
  my $s = "";
  my ($edge,$type);
  $s .= "void edge_inits()\n{\n";
  while (($edge,$type) = each %$et) {
    my $text = $Zound::edge_inits->{$type};
    my $xtext = Util->xlate( $text, "", { ID=>$edge } );
    $s .= $xtext;
  }
  $s .= "}\n\n";
  return $s;
}

sub gen_node_data {
  my $self = shift;
  my $ni;
  my $s = "";

  $s .= "/* Start Node Data */\n";
  foreach $ni (@{$self->{nodeinsts}}) {
    $s .= $ni->gen_global;
  }
  $s .= "/* End Node Data */\n\n";
}

sub gen_node_inits {
  my $self = shift;
  my $ni;
  my $s = "";

  foreach $ni (@{$self->{nodeinsts}}) {
    $s .= $ni->gen_param_defines;
  }
  $s .= "void node_inits()\n{\n";
  foreach $ni (@{$self->{nodeinsts}}) {
    $s .= $ni->gen_init;
  }
  $s .= "}\n\n";
  return $s;
}

sub gen_node_terms {
  my $self = shift;
  my $ni;
  my $s = "";

  $s .= "void node_terms()\n{\n";
  foreach $ni (@{$self->{nodeinsts}}) {
    $s .= $ni->gen_term;
  }
  $s .= "}\n\n";
  return $s;
}

sub gen_node_param_structs {
  my $self = shift;

  my $s = "";

  my $nni = $#{$self->{nodeinsts}} + 1;
  $s .= "params *node_params[$nni];\n\n";

  $s .= "void init_node_params()\n{\n";

  my $ni;
  foreach $ni (@{$self->{nodeinsts}}) {
   $s .= $ni->gen_param_inits;
  }

  $s .= "}\n\n";

  return $s;
}

sub gen_node_dones {
  my $self = shift;

  my $s = "";

  $s .= "int node_dones[] =\n{ ";

  $s .= join( ",", map {
    (($_->is_src) ? 0 : -1);
  } @{$self->{nodeinsts}} );

  $s .= " };\n";

  my $tot = 0;
  map { $tot++ if ($_->is_src); } @{$self->{nodeinsts}};

  $s .= "int notdone = $tot;\n";
  $s .= "int allnodesdone = 0;\n\n";

  return $s;
}

sub parse {
  my $self = shift;
  my $filename = shift;
  my $nodeinsts = [];
  my $text = "";
  my $ni_index = 0;

  open (PATCH, $filename) || die "Can't open $filename: $!";

  while (<PATCH>) {
    $text .= $_;
    next if /^\w*$/;
    next if /^#/;

    if (/^%/) {
      /^%\s*(\w*)\s+(.*$)/;
      my $command = $1;
      my $rest = $2;
      if ($command eq "node") {
        my $ni = NodeInst->new( "ni_$ni_index"."_", $rest );
        $ni_index++;
        push @$nodeinsts, $ni;
      } elsif ($command eq "option") {
        $self->{options} = { map { $_=>$_ } split( " ", $rest ) };
      } elsif ($command eq "buffering") {
        $self->{buffering} = [ split( " ", $rest ) ];
      } else { 
        die "Unknown directive $command";
      }
    }
  }

  close PATCH;

  $self->{nodeinsts} = $nodeinsts;
  $self->{text} = $text;

  return $self;
}

1;
