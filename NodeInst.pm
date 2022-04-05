#!/usr/local/bin/perl -w

package NodeInst;

use Node;
use Util;

local $gserial=0;

sub new {
  my $class = shift;
  my $name = shift;
  my $spec = shift;  # <nodename> [<param>=<value>, ...]

  my $self =
    { name => "",
      serial => ($gserial++),
      nodename => "",
      args => [],
      params => [],
      node => {} };

  bless $self;

#print "NODEINST $spec\n";

  my @spec = split( ' ', $spec );

  my $nodename = shift @spec;
  my ($arg, $args, $params);
  foreach $arg (@spec) {
    if ($arg =~ /=/) {
      my ($fname,$aname) = split( '=', $arg );
      push @$args, [ $fname, $aname ];
    } else {
      my ($fname,$aname) = split( ':', $arg );
      push @$params, [ $fname, $aname ];
    }
  }
  $self->{args} = $args;
  $self->{params} = $params;
  $self->{nodename} = $nodename;
  $self->{name} = $name;
  $self->{spec} = $spec;

  $self->{node} = Node->get( $nodename );

  bless $self;
#Util->dump( "Nodeinst", $self );
  return $self;
}

sub get_fparam {
  my $self = shift;
  my $fpname = shift;
}

# fparam->aparam
sub get_aparam {
  my $self = shift;
  my $fpname = shift;
  my @ho = map {
    ($_->[0] eq $fpname) ? ($_->[1]) : ();
  } @{$self->{args}};
  die if ($#ho != 0);
  return $ho[0];
}

# list of aparams that are outs
sub get_outs {
  my $self = shift;
  my @outs = map {
    $self->get_aparam( $_ );
  } @{$self->{node}->get_outs};
  return \@outs;
}

# list of aparams that are ins
sub get_ins {
  my $self = shift;
  my @ins = map {
    $self->get_aparam( $_ );
  } @{$self->{node}->get_ins};
  return \@ins;
}

sub is_src {
  my $self = shift;

  return $self->{node}->is_src;
}

sub get_edge_to_fparam {
  my $self = shift;
  my $i;
  my $etf;
  foreach $i (@{$self->{args}}) {
    $etf->{$i->[1]} = $i->[0];
  }
  return $etf;
}

sub get_edge_types {
  my $self = shift;
  my $etf = $self->get_edge_to_fparam;
  map {
    $etf->{$_} = $self->{node}->fparam_to_type( $etf->{$_} );
  } keys %$etf;
  return $etf;
}

sub get_param_xlate {
  my $self = shift;

  my $xlate = {};

  # translate fparam to aparam

  my $agp;
  foreach $agp (@{$self->{args}}) {
    $xlate->{$agp->[0]} = $agp->[1];
  }

  my $serial = $self->{serial};
  $xlate->{DONE} = "DONE($serial)";

  return $xlate;
}

sub gen_global {
  my $self = shift;

  return $self->{node}->gen_code( "global", $self->{name},
    $self->get_param_xlate );
}

sub gen_init {
  my $self = shift;

  return "/* Init: $self->{spec} */\n" .
         $self->{node}->gen_code( "init", $self->{name},
           $self->get_param_xlate );
}

sub gen_term {
  my $self = shift;

  return "/* Term: $self->{spec} */\n" .
         $self->{node}->gen_code( "term", $self->{name},
           $self->get_param_xlate );
}

sub gen_loop {
  my ($self,$options) = @_;

  return
    "/* $self->{spec} */\n" .
    "if (node_dones[$self->{serial}]!=1) {\n" .
    ($options->{timing} ? "long tyme = z_time();\n" : "") .
    ($Zound::verbose ?
      "printf( \"RUNNING NI $self->{serial}\\n\" );\n" : "") .
    "{" .
    $self->{node}->gen_code( "loop", $self->{name},
      $self->get_param_xlate ) .
    "}" .
    ($options->{timing} ? "tyme = z_time() - tyme;\n" .
      "if (tyme>10) printf( \"%d for %s\n\", tyme, \"$self->{spec}\" );\n"
      : "") .
    "}\n\n";
}

sub gen_param_defines {
  my $self = shift;

  my $s = "";
  my $serial = $self->{serial};

  $s .= "#define ni_$serial"."_param_int(s) PARAM_INT($serial,s)\n";
  $s .= "#define ni_$serial"."_param_double(s) PARAM_DOUBLE($serial,s)\n";
  $s .= "#define ni_$serial"."_param_string(s) PARAM_STRING($serial,s)\n";
}

sub gen_param_inits {
  my $self = shift;
  my $s = "";

  $s .= "{ params *ps = new_params();\n";

  # Stuff run-time and compile-time values
  my $p;
  foreach $p (@{$self->{params}}) {
    my $fname = $p->[0];
    my $aname = $p->[1];

    if ($aname =~ /^\$\$/) {
      $aname =~ s/^\$\$//;
      $s .= "param_add( ps, \"$fname\", param_string( rt_params, \"$aname\"".
        " ) );\n";
    } elsif ($aname =~ /^\$/) {
      $aname =~ s/^\$//;
      if (!exists $Zound::ct_params->{$aname}) {
        die "Need ct param $aname";
      }
      $s .= "param_add( ps, \"$fname\", \"$Zound::ct_params->{$aname}\" );\n";
    } else {
      $s .= "param_add( ps, \"$fname\", \"$aname\" );\n";
    }
  }

  # Stuff default values for anything not set
  my $pd = $self->{node}->{paramdefaults};
  foreach $p (keys %$pd) {
    $s .= "if (!param_is_set( ps, \"$p\" ))\n";
    $s .= "  param_add( ps, \"$p\", \"$pd->{$p}\" );\n";
  }

  $s .= "node_params[$self->{serial}] = ps;\n";
  $s .= "}\n";
}

1;
