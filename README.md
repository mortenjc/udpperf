# udpperf

A simple UDP transmitter and receiver with sequence numbers added
to the otherwise unused payload. This allows detection of packet loss
and calculation of packet error rates (PER).

The code is collected from two publicly available repositories.

CLI library header files comes from https://github.com/CLIUtils/CLI11

All other files are adapted from https://github.com/ess-dmsc/event-formation-unit

## Build

    > mkdir build
    > cd build
    > cmake -DCMAKE_BUILD_TYPE=Release ..
    > make

## Run
On one host

    > ./udprx

On another host

    > ./udptx
