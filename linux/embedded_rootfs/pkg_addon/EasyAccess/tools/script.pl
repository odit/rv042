#!/usr/bin/perl

$host_name=`hostname`;

while (<>)
{
    s/XXX_SERVER_NAME/$host_name/gi;
    print;
}
