Power Control Algorithms based on Contiki 2.7
============================


This repository implements several power control techniques for WPAN based on Contiki 2.7. The code resides in /examples/sky/tx-adjust.c. Each node runs the tx-adjust program and it can automatically adjust the TX power to the optimum. It can be used for networks with different topologies such as star, mesh, P2P.

To run the program:

cd /examples/sky
make TARGET=sky tx-adjust
make tx-adjust.upload

