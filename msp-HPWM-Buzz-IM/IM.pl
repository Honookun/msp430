#! /usr/bin/perl

use strict;
use warnings;
use MIDI;

use Data::Dumper;

my @EvtGet = ("note_on","note_off","instrument_name","track_name","set_tempo","time_signature","text_event");

my $midi =  MIDI::Opus->new( {
     "from_file" => $ARGV[0],
#     "exclusive_event_callback" => sub{print "$_[2]\n"},
     "include" => \@EvtGet
#     "include" => \@MIDI::Event::All_events
   } );

print $midi->ticks()."\n";

my $track;
my $mtracks;
my $event;
my $currevent;
my $tracknum =0;
my %tmpH = ();
my @trackar = ();
my @eventsAr = ();
my @contevents = ();
my @endevents = ();
my $currtime =0;
my $initpause = 0 ;
my $list_ref;
my %notes;
my %notesbfreq;
my @freqbnbr;
my $notename;
my $freq;
my $i;
my $tempo= 500000; #default tempo
my $tempo_rate=$tempo/$midi->ticks(); #8333 us / tick
my $aclkprescal = 4;
#print Dumper $midi;

$Data::Dumper::Indent = 0;


sub oldEvtParse {
    
  $currtime =0;
  for $list_ref ( sort { $a->[0] <=> $b->[0] } @eventsAr ) {
    if (@$list_ref[2] eq "note_on") {
      if ($currtime !=@$list_ref[0]) {
	#print "".($currtime-$initpause).",  @$list_ref[1], Pause, 0, ".((@$list_ref[0]-$currtime))."\n" if($currtime != 0);
	push (@contevents,[0, (@$list_ref[0]-$currtime) ] ); 
      }
      $currtime=@$list_ref[0];
    }
    if (@$list_ref[2] eq "note_off") {
	    
      $initpause = $currtime if ($initpause == 0 && $currtime != 0);  
      #print "".($currtime-$initpause).", @$list_ref[1], Note, @$list_ref[4], ".(@$list_ref[0]-$currtime) ."\n";
      push (@contevents,[@$list_ref[4], (@$list_ref[0]-$currtime) ] ); 
      $currtime=@$list_ref[0];
    }	
  } 
}

sub newEvtParse{
  my $state="note_off";
  my $oldnote=0;
  $currtime =0;
  for $list_ref ( sort { $a->[0] <=> $b->[0] } @eventsAr ) {
    if (@$list_ref[2] eq "note_on") {
      if ($state eq "note_on") {
	$oldnote=@$list_ref[4] if($oldnote == 0);
	push (@contevents,[$oldnote,int((@$list_ref[0]-$currtime)) ] );
	$oldnote=@$list_ref[4];
	
      }
      if ($currtime !=@$list_ref[0]) {
	push (@contevents,[0, (@$list_ref[0]-$currtime) ] );  
      }
      $state = "note_on";
      $currtime=@$list_ref[0];	
    } else {
      if (@$list_ref[2] eq "note_off") {
	$initpause = $currtime if ($initpause == 0 && $currtime != 0);  
	push (@contevents,[@$list_ref[4], (@$list_ref[0]-$currtime) ] );  
	$state = "note_off";
	$currtime=@$list_ref[0];	
      } else {
	print "T:".$currtime." ".@$list_ref[2]." ".@$list_ref[3]." ";
	print @$list_ref[4]." " if(defined @$list_ref[4] );
	print @$list_ref[5]." " if(defined @$list_ref[5] );
	print @$list_ref[6]." " if(defined @$list_ref[6] );

	print "\n";
      }
      
    }
  }
 
}



open my $infile,"<","notes.h" or die "can't open notes.h";
$i=0;
while (<$infile>) {
  if (/^\/+\*([^*]+)[^0-9]+([0-9]+)/) {
    $notename=$1;
    $freq=$2;
    foreach (split /\//,$notename) {
      chomp;
      if (!defined $notes{$_} ) {
	$notes{$_} = ($freq+0);
      }
    }
  }

}

close $infile;


%notesbfreq = reverse %notes;
$i=0;
foreach (sort { $a <=> $b } (keys %notesbfreq)) {
  push @freqbnbr, $_;
  #    print ("".($_)."->".($i)." ".$notesbfreq{$freqbnbr[$i++]}."\n");
}

#before removing tracks insert trck0 set tempos in tmp array 
#(key absolute time)
foreach $event (($midi->tracks())[0]->events) {
  $currtime += ((@$event[1]));
  if(@$event[0] eq "set_tempo" ){
     $tmpH{$currtime} = $event;
  }
}

if (defined $ARGV[2] ) {
  $mtracks = $midi->tracks_r;
  foreach (split ',',$ARGV[2]){
    print "Selected track :$_\n";
    push @trackar, @$mtracks[$_];
  }
  if( @trackar == 0 ){
    $Data::Dumper::Maxdepth = 1;
    print Dumper @$mtracks;
    die("track without events !");
  }
  $midi->tracks_r(\@trackar);
}

my @tempoChangeAr = (sort { $a <=> $b } keys %tmpH);
push @tempoChangeAr,-1;
my $nextempo =0;
my $evtnbr =0;
foreach $track ($midi->tracks()){ 
#$track = ($midi->tracks())[0]; 
  $currtime =0;
  $nextempo =0;
  $evtnbr =0;
  foreach $event ($track->events) {
    $currtime += @$event[1];
    if($currtime >= $tempoChangeAr[$nextempo] && $tempoChangeAr[$nextempo] != -1){
      splice $track->events_r(),$evtnbr+1,0,($tmpH{$tempoChangeAr[$nextempo]});
      print "ADD TMPO $currtime:$evtnbr: @{$tmpH{$tempoChangeAr[$nextempo]}}\n";
      $nextempo++;
    }
    $evtnbr++;
  }
  $track->skyline();
}


foreach $track ($midi->tracks()) {
  $currtime =0;
  if (keys $track->events_r) {
    print "#$tracknum EV $track->events_r)\n";
    foreach $event ($track->events) {
		
      $currtime += int(@$event[1] * $tempo_rate);
     
	  

      push (@eventsAr, [$currtime,$tracknum,@$event[0],@$event[2], @$event[3],@$event[4], @$event[5]]);
      if(@$event[0] eq "set_tempo"){
	print("@$event");
	$tempo_rate = (@$event[2]/$midi->ticks());
	print " old:".$tempo." New :".((@$event[2])*1.0)." NRate:".$tempo_rate."\n";
	$tempo = @$event[2];
      }
      # print "$tracknum @$event[0] @$event[1] @$event[2] @$event[3]\n";      
    }
  }
  $tracknum++;
}

#print Dumper @eventsAr;die();

#oldEvtParse();
newEvtParse();

#print Dumper @contevents;die();

$currevent = [0,0];

foreach $event (@contevents) {
  #     print "$event\n";
  if (@$event) {
    if (@$currevent[0] == @$event[0]) {
      @$currevent[1] += @$event[1];
    } else {
      push (@endevents,$currevent);
      $currevent = $event;
    }
  } 
}


open(my $outfile,">","song.h");
$i=0;
print  $outfile "\n#ifndef __SONG_H\n#define __SONG_H\n#define SONG_SIZE ".($ARGV[1]-1)."\nconst unsigned char notes[".($ARGV[1]+0)."]={";
foreach $event (@endevents) {
  if (@$event[0] == 0) {
    if( $i != 0 ){
      print $outfile "0," if(@$event[1]>0);
      $i++;
    }
  } else {
    if ( int((@$event[1]/1000000.0)/(1/(32767/$aclkprescal)))>0) {
      print $outfile (@$event[0]+1).",";
      $i++;
    }
  }
  last if($i == ($ARGV[1]-1));
}

$i=0;
print  $outfile "};\nconst unsigned int durations[".($ARGV[1]+0)."]={";
foreach $event (@endevents) {
  if (@$event[1]>0) {

    if (@$event[0] == 0 && $i ==0){
      $i++
    }else{
      my $t = int((@$event[1]/1000000.0)/(1/(32767/$aclkprescal)));
      if($t>0){
	print $outfile $t.",";
	$i++;
      }
    }
  }
 last if($i == ($ARGV[1]-1));
}
print  $outfile "};\n#endif\n";

close $outfile;
