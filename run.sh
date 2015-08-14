#
# Steps for running PulseAudio from within the git source tree
#
# Read and run the contents of `build.sh' script first!
#

LOG_DIR=build/logs
HOME_CONFIG_CLIENT_CONF=$HOME/.pulse/client.conf
AUTOSPAWN_NO=' *autospawn *= *no *'

#
# Even while PA modules actually exist under `build/src/.libs'
# instead of just `build/src', specificy the latter as the DL
# search path.
#
# Per the libtool documentation, libtool already adds the
# .libs/ suffix for us if we're running pulseAudio from within
# the source repository.
#
# It does this by applying some Unix trickery and by making
# `build/src/pulseaudio' actually a shell script wrapper
# around the real PA existing at `build/src/.libs/pulseaudio'!
#
PA=build/src/pulseaudio
PA_FLAGS="-n -F build/src/default.pa -p $(pwd)/build/src/ -vvvv"

run() {
	#
	# pulse-client.conf(5)
	# autospawn= Autospawn a PulseAudio daemon when needed.
	#
	touch $HOME_CONFIG_CLIENT_CONF
	grep "$AUTOSPAWN_NO" $HOME_CONFIG_CLIENT_CONF
	if [ "$?" != "0" ]; then
		echo "autospawn=no" >> $HOME_CONFIG_CLIENT_CONF
	fi

	# Assuming only user instances
	PULSE_PID=$(ps ux | grep pulseaudio | grep -v grep | awk '{ print $2 }')
	if [ x"$PULSE_PID" != x ]; then
		kill $PULSE_PID
	fi

	$PA $PA_FLAGS
}

#
# Did we forget to execute `build.sh'?
#
if [ ! -x $PA ]; then
	echo "In-place PA configuration and build not done"
	echo "Please run \`build.sh' first!"
	echo "Exiting ..."
	exit -1
fi

run 2>&1 | tee $LOG_DIR/RUN_LOG.txt
