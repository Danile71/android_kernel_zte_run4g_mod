#!/usr/bin/perl
($#ARGV != 0) && &Usage;
my $prj = $ARGV[0];
my @prjmks;
if ((exists $ENV{"MTK_ROOT_CONFIG_OUT"}) && (-e $ENV{"MTK_ROOT_CONFIG_OUT"} . "/ProjectConfig.mk"))
{
  push(@prjmks, $ENV{"MTK_ROOT_CONFIG_OUT"} . "/ProjectConfig.mk");
}
else
{
  push(@prjmks, "mediatek/config/common/ProjectConfig.mk");
  push(@prjmks, "mediatek/config/${prj}/ProjectConfig.mk");
}

print "\n";
print "# begin mediatek build properties\n";

foreach my $prjmk (@prjmks) {
  if (!-e $prjmk) {
    die "#### Can't find $prjmk\n";
  } else {
    open (FILE_HANDLE, "<$prjmk") or die "cannot open $prjmk\n";
    while (<FILE_HANDLE>) {
      if (/^(\S+)\s*=\s*(\S+)/) {
        $$1 = $2;
      }
    }
    close FILE_HANDLE;
  }
}

print "ro.mediatek.version.release=$MTK_BUILD_VERNO\n";
print "ro.mediatek.platform=$MTK_PLATFORM\n";
print "ro.mediatek.chip_ver=$MTK_CHIP_VER\n";
print "ro.mediatek.version.branch=$MTK_BRANCH\n";
print "ro.mediatek.version.sdk=$PLATFORM_MTK_SDK_VERSION\n";
print "# end mediatek build properties\n";

exit 0;

