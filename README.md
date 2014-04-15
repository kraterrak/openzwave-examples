openzwave-examples
==================

Small examples for openzwave

## Installation

### Linux

These examples require an installation of [openzwave](https://code.google.com/p/open-zwave/source/checkout) just run `make` and then `sudo make install` on the library.

From there, you can compile the examples. Simply navigate to an example and run `make`. Then run the example with `./<example_name>`

## Examples

### ozw-power-on-off
A simple example with a power switch as "Node 3". Change the code to the node that you would like to switch on/off. This program merely switches the node off, waits 5 seconds, then switches it on again. This is repeated 5 times.