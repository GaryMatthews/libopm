#!/usr/bin/perl
# A basic open proxy checker based on libopm and the OPM perl module
# Reads a file on stdin and returns if the proxies are open

# Basic protocol: (>> sent to bopchecker, << recived from bopchker).
# >> ip.ad.re.ss
# << ip.ad.re.ss open port,portN protocol,protocolN 
# << ip.ad.re.ss closed
# << ip.ad.re.ss error string
#
# You can also specify an additional port to scan as well as the default ones
#  >> ip.ad.re.ss port protocol
#  << ip.ad.re.ss open port,portN protocol,protocolN 
#  (and other values as above)

use strict;
use IO::Select;
use OPM;

# Buffer of input from STDIN
my $buffer;
# Temp. storage of ports proxies are open on
my %open;
# Number of open proxies found
my $numopen;

my $select = new IO::Select ( \*STDIN );
my $scan = OPM->new or die("Error loading OPM");

$scan->addtype(OPM->TYPE_HTTP, 80);
$scan->addtype(OPM->TYPE_HTTP, 3128);
$scan->addtype(OPM->TYPE_HTTP, 8080);
$scan->addtype(OPM->TYPE_SOCKS4, 1080);
$scan->addtype(OPM->TYPE_SOCKS5, 1080);
$scan->addtype(OPM->TYPE_ROUTER, 23);
$scan->addtype(OPM->TYPE_WINGATE, 23);
$scan->addtype(OPM->TYPE_HTTPPOST, 80);
$scan->addtype(OPM->TYPE_HTTPPOST, 3128);
$scan->addtype(OPM->TYPE_HTTPPOST, 8080);

# XXX: make configurable           "lik-m-aid.blitzed.org"
$scan->config(OPM->CONFIG_SCAN_IP, "203.56.139.100");
$scan->config(OPM->CONFIG_SCAN_PORT, 6667);
$scan->config(OPM->CONFIG_TARGET_STRING, ":lik-m-aid.ca.us.blitzed.org NOTICE AUTH :*** Looking up your hostname...");

$scan->callback(OPM->CALLBACK_END, \&callback_end);
$scan->callback(OPM->CALLBACK_OPENPROXY, \&callback_openproxy);

MAIN: while(1) {
   for my $sock($select->can_read(0.5)) {
      my $tmp;
      if(sysread($sock, $tmp, 1024) == 0) {
         last MAIN;
      }
      $buffer .= $tmp;

      while($buffer =~ s/^([^\n]+)\n//) {
         my($remote, $proxy, $proxyip);
         $proxy = $1;

         ($proxyip) = $proxy =~ /^([^:]+)/;
         $remote = OPM->new($proxyip);

         if($proxy =~ / (.+) (.+)$/) {
            my $port = $1;
            my $type = "TYPE_$2";
            unless(defined *{"OPM::$type"}) {
               print "$proxyip error Unknown protocol type\n";
               next;
            }
            $remote->addtype(OPM->$type, $port);
         }

         my $error = $scan->scan($remote);
         if($$error != OPM->SUCCESS) {
            print "$proxyip error " . $error->string . "\n";
         }
      }
   }
   $scan->cycle;
}

while($scan->active) {
   sleep 1;
   $scan->cycle;
}

exit $numopen;


sub callback_openproxy {
   my($scan, $remote, $val) = @_;
   push @{$open{$remote->ip}}, [$remote->port, $remote->protocol];
}

sub callback_end {
   my($scan, $remote, $val) = @_;
   my $ip = $remote->ip;

   if(exists $open{$ip}) {
      printf("%s open %s %s\n", $ip,
         join(",", map { $_->[0] } @{$open{$ip}}),
         join(",", map { $_->[1] } @{$open{$ip}}));
      delete $open{$ip};
      $numopen++;
   }else{
      print "$ip closed\n";
   }

   $remote->free;
}

