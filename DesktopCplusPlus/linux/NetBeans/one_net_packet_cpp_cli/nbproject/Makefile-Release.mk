#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Release
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/chip_connection.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/src/cli.o \
	${OBJECTDIR}/src/string_utils.o \
	${OBJECTDIR}/attribute.o \
	${OBJECTDIR}/filter.o \
	${OBJECTDIR}/src/xtea_key.o \
	${OBJECTDIR}/time_utils.o \
	${OBJECTDIR}/src/packet.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../one_net_lib/dist/Release/GNU-Linux-x86/libone_net_lib.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Release.mk dist/Release/GNU-Linux-x86/one_net_packet_cpp_cli

dist/Release/GNU-Linux-x86/one_net_packet_cpp_cli: ../one_net_lib/dist/Release/GNU-Linux-x86/libone_net_lib.a

dist/Release/GNU-Linux-x86/one_net_packet_cpp_cli: ${OBJECTFILES}
	${MKDIR} -p dist/Release/GNU-Linux-x86
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/one_net_packet_cpp_cli ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/chip_connection.o: chip_connection.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -I../one_net_lib/include -Iinclude -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/chip_connection.o chip_connection.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -I../one_net_lib/include -Iinclude -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/src/cli.o: src/cli.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -I../one_net_lib/include -Iinclude -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/cli.o src/cli.cpp

${OBJECTDIR}/src/string_utils.o: src/string_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -I../one_net_lib/include -Iinclude -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/string_utils.o src/string_utils.cpp

${OBJECTDIR}/attribute.o: attribute.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -I../one_net_lib/include -Iinclude -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/attribute.o attribute.cpp

${OBJECTDIR}/filter.o: filter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -I../one_net_lib/include -Iinclude -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/filter.o filter.cpp

${OBJECTDIR}/src/xtea_key.o: src/xtea_key.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -I../one_net_lib/include -Iinclude -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/xtea_key.o src/xtea_key.cpp

${OBJECTDIR}/time_utils.o: time_utils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -I../one_net_lib/include -Iinclude -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/time_utils.o time_utils.cpp

${OBJECTDIR}/src/packet.o: src/packet.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -I../one_net_lib/include -Iinclude -Isrc -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/packet.o src/packet.cpp

# Subprojects
.build-subprojects:
	cd /root/NetBeansProjects/one_net_lib && ${MAKE}  -f Makefile CONF=Release

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release
	${RM} dist/Release/GNU-Linux-x86/one_net_packet_cpp_cli

# Subprojects
.clean-subprojects:
	cd /root/NetBeansProjects/one_net_lib && ${MAKE}  -f Makefile CONF=Release clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
