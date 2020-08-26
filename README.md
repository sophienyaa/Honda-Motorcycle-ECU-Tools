# Honda Motorcycle ECU Tools

Here is my work (so far...) on reading data from my Honda CRF250L via its on board diagnostics port.

## Hardware

There is a small red connector tucked up under the fuel tank on the RHS of the bike (sitting on it, facing forward). This is the diagnostic port. Its a 4 pin connector that has a small red cover on it.

The connector itself is a Sumitomo 6187-4441 and has the following 4 pins

1 - Ground
2 - Reset
3 - +12V
4 - K-Line

In order to read data from it, we need to connect to the K-Line. The K-Line is a single wire communicaton standard, it is esentially half-duplex serial (device can only send or recieve, not both at the same time)

Honda's implementation at a hardware level seems to be standard ISO9141, however it does not use a standard protocol. This means you can use a standard ISO9141 Interface chip, such as the L9637D (which is what I have used). You then need a microcontroller to read the data, as normal OBDII chips like the ELM327 are incompatable.

Here is an example circut for use with an Arduino or similar.

<circut diagram here>

## Software

### Protocol

