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
	${OBJECTDIR}/src/one_net_peer.o \
	${OBJECTDIR}/src/one_net_features.o \
	${OBJECTDIR}/src/one_net_timer.o \
	${OBJECTDIR}/src/one_net_acknowledge.o \
	${OBJECTDIR}/src/one_net_crc.o \
	${OBJECTDIR}/src/one_net_application.o \
	${OBJECTDIR}/src/one_net_xtea.o \
	${OBJECTDIR}/src/one_net.o \
	${OBJECTDIR}/src/one_net_port_specific.o \
	${OBJECTDIR}/src/one_net_packet.o \
	${OBJECTDIR}/src/one_net_prand.o \
	${OBJECTDIR}/src/one_net_encode.o \
	${OBJECTDIR}/src/one_net_message.o


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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Release.mk dist/Release/GNU-Linux-x86/libone_net_lib.a

dist/Release/GNU-Linux-x86/libone_net_lib.a: ${OBJECTFILES}
	${MKDIR} -p dist/Release/GNU-Linux-x86
	${RM} dist/Release/GNU-Linux-x86/libone_net_lib.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libone_net_lib.a ${OBJECTFILES} 
	$(RANLIB) dist/Release/GNU-Linux-x86/libone_net_lib.a

${OBJECTDIR}/src/one_net_peer.o: src/one_net_peer.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_peer.o src/one_net_peer.c

${OBJECTDIR}/src/one_net_features.o: src/one_net_features.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_features.o src/one_net_features.c

${OBJECTDIR}/src/one_net_timer.o: src/one_net_timer.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_timer.o src/one_net_timer.c

${OBJECTDIR}/src/one_net_acknowledge.o: src/one_net_acknowledge.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_acknowledge.o src/one_net_acknowledge.c

${OBJECTDIR}/src/one_net_crc.o: src/one_net_crc.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_crc.o src/one_net_crc.c

${OBJECTDIR}/src/one_net_application.o: src/one_net_application.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_application.o src/one_net_application.c

${OBJECTDIR}/src/one_net_xtea.o: src/one_net_xtea.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_xtea.o src/one_net_xtea.c

${OBJECTDIR}/src/one_net.o: src/one_net.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net.o src/one_net.c

${OBJECTDIR}/src/one_net_port_specific.o: src/one_net_port_specific.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_port_specific.o src/one_net_port_specific.c

${OBJECTDIR}/src/one_net_packet.o: src/one_net_packet.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_packet.o src/one_net_packet.c

${OBJECTDIR}/src/one_net_prand.o: src/one_net_prand.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_prand.o src/one_net_prand.c

${OBJECTDIR}/src/one_net_encode.o: src/one_net_encode.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_encode.o src/one_net_encode.c

${OBJECTDIR}/src/one_net_message.o: src/one_net_message.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -I/root/NetBeansProjects/one_net_lib/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/one_net_message.o src/one_net_message.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release
	${RM} dist/Release/GNU-Linux-x86/libone_net_lib.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
