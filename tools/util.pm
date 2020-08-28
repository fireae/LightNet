#! /usr/bin/env perl

package util;

use 5.014;
use warnings;
use strict;
use File::Copy;

require Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw(add_custom_block make_defs_neat indent_block indent_lines
                 indent_line read_file backup_write err_exit warn_msg exit_msg);

sub add_custom_block {
    my $indent_offset = shift;
    my $custom_str = shift;
    my $states = shift;
    my @customs = split "\n", $custom_str;
    &indent_block($indent_offset, \@customs);
    push @$states, "{";
    push @$states, @customs;
    push @$states, "}";
}

sub make_defs_neat {
    my $defs = shift;
    my $max_offset = 0;
    my $def_re = qr /( |\*)([a-zA-Z0-9_\[\]]+(;$| *=.+;$))/;
    foreach (@$defs) {
        if (/$def_re/) {
            my $offset = index($_, $2);
            $max_offset = $max_offset < $offset ? $offset : $max_offset;
        } else {
            &err_exit("make_defs_neat: wrong format: $_");
        }
    }
    foreach (@$defs) {
        my $type = $1 if /^(struct +\w+|\w+)/;
        my $nstars = 0;
        $nstars = length $1 if /^$type *(\*+)/;
        my $rest = $2 if /$def_re/;
        $_ = sprintf("%-${max_offset}s", $type);
        my $re = " "x$nstars;
        my $stars = "*"x$nstars;
        s/$re$/$stars$rest/;
    }
}

sub indent_block {
    my $nspaces = shift;
    my $str = shift;
    my @lines = split "\n", $str;
    join "\n", &indent_lines($nspaces, \@lines);
}

sub indent_lines {
    my $nspaces = shift;
    my $states = shift;
    foreach (@$states) {
        $_ = " "x$nspaces.$_ unless /^\s*$/;
    }
    @$states;
}

sub indent_line {
    my $nspaces = shift;
    my $state = shift;
    $state = " "x$nspaces.$state;
}

sub read_file {
  my ($file) = @_;

  open FILE, '<', $file or die "Cannot open ${file}: $!";
  my $text = join '', <FILE>;
  close FILE;

  return $text;
}

sub backup_write {
    my $file = shift;
    my $str = shift;
    if (-e $file) {
        &warn_msg("${file} exists, backuped with subfix .bak");
        copy($file, "${file}.bak")
            or die "Cannot backup file ${file}.bak: $!";
    }
    open FILE, '>', $file or die "Cannot open $file: $!";
    print FILE $str;
    close FILE;
}

sub err_exit {
    my $msg = $_[0];
    die "ERROR: $msg";
}

sub warn_msg {
    my $msg = $_[0];
    print STDERR "WARNING: $msg\n";
}

sub exit_msg {
    my $status = shift;
    my $msg = shift;
    print $msg;
    exit $status;
}

1;
