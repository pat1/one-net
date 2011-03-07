#! /bin/bash

# usage: make_xcvr_app_example XCVR_CFG_FILE VERSION
#
# This shell script makes a transceiver specific application example project.
# Files not pertaining to the application example for the specific transceiver
# will be omitted.
#
# Note: This script assumes the structure of the source tree, so make changes
#   carefully.
#

# variables
XCVR_CFG_FILE=$1
VERSION_STR=$2

# Source directory that contains application source code.
SRC_APP_DIR=applications/simple_dev_ex

# Source directory that contains the processor dependent functionality
PROCESSORS_DIR="processors"


#
# \brief Function to print out the usage case and exit
#
# \param void
#
# \return void
#
usage()
{
    echo "usage: $0 XCVR_CFG_FILE VERSION"
    exit 1
} # usage #


#
# \brief Function to copy a list of files or directories
#
# \param $1 -> FILE_LIST[in] The list of files to copy
# \param $2 -> SRC_DIR[in] The directory to copy the files from.
# \param $3 -> DST_DIR[in] The destination directory to copy the files to.
#
# \return 0 if successful, otherwise non-0
#
copy_files()
{
    # make sure the correct number of parameters were passed in.
    if [[ 3 -ne $# ]]
    then
        return 1
    fi

    FILE_LIST=$1
    SRC_DIR=$2
    DST_DIR=$3

    for file_name in $FILE_LIST
    do
        # Check if the file is explicitly named (by including "." or "*" in
        # filename), or if the file(s) are implicitly named (does not include
        # the ".").
        if [[ -n "`echo "$file_name" | grep "\."`" || -n "`echo "$file_name" | grep "*"`" ]]
        then
            # copy the file as is
            echo `cp --parents -v "$SRC_DIR/$file_name" "$DST_DIR"`
        else
            # copy the .c & .h files
            echo `cp --parents -v "$SRC_DIR/$file_name".c \
              "$SRC_DIR/$file_name".h "$DST_DIR"`
        fi
    done
} # copy files #


#
# \brief Function for removing directories
#
# This function starts searching in START_PATH for directories in DIR_LIST and
# removes all occurancies of the directoires in DIR_LIST in sub paths.
#
# \param $1 -> DIR_LIST[in] The list of directory names to search for and
#   remove.
# \param $2 -> START_PATH The directory to start the search in.
#
# \return 0 if successful, otherwise non-0
rm_dir()
{
    # make sure the correct number of parameters were passed in
    if [[ 2 -ne $# ]]
    then
        return 1
    fi

    DIR_LIST=$1
    START_PATH=$2

    for dir_name in $DIR_LIST
    do
        echo `find "$START_PATH" -name "$dir_name" -type d -print | xargs rm -rf`
    done
} # rm_dir #


# make sure the correct number of parameters were passed in.
if [[ 2 -ne $# ]]
then
    usage
fi

# reset positional parameters to get version number
set -- `echo $2 | gawk -F. '{print $1" "$2}'`

MAJOR_VER=$1
MINOR_VER=$2

if [[ -z "$MAJOR_VER" ]]
then MAJOR_VER=0
fi

if [[ -z "$MINOR_VER" ]]
then MINOR_VER=0
fi

if [ ! -f "$XCVR_CFG_FILE" ]
then
    echo "The transceiver config file \"$XCVR_CFG_FILE\" does not exist!"
    echo "Exiting!"
    exit 1
fi

source $XCVR_CFG_FILE

# Make sure all variables were set up correctly

if [[ -z XCVR || -z XCVR_VENDOR || -z MICRO_CONTROLLER || -z MICRO_VENDOR ]]
then
    echo "The transceiver config file \"$XCVR_CFG_FILE\" is not formatted properly!"
    echo "Exiting!"
    exit 1
fi

# Files to copy from /processors/$MICRO_CONTROLLER_VENDER_DIR/src/common
SRC_PROCESSOR_COMMON_FILE="client_port_specific.c client_util.h flash one_net_port_specific.c one_net_types.h"

# Add any common files that may depend on the transceiver being used
if [[ "$XCVR" == "ADF7025" ]]
then
    XCVR_DIR="adi"
    XCVR_IMPLEMENTATION_FILE="adi.c"
elif [[ "$XCVR" == "IA4421" ]]
then
    XCVR_DIR="ia"
    XCVR_IMPLEMENTATION_FILE="ia.c"
    SRC_PROCESSOR_COMMON_FILE="$SRC_PROCESSOR_COMMON_FILE spi"
elif [[ "$XCVR" == "XE1205" ]]
then
    XCVR_DIR="semtech"
    XCVR_IMPLEMENTATION_FILE="semtech_xe1205.c"
    SRC_PROCESSOR_COMMON_FILE="$SRC_PROCESSOR_COMMON_FILE spi"
else
    echo "Invalid transceiver $XCVR!"
    echo "Exiting!"
    exit 1
fi

# Add any common files that depend on the processor
if [[ "$MICRO_VENDOR" == "Renesas" ]]
then
    SRC_PROCESSOR_COMMON_FILE="$SRC_PROCESSOR_COMMON_FILE ncrt0.a30"

    SRC_MICRO_VENDOR_DIR="renesas"
    IDE_DIR="hew"
    IDE="HEW"

    RM_DST_DIR="$RM_DST_DIR Debug_R8C*"

    DEVELOPMENT_ENVIRONMENT="These examples have been developed to run on an R8C\/1B with 16KB of flash, 2KB of data flash, and 1KB of RAM.  Development was done using the HEW (version 4.02.00.022) IDE, along with an E8 Debugger (version 2.09)."
    # Set up microcontroller directory
    if [ "$MICRO_CONTROLLER" == "R8C1B" ]
    then
        MICRO_CONTROLLER_DIR="r8c1"
    else
        echo "Invalid $MICRO_VENDOR Micro Controller $MICRO_CONTROLLER!"
        echo "Exiting!"
        exit 1
    fi
else
    echo "The MICRO_VENDOR \"$MICRO_VENDOR\" is unknown!"
    echo "Exiting!"
    exit 1
fi

# Create the top level and software directory
TOP_LEVEL_DESTINATION_DIR=../Client_$XCVR-$MICRO_CONTROLLER\_Rev$MAJOR_VER\_$MINOR_VER
SW_DESTINATION_DIR=$TOP_LEVEL_DESTINATION_DIR/sw

# The directories to be removed from the destination at the end
RM_DST_DIR=".svn"

# remove the old directory if it exists.
if [ -e "$TOP_LEVEL_DESTINATION_DIR" ]
then
    echo `rm -rfv $TOP_LEVEL_DESTINATION_DIR`
fi

# Create top level & software directory
echo `mkdir --parents -v $SW_DESTINATION_DIR`

# Copy application
echo `cp --parents -v "$SRC_APP_DIR"/*.c "$SRC_APP_DIR"/*.h \
  "$SW_DESTINATION_DIR" 2>&1 | grep -v *.h`

# Copy ONE-NET application files
# Source Directory that contains the ONE-NET application layer source code
SRC_ONA_DIR=one_net/app

# ONE-NET Application (ONA) layer files (these can be lists of files)
ONA_UNIT_FILE="switch"
ONA_VOLTAGE_FILE="voltage_simple"

copy_files "$ONA_UNIT_FILE $ONA_VOLTAGE_FILE one_net_application" \
    "$SRC_ONA_DIR" "$SW_DESTINATION_DIR"

# Copy the ONE-NET mac & port specific filesfiles
# Source Directory that contains the ONE-NET application layer source code
SRC_ONE_NET_MAC_DIR=one_net/mac

# Source Port Specific directory
SRC_ONE_NET_PORT_SPECIFIC_DIR=one_net/port_specific

ONE_NET_MAC_FILE="one_net one_net_status_codes.h"
ONE_NET_PORT_SPECIFIC_FILE="one_net_port_specific.h"

# check if including the MASTER files
if [ $ONE_NET_MASTER == "yes" ]
then
    ONE_NET_MAC_FILE="$ONE_NET_MAC_FILE one_net_master"
    ONE_NET_PORT_SPECIFIC_FILE="$ONE_NET_PORT_SPECIFIC_FILE master_port_specific.h"
fi

# check if including the CLIENT files
if [ $ONE_NET_CLIENT == "yes" ]
then
    ONE_NET_MAC_FILE="$ONE_NET_MAC_FILE one_net_client"
    ONE_NET_PORT_SPECIFIC_FILE="$ONE_NET_PORT_SPECIFIC_FILE client_port_const.h client_port_specific.h"
fi

copy_files "$ONE_NET_MAC_FILE" "$SRC_ONE_NET_MAC_DIR" "$SW_DESTINATION_DIR"
copy_files "$ONE_NET_PORT_SPECIFIC_FILE" "$SRC_ONE_NET_PORT_SPECIFIC_DIR" "$SW_DESTINATION_DIR"

# Copy the ONE-NET utility folder
# Source Port Specific directory
SRC_ONE_NET_UTIL_DIR="one_net/utility"
echo `cp --parents -v "$SRC_ONE_NET_UTIL_DIR"/* "$SW_DESTINATION_DIR"`

# Copy the processor specific stuff.
# Copy workspace
echo `cp --parents -v -r "$PROCESSORS_DIR/$SRC_MICRO_VENDOR_DIR/$IDE_DIR/one_net_app_ex/$XCVR_DIR" \
  "$SW_DESTINATION_DIR"`

copy_files "$SRC_PROCESSOR_COMMON_FILE" "$PROCESSORS_DIR/$SRC_MICRO_VENDOR_DIR/src/common" \
  "$SW_DESTINATION_DIR"

echo `cp --parents -v -r "$PROCESSORS_DIR/$SRC_MICRO_VENDOR_DIR/src/$MICRO_CONTROLLER_DIR" \
  "$PROCESSORS_DIR/$SRC_MICRO_VENDOR_DIR/src/simple_io_example_$XCVR_DIR" "$SW_DESTINATION_DIR"`

# copy the transceiver directory
echo `cp --parents -v -r "transceivers/$XCVR_DIR" "$SW_DESTINATION_DIR"`

# remove unnecessary directories
rm_dir "$RM_DST_DIR" "$SW_DESTINATION_DIR"

# copy the license
echo `cp "one_net/ONE-NET License.pdf" "$TOP_LEVEL_DESTINATION_DIR"`

# copy the ONE-NET change log
echo `cp "one_net/ChangeLog.txt" "$SW_DESTINATION_DIR/one_net/ChangeLog.txt"`

# copy the device Change Log
echo `cp "$PROCESSORS_DIR/$SRC_MICRO_VENDOR_DIR/$IDE_DIR/one_net_app_ex/$XCVR_DIR/Release.txt" "$TOP_LEVEL_DESTINATION_DIR"`

# copy the README
sed -e "s/\$TRANSCEIVER/$XCVR_VENDOR $XCVR/g
  s/\$MICRO_CONTROLLER_DIR/$SRC_MICRO_VENDOR_DIR/g
  s/\$MICRO_CONTROLLER/$MICRO_VENDOR $MICRO_CONTROLLER/g
  s/\$DEVELOPMENT_ENVIRONMENT/$DEVELOPMENT_ENVIRONMENT/g
  s/\$IDE_DIR/$IDE_DIR/g
  s/\$IDE/$IDE/g
  s/\$XCVR_DIR/$XCVR_DIR/g
  s/\$XCVR_IMPLEMENTATION_FILE/$XCVR_IMPLEMENTATION_FILE/g" applications/simple_dev_ex/README.in > "$TOP_LEVEL_DESTINATION_DIR"/README.txt

