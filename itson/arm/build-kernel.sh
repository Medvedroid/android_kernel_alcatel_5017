#!/bin/bash
set -e

J="-j4"

if [[ $# < 2 || $# > 7 ]]; then
  echo "Usage: `basename $0` <toolchain> <kernel-dir> [ <out-dir> <itson-uid> <build-type> <naming-convention> <android-version> ]"
  exit 1
fi

BASEDIR="$( dirname `readlink -e "${BASH_SOURCE[0]}"` )"
ITSON_SRC="${BASEDIR}"
ITSON_VERSION="${BASEDIR}/version"

export ARCH="arm"
export SUBARCH="arm"

##############################################################################
# CONFIGURATION SECTION

# The first parameter is the location of your Android toolchain; for
# example, /Development/android-5.0.2_r1/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-
TOOLCHAIN=${1}

# The second parameter is the location of your Android kernel build tree;
# for example, /Development/lolly-kernel/msm
KERNEL_DIR=${2}

# Configure OUT_DIR to the directory where you want the build modules
# placed; or pass it as the third parameter.
OUT_DIR=${3:-${BASEDIR}/out}

# Configure ITSON_UID to be the user ID for the ItsOn application; or
# pass it as the fourth parameter.
ITSON_UID=${4:-1500}

BUILD_TYPE=${5:-user}
NAMING_CONVENTION=${6:-system}
ANDROID_VERSION=${7:-XXX}

# END CONFIGURATION SECTION
##############################################################################

# Make sure that OUT_DIR is an absolute path.
if [[ "${OUT_DIR}" != /* ]]; then
   OUT_DIR=${BASEDIR}/${OUT_DIR}
fi

USER_ARGS="TARGET_BUILD_VARIANT=user DO_PURGE_LOG=1 ITSON_UID=${ITSON_UID}"
ENG_ARGS="ITSON_UID=${ITSON_UID}"

if [[ "$BUILD_TYPE" == eng* ]] || [[ "$BUILD_TYPE" == "userdebug" ]]; then
  #echo "Using eng args"
  ARGS=$ENG_ARGS
else
  #echo "Using user args"
  ARGS=$USER_ARGS
fi

if [ "$ARCH" == "arm64" ]; then
  ARGS="${ARGS} CFLAGS_MODULE=-fno-pic"
fi


# Start out in the kernel directory
cd ${KERNEL_DIR}

echo "Building in ${KERNEL_DIR}..."
echo "   BUILD_TYPE [${BUILD_TYPE}] NAMING_CONVENTION [${NAMING_CONVENTION}] ARCH [${ARCH}] TOOLCHAIN [${TOOLCHAIN}] ARGS [${ARGS}]"

#echo ""
#echo ">>> make ${J} ARCH=${ARCH} CROSS_COMPILE=${TOOLCHAIN} modules_prepare"
#make ${J} ARCH=${ARCH} CROSS_COMPILE=${TOOLCHAIN} modules_prepare

echo ""
echo ">>> make ${J} ARCH=${ARCH} CROSS_COMPILE=${TOOLCHAIN} ${ARGS} M=${ITSON_SRC}"
make ${J} ARCH=${ARCH} CROSS_COMPILE=${TOOLCHAIN} ${ARGS} M=${ITSON_SRC}


echo "Renaming..."
ARCHIVE_DIR="${OUT_DIR}/${BUILD_TYPE}"
mkdir -p ${ARCHIVE_DIR}
if [[ "${NAMING_CONVENTION}" == "original" ]]; then
	cp -a ${ITSON_SRC}/itson/module1.ko             ${ARCHIVE_DIR}/module1.ko
	cp -a ${ITSON_SRC}/itson/itson_logic/module2.ko ${ARCHIVE_DIR}/module2.ko
elif [[ "${NAMING_CONVENTION}" == "versioned" ]]; then
	cp -a ${ITSON_SRC}/itson/module1.ko             ${ARCHIVE_DIR}/itson_module1-${ANDROID_VERSION}.ko
	cp -a ${ITSON_SRC}/itson/itson_logic/module2.ko ${ARCHIVE_DIR}/itson_module2-${ANDROID_VERSION}.ko
elif [[ "${NAMING_CONVENTION}" == "system" ]]; then
	cp -a ${ITSON_SRC}/itson/module1.ko             ${ARCHIVE_DIR}/itson_module1.ko
	cp -a ${ITSON_SRC}/itson/itson_logic/module2.ko ${ARCHIVE_DIR}/itson_module2.ko
else
	echo "${NAMING_CONVENTION} is not a valid naming convention"
	exit 1
fi
cp -a ${ITSON_SRC}/kernel.api             ${ARCHIVE_DIR}/kernel.api

