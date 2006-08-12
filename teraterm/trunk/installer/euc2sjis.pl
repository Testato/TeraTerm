#!/usr/bin/perl

require 5.8.0;
use Encode;
use utf8;
use File::Basename;

$filename = $ARGV[0];
$outname  = $ARGV[1];
open (IN,  '<:unix:encoding(euc-jp)',  $filename);
open (OUT, '>:encoding(shiftjis)', $outname);
while (<IN>) {
	print OUT $_;
}
close OUT;
close IN;
