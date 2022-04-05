#!/usr/local/bin/perl -w

package Graph;

use Util;

# spec is { node=>[node, ...], ... }

# ex:
#   graph is { a=>[b,c], b=>[c] }
#   rev is { b=>[a], c=>[a,b], a=>[] }

sub new {
  my $class = shift;
  my $spec = shift;

  bless $spec;

  $spec->include_empties;

  return $spec;
}

sub build_empty {
  my $self = shift;

  my $empty = {};

  while (($k,$v) = each %$self) {
    $empty->{$k} = [];
    my $n;
    foreach $n (@$v) {
      $empty->{$n} = [];
    }
  }

  return $empty;
}

sub include_empties {
  my $self = shift;

  my ($k, $v);

  while (($k,$v) = each %$self) {
    my $vi;
    foreach $vi (@$v) {
      if (!(exists $self->{$vi})) {
        $self->{$vi} = [];
      }
    }
  }
}

sub build_rev {
  my $self = shift;

  my $rev = $self->build_empty;

  my ($k,$v);

  while (($k,$v) = each %$self) {
    my $n;
    foreach $n (@$v) {
      push @{$rev->{$n}}, $k;
    }
  }

  return $rev;
}

sub sort_nodes {
  my $self = shift;
  my $nnodes = $#{[keys %$self]};

  my $rev = $self->build_rev;

  my $sorted = [];

  while ($#{$sorted} lt $nnodes) {
    my ($k, $v, $removing);
    while (($k,$v) = each %$rev) {
      if ($#{$v} == -1) {
        push @$removing, $k;
      }
    }
    my $ri;
    foreach $ri (@$removing) {
      push @$sorted, $ri;
      delete $rev->{$ri};
    }
    foreach $ri (@$removing) {
      while (($k,$v) = each %$rev) {
        $rev->{$k} = Util->rember( $v, $ri );
      }
    }
  }

  return $sorted;
}

#$g = Graph->new( { a=>["b","c"], b=>["c"], c=>[], d=>["a","b","e"], e=>["c"], g=>["d","e"] } );
#Util->dump( "g", $g );
#Util->dump( "emp", $g->build_empty );
#Util->dump( "rev", $g->build_rev );
#$sorted = $g->sort_nodes;
#Util->dump( "sorted", $sorted );


1;
