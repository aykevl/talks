Hi everybody
I'm Ayke and I work on TinyGo. I started this project almost two years ago because I wanted to program microcontrollers using Go instead of C or C++. Or Python which is too slow for the things I want to do.

So, a year ago I was at the CCC in Germany and I saw someone with a cube like this.
[image: CCC]
Obviously I wanted to build one myself. But I also wanted to drive it with a microcontroller instead of a Raspberry Pi and an FPGA.
[image: hub75]
The panels used here come from AliExpress. They are normally used for very large screens on festivals and the like. They're called hub75, for some reason.

And with some math I figured out it should be possible if I used slightly smaller panels, and use a fast microcontroller. This one is running at 12MHz, which is considered high performance.

Driving these panels is not easy. They have very little hardware support. There are two main limitations.

  * [image: SparkFun rows]
    The first limitation is that you can have only two rows on at a time. So you have to switch between rows fast enough that the eye won't notice.
  * [image: 8 colors]
    The second limitation is that LEDs can only be on or off, you can't directly dim them. That leaves 6 or 8 colors, depending on how you count.


  * [image: BCM]
    You can only turn LEDs on or off, so you have to turn them on and off quickly enough that it appears to be varying in brightness. This is done using BCM, which is very much like PWM, for those familiar with that. The trick is that with BCM, you only need to update the whole screen 8 times per frame instead of 256 times for the same perceived brightness per pixel.

So how do you actually drive them? Turns out most microcontrollers have special hardware that can be used for this purpose:

  * SPI, driven by DMA. To send all the color data, while the CPU is doing something else.
  * Precise timer, to turn the screen on and off for a very precise time. Again, the CPU can do something else while waiting for the output to go low again [or high?].

Will say something more about DMA.


# DMA

Direct Memory Access
This is a way of quickly moving bytes between different parts of the system, by letting dedicated hardware pull the data direct out of main memory. This is how disk drives and network card get their high speeds: by letting this hardware read/write directly from or to RAM.
Something similar happens in microcontrollers, although the so-called SPI peripheral I use for this cube is much simpler and is deeply integrated in the microcontroller.

The whole point of me writing this driver (apart from nice LEDs) was testing out the limits of the TinyGo compiler and standard library. As it turns out, with some performance optimizations it actually runs really fast. However, the SPI peripheral I'm using does not yet integrate well in the Go language, this is something I'm still working on.

But there is definitely a nice advantage of using Go here. Most of these animations are based on so-called Simplex noise, which is a newer version of Perlin noise.

Whole buffer for one bitplane: 144 bytes at 24MHz gets sent in 48µs

---


There is a micrcontroller inside running at a blazing 120MHz. That's actually pretty fast for a microcontroller.
Recently I've been working on this LED cube, in part to test the limits of the compiler. And as it turns out, it actually works!
