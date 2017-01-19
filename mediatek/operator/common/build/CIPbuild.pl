use Getopt::Long;

my $addtionalOption = "";
my $project = "";
my $configFile = "mediatek/operator/common/build/CIPconfig.ini";

&GetOptions(
  "ini=s" => \$configFile,
  "o=s" => \$addtionalOption,
  "p=s" => \$project,
);

if($configFile =~ /^\s*$/){
  print "configFile is empty $configFile\n";
  &usage();
}
elsif(!-e $configFile){
  print "Can not find configFile=$configFile\n";
  &usage();
}
if($project =~ /^\s*$/){
  print "project is empty $project\n";
  &usage();
}

my $shotProject = $project;
$shotProject =~ s/\[\S+\]//;
my @AllCIPSupportOP;
my %feature2value;
my $firstMeetCIPSupport = 1;
open CONFIGFILE,"<$configFile" or die "Can not open configFile=$configFile\n";
while(<CONFIGFILE>){
  my $line = $_;
  chomp $line;
  next if($line =~ /^#/);
  next if($line =~ /^\s*$/);
  if($line =~ /^\s*CIP_SUPPORT\s*:=\s*(.+?)\s*$/){
    my $CIPSupportOP = $1;
    chomp $CIPSupportOP;
    die "Error: Find \"CIP_SUPPORT:=\" again\n" if(! $firstMeetCIPSupport);
    print "CIP_SUPPORT = $CIPSupportOP\n";
    @AllCIPSupportOP = split /\s+/,$CIPSupportOP;
  }
  elsif($line =~ /^\s*(\S+?)\s*:=\s*(\S+)\s*/){
    $feature2value{$1}=$2;
  }
  else{
    print "Warning: Unknow input line:$line\n";
  }
}
close CONFIGFILE;

my $cmd = "mv out/target/product/$shotProject/custom.img out/target/product/$shotProject/custom_bak.img";
system("$cmd") && &errorFlow("Backup custom.img fail");
for my $CIPSupportOP (@AllCIPSupportOP){
  my $featureValue = $feature2value{$CIPSupportOP};
  print $CIPSupportOP." = ".$featureValue."\n";
  my $command = "./mk -o=$featureValue $project customimage";
  print "$command\n";
  system("$command") && &errorFlow("Build customimage fail");
  &backup($CIPSupportOP);
}
$cmd = "mv out/target/product/$shotProject/custom_bak.img out/target/product/$shotProject/custom.img";
system("$cmd") && &errorFlow("Restore custom.img fail");
exit 0;

sub backup{
  my $backupName = $_[0];
  my $cmd = "mv out/target/product/$shotProject/custom.img out/target/product/$shotProject/custom${backupName}.img";
  print $cmd."\n";
  system("$cmd") && &errorFlow("Backup custom.img fail");
  $cmd = "mv out/target/product/${shotProject}_customimage.log out/target/product/${shotProject}_customimage${backupName}.log";
  print $cmd."\n";
  system("$cmd") && &errorFlow("Backup customimage.log fail");
  if(-e "out/target/product/${shotProject}_customimage.log_err"){
    $cmd = "mv out/target/product/${shotProject}_customimage.log_err out/target/product/${shotProject}_customimage${backupName}.log_err";
    print $cmd."\n";
    system("$cmd") && &errorFlow("Backup customimage.log_err fail");
  }
}

sub errorFlow{
  $errorMessage = $_[0];
  print "$errorMessage\n";
  exit 2;
}

sub usage{
  warn << "__END_OF_USAGE";
Usage: perl mediatek/operator/common/build/CIPbuild.pl [options]

Options:
  -ini      : CIP config file.
  -p        : Project to build.
  -o        : Pass extra arguments to build system.

Example:
  perl mediatek/operator/common/build/CIPbuild.pl -ini=mediatek/operator/common/build/CIPconfig.ini -p=mt6582_phone

__END_OF_USAGE

  exit 1;

}
