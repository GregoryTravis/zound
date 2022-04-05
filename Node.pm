#!/usr/local/bin/perl -w

package Node;

use Util;
use Zound;

# PUBLIC

sub get {
  my $class = shift;
  my $name = shift;

  my $node = $cache->{$name};

  if (!$node) {
    $node = $class->read( $name );
    $cache->{$name} = $node;
  }

  return $node;
}

sub get_outs {
  my $self = shift;

  my @outs = map {
    ($_->[0] eq "out") ? ($_->[1]) : ();
  } @{$self->{args}};

  return \@outs;
}

sub get_ins {
  my $self = shift;

  my @ins = map {
    ($_->[0] eq "in") ? ($_->[1]) : ();
  } @{$self->{args}};

  return \@ins;
}

sub is_src {
  my $self = shift;

  my $ins = $self->get_ins;

  return $#{$ins} == -1;
}

sub fparam_to_type {
  my $self = shift;
  my $fparam = shift;
  my @ftt = map {
    ($_->[1] eq $fparam) ? ($_->[2]) : ();
  } @{$self->{args}};
  die "Can't find type of param $fparam" if ($#ftt!=0);
  return $ftt[0];
}

sub gen_code {
  my $self = shift;
  my $segment = shift;
  my $prefix = shift;
  my $xlate = shift;

  return "" if (!exists $self->{sections}->{$segment});
  my $text = $self->{sections}->{$segment};

  return
    "\n" .
    Util->xlate( $text, $prefix,
    {%$xlate,%$Zound::global_xlate} ) .
    "\n";
}

# PRIVATE

local $cache = {};

sub read {
  my $class = shift;
  my $fname = shift;

  my $self = $class->new;
  $self->parse( $fname );

#Util->dump( "Node", $self );

  return $self;
}

sub new {
  my $class = shift;

  return bless
    { args => [ [] ],
      sections => { name=>"" },
      name => "" };
}

sub parse {
  my $self = shift;
  my $name = shift;
  my $section = "";
  my $sections = {};
  my $args = [];
  my $paramdefaults = {};

  open( NODE, $name.".c" ) || die "Can't open node $name.c";

  while (<NODE>) {
    next if /^\w*$/;
    #next if /^#/;
    if (/^%/) {
      /^%\s*(\w*)\s+(.*$)/;
      my $command = $1;
      my @rest = split( ' ', $2 );
      if ($command eq "sect") {
	  $section = $rest[0];
          $sections->{$section} = "";
	  die "bad section $section" if ($section eq "");
      } elsif ($command eq "arg") {
	  my $dir = $rest[0];
	  my $name = $rest[1];
	  my $type = $rest[2];
          push @$args, [ $dir, $name, $type ];
      } elsif ($command eq "lib") {
        my $l;
        foreach $l (@rest) {
          Zound::add_lib( $l );
        }
      } elsif ($command eq "paramdefault") {
        $paramdefaults->{$rest[0]} = $rest[1];
      } elsif ($command eq "rem") {
      } else { 
        die "Unknown directive $command";
      }
    } else {
      next if ($section eq "" && /^\/\//);
      die "sectionless line $_" if ($section eq "");
      $sections->{$section} .= $_;
#print "SECT $sections->{$section}\n";
#      push @{$sections->{$section}}, $_;
    }
  }

  close NODE;

  $self->{args} = $args;
  $self->{sections} = $sections;
  $self->{name} = $name;
  $self->{paramdefaults} = $paramdefaults;

  return $self;
}

1;
