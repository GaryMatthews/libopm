package OPM;

use 5.005;
use strict;
use vars qw/@ISA $VERSION $AUTOLOAD/;
use Carp;

require Exporter;
require DynaLoader;

@ISA = qw(Exporter DynaLoader);

$VERSION = '0.01';

sub AUTOLOAD {
   # This AUTOLOAD is used to 'autoload' constants from the constant()
   # XS function.  If a constant is not found then control is passed
   # to the AUTOLOAD in AutoLoader.

   my $constname;
   ($constname = $AUTOLOAD) =~ s/.*:://;
   croak "& not defined" if $constname eq 'constant';
   my $val = constant($constname, 0);
   if ($! != 0) {
	   croak "Your vendor has not defined OPM macro $constname";
   }

   {
      no strict 'refs';
      # Fixed between 5.005_53 and 5.005_61
      if ($] >= 5.00561) {
         *$AUTOLOAD = sub () { $val };
      }
      else {
         *$AUTOLOAD = sub { $val };
      }
   }
   goto &$AUTOLOAD;
}

bootstrap OPM $VERSION;

# Preloaded methods go here.

sub new {
   my $package = shift;
   if(@_) {
      return _remote_create(@_);
   }else{
      my $scan = _create();
      return $scan unless ref $scan;
      
      # It looks bad if we segfault easily, this just means we'll sit in an
      # infinite loop...
      $scan->config(OPM->CONFIG_FD_LIMIT, 512);
      $scan->config(OPM->CONFIG_MAX_READ, 4096);
      $scan->config(OPM->CONFIG_TIMEOUT,  30);

      return $scan;
   }
}

sub fatal_errors {
   OPM::_fatal_errors();
}

package OPM::Scan;

sub config {
   my($scanner, $type, $var) = @_;
   if($var =~ /^\d+$/) {
      $scanner->_config_int($type, $var);
   }else{
      $scanner->_config_string($type, $var);
   }
}

package OPM::Remote;

sub addr {
   my $remote = shift;
   return $remote->ip . ':' . $remote->port;
}

#package OPM::Error;

1;
__END__

=head1 NAME

OPM - Perl interface to libopm open proxy scanning library

=head1 SYNOPSIS

  use OPM;
  OPM->fatal_errors; # croak on errors

  my $scan = OPM->new; # new scanner
  my $remote = OPM->new("127.0.0.1"); # new remote IP to scan
  $scan->scan($remote); # Scans it

=head1 DESCRIPTION

Docs not yet done - see test.pl for best example.

=head2 EXPORT

None.

=head1 AUTHOR

David Leadbeater, E<lt>dg@blitzed.orgE<gt>

=head1 SEE ALSO

L<perl>.

=cut
