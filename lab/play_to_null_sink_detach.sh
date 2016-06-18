PAPLAY=../src/paplay

if ! ps aux | grep $PAPLAY | grep -v grep; then
    $PAPLAY --file-format=wav -d NULL_5.1 sine.wav &
fi

sleep 0.1s
