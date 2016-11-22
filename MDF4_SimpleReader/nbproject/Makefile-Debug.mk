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
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/7daaf93a/md5.o \
	${OBJECTDIR}/_ext/7daaf93a/mdFile.o \
	${OBJECTDIR}/_ext/7daaf93a/mdf4.o \
	${OBJECTDIR}/ImportMdf4.o \
	${OBJECTDIR}/MDF4_SimpleReader.o \
	${OBJECTDIR}/stdafx.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mdf4_simplereader

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mdf4_simplereader: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mdf4_simplereader ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/7daaf93a/md5.o: ../Common/md5.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/7daaf93a
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../Common -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7daaf93a/md5.o ../Common/md5.cpp

${OBJECTDIR}/_ext/7daaf93a/mdFile.o: ../Common/mdFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/7daaf93a
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../Common -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7daaf93a/mdFile.o ../Common/mdFile.cpp

${OBJECTDIR}/_ext/7daaf93a/mdf4.o: ../Common/mdf4.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/7daaf93a
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../Common -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7daaf93a/mdf4.o ../Common/mdf4.cpp

${OBJECTDIR}/ImportMdf4.o: ImportMdf4.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../Common -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ImportMdf4.o ImportMdf4.cpp

${OBJECTDIR}/MDF4_SimpleReader.o: MDF4_SimpleReader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../Common -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MDF4_SimpleReader.o MDF4_SimpleReader.cpp

${OBJECTDIR}/stdafx.o: stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../Common -I. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/stdafx.o stdafx.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mdf4_simplereader

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
