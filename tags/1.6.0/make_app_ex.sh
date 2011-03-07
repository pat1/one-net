#! /bin/bash

# usage: make_xcvr_app_example VERSION
#
# This shell script makes the transceiver specific application example projects
# for release.
#

# The application examples to make
APP_EX="adi.conf ia.conf semtech.conf"

#
# \brief Function to print out the usage case and exit
#
# \param void
#
# \return void
#
usage()
{
    echo "usage: $0 VERSION"
    exit 1
} # usage #

# make sure the correct number of parameters were passed in.
if [[ 1 -ne $# ]]
then
    usage
fi

VERSION=$1

for xcvr_app_ex in $APP_EX
do
    ./make_xcvr_app_example.sh "$xcvr_app_ex" $VERSION
done

