#!/bin/bash

#
# generate_waveform.sh - Record from a PulseAudio sink monitor and
# plot the resulting waveform using sox and gnuplot
#
# usage:
#   - generate_waveform.sh alsa - Plot from ALSA sink monitor
#   - generate_waveform.sh null - Plot from NULL sink monitor
#
# notes:
#   - Before usage, set ALSA_SINK_NAME to system-appropriate value
#   - If not available, a NULL_5.1 sink is automatically created
#

set -o errexit
set -o nounset
#set -o xtrace

PACTL=../src/pactl
PACAT=../src/pacat
PAREC=../src/parec
PAREC_PARAMS="--no-remix --channels=6 --channel-map=surround-51 --file-format=wav"

NULL_SINK_NAME=NULL_5.1
NULL_SINK_MONITOR=${NULL_SINK_NAME}.monitor

ALSA_SINK_NAME=alsa_output.pci-0000_00_14.2.analog-stereo
ALSA_SINK_MONITOR=${ALSA_SINK_NAME}.monitor

OUTPUT_WAV_FILE=audio.wav
OUTPUT_DAT_FILE=audio.dat
OUTPUT_PLOT_FILE=audio.plot
OUTPUT_PNG_FILE=audio.png
GNUPLOT_SCRIPT=plot_audio.gp

RECORD_PERIOD=0.1s

usage() {
    echo "usage: $0 [null|alsa]" >& 2
    exit -1
}

[[ "$#" != "1" ]] && usage
case "$1" in
    "null") RECORD_SINK_MONITOR=$NULL_SINK_MONITOR;;
    "alsa") RECORD_SINK_MONITOR=$ALSA_SINK_MONITOR;;
    *) usage
esac

null_sink_exists() {
    $PACTL list sinks | grep --silent $NULL_SINK_NAME
}

create_null_sink() {
    echo "Creating NULL sink $NULL_SINK_NAME"
    $PACTL load-module module-null-sink sink_name=$NULL_SINK_NAME		\
         channels=6								\
         channel_map=front-left,front-right,rear-left,rear-right,front-center,lfe
}

null_sink_exists || create_null_sink

$PAREC $PAREC_PARAMS -d $RECORD_SINK_MONITOR $OUTPUT_WAV_FILE &
_pid=$!

echo "Recording audio, from $RECORD_SINK_MONITOR, for ${RECORD_PERIOD} .."
sleep $RECORD_PERIOD
kill $_pid

echo "Generating signal amplitude values .."
sox $OUTPUT_WAV_FILE $OUTPUT_DAT_FILE
#head -n8000 $OUTPUT_DAT_FILE > tmp	# In case of huge file, get only first 8000 samples
tail -n+3 $OUTPUT_DAT_FILE > $OUTPUT_PLOT_FILE	# Remove comments

cat > $GNUPLOT_SCRIPT <<EOF
set terminal pngcairo enhanced size 640,480 font 'Verdana,10'
set output "$OUTPUT_PNG_FILE"
set yrange [-1:1]
set ytics -1,0.1,1		# 0.1 jumps
set key outside

# FIXME: Revisit channel names.. we're not sure they're of
# the same order in the file
plot "$OUTPUT_PLOT_FILE" using 2 with lines title 'front-left', '' using 3 with lines title 'front-right', '' using 4 with lines title 'rear-left', '' using 5 with lines title 'rear-right', '' using 6 with lines title 'front-center', '' using 7 with lines title 'lfe'
EOF

echo "Plotting audio .."
gnuplot $GNUPLOT_SCRIPT

echo "Success! .. please check audio waveform at $OUTPUT_PNG_FILE"
