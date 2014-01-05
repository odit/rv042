#!/bin/sh
# For Version ID get it from CVS release.
VersionID="1"
MinorVersionID="6.1"
echo "int versionId = $VersionID;" > version.c
echo "float minorVersionId = $MinorVersionID;" >> version.c
echo "char versionString[]="'"'\
"Version ID: $VersionID\n Build Date            : `date`\n Build Machine         : `uname -n`\n Build SRC             : `cd ..; pwd`"'"'";" >> version.c
