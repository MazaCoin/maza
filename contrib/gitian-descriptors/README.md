gitian-descriptors
==================

### Descriptor files

Multiple descriptors are provided
  * gitian-{linux,win,osx}.yml 
     - "standard" descriptors, full build
  * gitian-{linux,win,osx}-deps.yml 
     - Builds ONLY dependencies for caching for future builds
  * gitian-win{32,64}.yml
     - Builds win32 and win64 binaries separately
  * gitian-win{32,64}-deps.yml
     - Builds win32 and win64 dependencies seperately
  * gitian-osx-signer.yml
     - codesigning for OSX package

The multiple descriptors are provided to trim down time spent in each step 
of the build process when running on automated systems (with timeouts) such as travis-ci.org


### guruvan's notes on getting gitian builds running using Docker

In general, most modern CPU with virtualization capability will work to run gitian inside LXC from Docker
[gitian-docker](https://github.com/guruvan/gitian-docker) is known to run on 
a Stock AWS Ubuntu 15.10 image with Docker 1.9.1 installed
  - clone the above project
  - make a .env file, set variables as per examples
  - run:
  ```
  ./build.sh maza-release-v0,10.2.env all
  ```
  The above should simply run, and build MAZA for all available OSes (Windows32 Windows64, Linux, OSX) 
  Please report issues if the above fails
 
 This should also easily run in a VM with Ubuntu or similar installed, OR 
  with boot2docker on Windows or OSX. 
    - Standard boot2docker Vbox shared folders will not have correct permissions, umount & mount correctly
    



### Gavin's notes on getting gitian builds up and running using KVM:###

These instructions distilled from:
[  https://help.ubuntu.com/community/KVM/Installation](  https://help.ubuntu.com/community/KVM/Installation)
... see there for complete details.

You need the right hardware: you need a 64-bit-capable CPU with hardware virtualization support (Intel VT-x or AMD-V). Not all modern CPUs support hardware virtualization.

You probably need to enable hardware virtualization in your machine's BIOS.

You need to be running a recent version of 64-bit-Ubuntu, and you need to install several prerequisites:

	sudo apt-get install ruby apache2 git apt-cacher-ng python-vm-builder qemu-kvm

Sanity checks:

	sudo service apt-cacher-ng status  # Should return apt-cacher-ng is running
	ls -l /dev/kvm   # Should show a /dev/kvm device


Once you've got the right hardware and software:

    git clone git://github.com/bitcoin/bitcoin.git
    git clone git://github.com/devrandom/gitian-builder.git
    mkdir gitian-builder/inputs
    cd gitian-builder/inputs

    # Create base images
    cd gitian-builder
    bin/make-base-vm --suite precise --arch amd64
    cd ..

    # Get inputs (see doc/release-process.md for exact inputs needed and where to get them)
    ...

    # For further build instructions see doc/release-notes.md
    ...

---------------------

`gitian-builder` now also supports building using LXC. See
[  https://help.ubuntu.com/12.04/serverguide/lxc.html](  https://help.ubuntu.com/12.04/serverguide/lxc.html)
... for how to get LXC up and running under Ubuntu.

If your main machine is a 64-bit Mac or PC with a few gigabytes of memory
and at least 10 gigabytes of free disk space, you can `gitian-build` using
LXC running inside a virtual machine.

Here's a description of Gavin's setup on OSX 10.6:

1. Download and install VirtualBox from [https://www.virtualbox.org/](https://www.virtualbox.org/)

2. Download the 64-bit Ubuntu Desktop 12.04 LTS .iso CD image from
   [http://www.ubuntu.com/](http://www.ubuntu.com/)

3. Run VirtualBox and create a new virtual machine, using the Ubuntu .iso (see the [VirtualBox documentation](https://www.virtualbox.org/wiki/Documentation) for details). Create it with at least 2 gigabytes of memory and a disk that is at least 20 gigabytes big.

4. Inside the running Ubuntu desktop, install:

	sudo apt-get install debootstrap lxc ruby apache2 git apt-cacher-ng python-vm-builder

5. Still inside Ubuntu, tell gitian-builder to use LXC, then follow the "Once you've got the right hardware and software" instructions above:

	export USE_LXC=1
	git clone git://github.com/bitcoin/bitcoin.git
	... etc
