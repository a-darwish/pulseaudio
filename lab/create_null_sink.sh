#!/bin/bash

PACTL=../src/pactl
NULL_SINK_NAME=NULL_5.1

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
