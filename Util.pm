#!/usr/local/bin/perl -w

package Util;

sub tostring {
  my $class = shift;
  my $obj = shift;
  my $s = "";
  if (("".$obj) =~ /HASH/) {
    my ($k, $v);
    $s .= "{";
    while (($k,$v) = each %$obj) {
      $s .= "$k=>" . Util->tostring( $v ) . ", ";
    }
    $s =~ s/(^.*)(, $)/$1/;
    $s .= "}";
  } elsif (ref( $obj ) eq "ARRAY") {
    my $v;
    $s .= "[";
    foreach $v (@$obj) {
      $s .= Util->tostring( $v ) . ", ";
    }
    $s =~ s/(^.*)(, $)/$1/;
    $s .= "]";
  } elsif (ref( $obj ) eq "SCALAR") {
    $s .= $$obj;
  } else {
    $s .= $obj;
  }
  return $s;
}

sub dump {
  my $class = shift;
  my $obj;
  foreach $obj (@_) {
    print Util->tostring( $obj ) . " ";
  }
  print "\n";
}

sub rember {
  my $class = shift;
  my $list = shift;
  my $elem = shift;
  my @rret = grep {
    ($_ eq $elem) ? () : ($_);
  } @$list;
  my $ret = \@rret;
  return $ret;
}

sub member {
  my $class = shift;
  my $list = shift;
  my $elem = shift;
  my @g = grep {
    $_ eq $elem;
  } @$list;
  return $#g > -1;
}

sub intersectp {
  my $class = shift;
  my $list1 = shift;
  my $list2 = shift;
  my $l;
  foreach $l (@$list1) {
    return 1 if (Util->member( $list2, $l ));
  }
  return 0;
}

sub clone {
  my $class = shift;
  my $obj = shift;
  my $s = "";
  if (("".$obj) =~ /HASH/ || ref( $obj ) eq "ARRAY") {
  } elsif (ref( $obj ) eq "SCALAR") {
  } else {
  }
  return $s;
}

sub xlate {
  my $class = shift;
  my $text = shift;
  my $prefix = shift;
  my $map = shift;

  my $xtext = "";

  if ($text =~ /(^.*)"(.*)"(.*$)/s) {
    my $first = $1;
    my $string = $2;
    my $second = $3;
    return $class->xlate( $first, $prefix, $map ) .
           "\"$string\"" .
           $class->xlate( $second, $prefix, $map );
  }

  while ($text =~ /([^a-zA-Z_]*)([a-zA-Z\._][a-zA-Z0-9\._]*)(.*$)/s) {
    my $pre = $1;
    my $id = $2;
    my $post = $3;

    $xtext .= $pre;

    if (exists $map->{$id}) {
      $id = $map->{$id};
    } else {
      $id = $prefix . $id;
    }

    $xtext .= $id;
    $text = $post;
    #print "ID $id\n";
  }

  $xtext .= $text;

  return $xtext;
}

1;
