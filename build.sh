#
# Steps for configuring and compiling PulseAudio to run
# directly from the git source tree. This is useful for
# day-to-day PA developers not to mess with the
# distribution-installed PulseAudio instance and configs.
#
# Detailed steps taken from Colin Guthrie's blog article:
#
#	Compiling and running PulseAudio from git
#	http://colin.guthr.ie/2010/09/compiling-and-running-pulseaudio-from-git/
#

LOG_DIR=logs

build() {
    NOCONFIGURE=true ./bootstrap.sh -V

    ./configure --prefix=$(pwd)

    #
    # "Due to an intltool upstream bug, the translations for
    # .desktop files are not written if the destination folder
    # does not exist" --coling
    #
    mkdir -p src/daemon
    make -j 30

    # Mixer profile definitions
    mkdir -p share/pulseaudio
    ln -s ../../../src/modules/alsa/mixer share/pulseaudio/alsa-mixer

    ln -s pacat src/paplay
    ln -s pacat src/parec
}

mkdir -p $LOG_DIR
build 2>&1 | tee $LOG_DIR/BUILD_LOG.txt
