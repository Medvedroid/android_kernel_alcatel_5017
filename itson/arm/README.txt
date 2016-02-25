ItsOn Kernel Modules
====================

17 Feb 2015

This package contains the OEM deliverable of the ItsOn kernel modules.
These modules are delivered in a combination of source (.h and .h files)
and binary (.o files) form, with Makefiles and build scripts to allow
these to be built into the final kernel modules (.ko files).


Motivation
----------

The ItsOn kernel modules are the interface layer that allows the ItsOn
Service to control and account for data usage.  The kernel modules
implement the low-level access, control and accounting policies and
interact with the ItsOn Service APK.

Two modules are build: module 1, delivered as source code, interfaces
with Linux and provides platform abstraction for module 2.  Module 2
contains the data flow control and accounting logic.


Building
--------

To build the modules, use the supplied script build-kernel.sh.  This
script needs to be told the location of your kernel build tree, since
it uses the kernel build system to create the modules; and your Android
build tree, to find the correct toolchain to build the modules.

The kernel modules need to be built on a Linux system, in the same build
environment where your kernel is built.  The kernel will need to be
configured to enable loadable module support.

To build the modules, follow these steps:

1. Configure the build script, build-kernel.sh, in the section marked
   "CONFIGURATION SECTION".  Primarily you will need to ensure that the
   ITSON_UID setting is correct for your system, and that the
   ANDROID_VERSION and TOOLCHAIN are set correctly.
   
2. In the directory where build-kernel.sh lives (itson/<arch>), run
   build-kernel.sh, specifying the toolchain location and kernel build
   tree location, in that order.  The toolchain will typically be the one
   in your Android build tree; so, for example, if you have Android in
   /Development/android-5.0.2_r1 and the Android kernel in
   /Development/lolly-kernel/msm, you would use the command:

build-kernel.sh /Development/android-5.0.2_r1/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi- /Development/lolly-kernel/msm

3. The output from the build will be in itson/<arch>/out/user; the modules
   will be called itson_module1.ko and itson_module2.ko.

The built kernel modules must be integrated into your build before you
create the ROM for your device.
