#!/usr/bin/perl

use strict;
use Getopt::Long;
use Cwd;
use Env qw(PATH);


# Strip shell argument off of remaining args. Do this first
# as some machines (SL3) list shell as '-bash', which if left
# on will confuse getoptions
my $shell = shift(@ARGV);

# Clean up empty args we get from wrapper script
my $tmp;
while (!$tmp && scalar(@ARGV))
{
   $tmp = pop(@ARGV);
   push(@ARGV, $tmp) if $tmp;
}

my $octeon_model;
my $octeon_root;

# process command line options
my $runtime_model_flag = 0;
my $verbose = 0;
GetOptions("runtime-model!"      => \$runtime_model_flag,
           "verbose!"            => \$verbose,
           );


if (scalar(@ARGV) > 1 || scalar(@ARGV) < 1)
{
   usage();
   print 'echo ""';
   exit;
}
elsif (scalar(@ARGV) == 2)
{
   $octeon_root = $ARGV[1];
}
else
{
   $octeon_root = cwd();
}
$octeon_model = $ARGV[0];


sub usage
{
   warn "Usage: source ./env-setup <OCTEON_MODEL> [--runtime-model] ...\n";
   warn " OCTEON_MODEL:     Model of Octeon to build and simulate for.\n";
   warn " --runtime-model:  enables runtime model detection build option by setting environment variable.\n";
   warn "                   use --noruntime-model to clear environment variable if desired.\n";
   warn " --verbose:        be verbose about what the script is doing\n";
}


my %env_hash;  # Hash to fill with env variables to set
my @env_clear; #list of environment variables to clear
my $key;
my $extra_path = "$octeon_root/tools/bin:$octeon_root/host/bin";


# validate OCTEON_MODEL.  Warn but proceed if if octeon-models.txt is not present.
if (open(FH, "$octeon_root/octeon-models.txt"))
{
   my @models = <FH>;
   my ($match) = grep(/^$octeon_model$/, @models);
   if (!$match)
   {
      warn "ERROR: $octeon_model is not a valid OCTEON_MODEL value.  Please see \$OCTEON_ROOT/octeon-models.txt for list\n";
      print "echo ERROR: $octeon_model is not a valid OCTEON_MODEL value.  Please see \\\$OCTEON_ROOT/octeon-models.txt for list\n";
      exit;
   }
}
else
{
   warn 'Warning: unable to open file $OCTEON_ROOT/octeon-models.txt, can\'t validate OCTEON_MODEL\n';
}



# Set up hash of environment variables to set
$env_hash{'OCTEON_MODEL'} = $octeon_model;
$env_hash{'OCTEON_ROOT'} = $octeon_root;
if (!($PATH=~ /^$extra_path/))
{
   $env_hash{'PATH'} = $extra_path.':$PATH';
   warn "Updating PATH, adding $extra_path to beginning.\n" if ($verbose);
}
else
{
   warn "Not updating PATH - OCTEON SDK dirs already present.\n" if ($verbose);
}

# Clear the the env variable if not needed
if ($runtime_model_flag)
{
   $env_hash{'OCTEON_CPPFLAGS_GLOBAL_ADD'} = '-DUSE_RUNTIME_MODEL_CHECKS=1';
}
else
{
   push(@env_clear, 'OCTEON_CPPFLAGS_GLOBAL_ADD');
}

#print out what we are doing
if ($verbose)
{
   foreach $key (keys(%env_hash))
   {
      warn "Setting $key  to ".'"'."$env_hash{$key}".'"'."\n";
   }
   if (@env_clear)
   {
      foreach $key (@env_clear)
      {
         warn "Unsetting  $key\n";
      }
   }
}


# Generate environment setting command based on shell
my $env_cmd;
if ($shell =~ /csh$/)  # csh and tcsh
{
   foreach $key (keys(%env_hash))
   {
      $env_cmd .= "setenv $key ".'"'."$env_hash{$key}".'"'.';';
   }
   if (@env_clear)
   {
      foreach $key (@env_clear)
      {

         $env_cmd .= ";unsetenv  $key ";
      }
   }
   $env_cmd .= "\n";
   warn "CSH command: $env_cmd\n" if ($verbose);
   print $env_cmd;
}
elsif ($shell =~ /bash$/ || $shell =~ /ksh$/ || $shell =~ /sh$/)
{
   $env_cmd = 'export ';
   foreach $key (keys(%env_hash))
   {
      
      $env_cmd .= "$key=".'"'."$env_hash{$key}".'"'.' ';
   }
   if (@env_clear)
   {
      $env_cmd .= ";export ";
      foreach $key (@env_clear)
      {

         $env_cmd .= "-n $key ";
      }
   }
   
   $env_cmd .= "\n";
   warn "BASH command: $env_cmd\n" if ($verbose);
   print $env_cmd;
}
else
{
   print "echo Unsupported shell\n";
}
