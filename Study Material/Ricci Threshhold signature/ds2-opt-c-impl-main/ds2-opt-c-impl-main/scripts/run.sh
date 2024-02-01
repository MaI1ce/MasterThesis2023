#!/bin/zsh

for i in {1..$1}; do
    src/test_network $i $RANDOM &
    echo $RANDOM
done;

src/test_network 0 54321