#! /usr/bin/perl

use strict;
use warnings;
use Switch;
use Data::Dumper;
use Getopt::Long;

my $dump;
my $carray;
my $csv;
my $file;

GetOptions (
	    'dump!' => \$dump, 
	    'carray!' => \$carray,
	    'csv!' => \$csv,
	    'file=s' => \$file
);

my $state = "name";
my $name = "";
my %codes=();
my $i;
my $j;
my $val;
my $fh;
my @ararref=();

open($fh,"<",$file) or die ("can't open :".$file);

LINE:while(<$fh>){
chomp;

switch($state){
  #print $_."\n";
  case "name" 
    {
    # print "NAME";
     if($_ =~ m/@/){
       $codes{$name}=();
      # print $name.":";
       $state = "wait0";
       	push(@{$codes{$name}},"0");
	$i=1;
       next LINE;
    }else{
      $name .= $_;
    }
    }
  case "wait0" 
    {
    #  print "w0";
     if(/^0$/){
       $state = "getcodes";
        next LINE;
     }
    }
   case "getcodes" 
     {
     #  print "gc";
      if(/^#[0-9]+/){
	$state = "end";
      }else{
	push(@{$codes{$name}},$_);
	$i++;
	 next LINE;
      }
     }
    case "end"{
	      # print "end";
	       if(/^--END--$/){
		 $state = "name";
	#	 print $name." ".$i."\n";
		 $name="";
		  next LINE;
	       }
	      }
  }
}


my $arref;
my $key;
my $arsz;
foreach $key (keys(%codes)){
$arref = $codes{$key};
$arsz = @$arref-1;
#print $key." ".$arsz."\n";
while($arsz > 0){
  @$arref[$arsz]= @$arref[$arsz]- @$arref[$arsz-1];
$arsz--;
}
}

if($dump){
  print ("-" x 10);
  print "\n";
  print Dumper(%codes);
  print ("-" x 10);
  print "\n";
}

my $codenbr;
my $maxsz;
if($carray){

 $codenbr = keys(%codes);
 $maxsz=0;
foreach $key (keys(%codes)){
$j = @{$codes{$key}};
$maxsz = $j if($i>$maxsz);
}
$j = $codenbr;
print "unsigned int codes[".$codenbr."][".$maxsz."]={";
foreach $key (keys(%codes)){
print "{";
$i = @{$codes{$key}};
foreach $val (@{$codes{$key}}){
  print $val;
  $i--;
  print "," if($i);
}
print "}";
$j--;
print "," if($j);
}
print "};\n";

}

$maxsz=0;
$codenbr = keys(%codes);
if($csv){
  foreach $key (sort(keys(%codes))){
    print $key.",,";
    push @ararref,$codes{$key};
    $j = @{$codes{$key}};
    $maxsz = $j if($i>$maxsz);
  }
  print "\n";
  for($j=0;$j<$maxsz;$j++){
    for($i=0;$i<$codenbr;$i++){
      print @{$ararref[$i]}[$j].",";
    }
    print "\n";
  }
  


}
