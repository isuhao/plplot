#!/usr/bin/perl -w
#
# check-copyright.pl: Parse a ChangeLog file generated by cvsreport (like
#   those in the www/announce/ directory) and output all the files mentioned
#   there together with the CVS users that modified them and the  lines
#   containing the Copyright holders.
#
# Copyright (C) 2004  Rafael Laboissiere

if (scalar @ARGV) {
  open (CHGLOG, "< $ARGV[0]")
    or die "Cannot open ChangeLog file $ARGV[0]";
} else {
  open (CHGLOG, "< &STDIN");
}

my %files = ();
my ($user, $file);

while (<CHGLOG>) {
  if (/^Commit from (\w+)/) {
    $user = $1;
    next;
  }
  if (/^..plplot  ([^\s]+)/) {
    $file = $1;
    next if (not -f $file);
    if (not defined $files{$file}) {
      $files{$file} = {
        users => { $user => 1 },
        copyright => join ("", qx {grep Copyright $file})
      };
    } else {
      $files{$file}->{users}->{$user} = 1;
    }
  }
}

for my $f (sort keys %files) {
  print STDOUT ("$f: " . join (", ", keys %{$files{$f}->{users}}) . "\n"
                . $files{$f}->{copyright} . "=====\n");
}
