## Intro

Hi all, welcome to this talk. I am [...]. Creator of TinyGo.

About six years ago I wanted to program an addressable LED strip using MicroPython, found it to be too slow, so decided to write a Go compiler for microcontrollers instead.

Now that programming LED strips in TinyGo works, I've worked on something bigger: a smartwatch programmed in Go using TinyGo. This has been a bit more ambitious than just a LED strip.

Oh and here is the result by the way

[photo]

[demo/camera]

Today I'm going to talk about how I made that, and why.

## Motivation

I've long wanted a smartwatch that is hackable, has a long battery life, and can actually be used as a daily watch. And while there are options for all three, I've found that none of the available options really fit my goal. So I made my own.

This is the PineTime, by Pine64 who are also around at FOSDEM. I'm not related to Pine64 in any way though, I'm just using their hardware.

When I started adding support for the PineTime in TinyGo, I realized I could actually use this watch as reference hardware for TinyGo support: the hardware is exactly the kind that is usually in TinyGo projects: it's got a typical chip with Bluetooth Low Energy, a small screen, an accelerometer, a heartrate sensor, and an external flash chip.

[hardware]

The microcontroller in there is pretty weak like most microcontrollers. In fact this is a relatively powerful microcontroller: it even has a floating point unit which many microcontrollers lack. Only for float32 though, there is no hardware support for float64.

The default firmware is called InfiniTime and it is written in C++, which is very typical for this sort of project. But of course, I wanted to write firmware in Go, not in C++.

I wanted to use this project as a way to improve TinyGo overall, by not just making it work but make it work _well_. There are two sides to this: the developer side and the user side. And I wanted to improve both.

### Developer productivity

First on the developer side, I wanted to make things as easy and productive as possible. That means really good simulation of the hardware. So I built a simulator.

[simulator-images]

[demo] run simulator, connect Gadgetbridge, show step increments

Bluetooth works like this thanks to the fact that we have a cross-platform Bluetooth package as part of TinyGo, tinygo.org/x/bluetooth, which supports Linux, the chip in this watch, and a number of other platforms. I'm sure Ron Evans will talk a bit more about this in the next talk.

This is done through a board package, which I've developed.

It's actually a hardware abstraction layer at the same time, supporting the following boards:

[boards]

Yeah, I've even added support for the GameBoy Advance if anybody needs that.

The badge I have here actually runs on top of the same board package. In fact, because of the hardware abstraction layer I could run the badge firmware on the watch or the watch firmware on the badge, though it won't work very well because the watch assumes a touchscreen and the badge is mostly used through physical buttons.

My end goal with this simulator is that most development can happen on a regular computer with a fast edit/compile/test cycle. The only times the actual hardware is needed is when working on device drivers for things like the display or sensors, or when testing performance.

Many simulators need special support from the firmware, to be built in a special way. For the board package, the only part that I use (that is optional) is to set a few parameters. For the PineTime, it looks like this:

[simulator-params]

Features:

  * Size of the screen

As you can see, it's possible to set the speed of the display. This makes the simulation more realistic, as many devices (like this smartwatch) have a rather slow display. It doesn't simulate the CPU speed, for that you'll really have to test on the device itself.


Because the hardware is abstracted, it is often possible to test the code on different hardware than the hardware that you're actually targeting, for example because it is easier. For example, here is the Adafruit PyPortal, which has a touch screen and accelerometer. The benefit is that it can be programmed using a simple USB cable, instead of a special programmer and a dodgy four wire connection directly to the very sensitive watch PCB.

[demo]

### User experience

The other side of polish is the user side, and make it work as well as a commercially available watch. In particular, that means:

  * Stable
  * Fast and responsive
  * Efficient - long battery life
  * Secure
  * Extra:
    * translation, localization
    * Good UI/UX

These things are usually ignored in a prototype but before you can actually sell some hardware you really need to have these things.

Let's go through some of these.

#### Battery life

When I started working on this watch I first focused on battery life. It's been my experience that it's much easier to do that first and only add features later.

[battery-life-list]

To give you an idea of the things that needed fixing to get good battery life:

  * Serial port - enabled by default
  * DCDC
  * Heart rate sensor not in lowest setting
  * External flash memory not in sleep mode
  * Read buttons by polling instead of enabling the reading all the time. Apparently these buttons need to be enabled before they can be read, and quickly enabling them before reading is actually more efficient than leaving them enabled all the time.
  * Bluetooth advertisement / connection interval

This is just some of the things that you'll find when working on firmware. Actually some of these things weren't known to InfiniTime. I shared my findings with the InfiniTime people so now InfiniTime is much more efficient, though still not as efficient as the firmware I wrote.

All of these things are part of the board package I wrote, neatly abstracted away. So if any of you would like to write your own firmware for the PineTime it's all ready to use.

## Display

The next thing I worked on was making the display fast and responsive. This was actually a lot of work to get right.

To understand what is needed, I need to explain some of the hardware.

### Display hardware

As I mentioned before, the PineTime has a nice high-resolution screen with its own memory that's connected through a very slow serial connection.

[display-hw]

The display has 240x240 pixels, which are typically used with 16 bits per pixel. The connection between the main microcontroller and the display runs at 8Mbit/s, which, if you do the math, means you have a framerate of about 8.5. Which is really quite slow.

This kind of displays are really common though in embedded systems because they're also cheap - though usually have a somewhat faster connection.

There are a few ways to speed up the display though:

  1. The display can also be used in a 12 bits per pixel mode. This means graphics don't look as nice, but in practice it's good enough for a smartwatch. This gets us to 11.5fps.
  2. You can update just a rectangle on the screen, instead of the whole screen.
  3. The display has special support for fast scrolling, where you can rotate a part of the screen and only need to update the part that got moved into view.
  4. The microcontroller has DMA support. DMA is like a separate core or coprocessor with the sole purpose of copying data from one place to another and runs parallel to the main core. It can be used to send a block of pixels to the screen while the main processor is busy rending the next block of pixels.

I've all of them except for the last, because we haven't agreed on a nice API for it yet. The trouble is that the hardware behaves a lot like the async/await pattern you see many languages, but the async/await pattern isn't very Go style. But I'm sure we'll be able to figure something out.

### Driver interface

The way this kind of hardware is mapped in Go is with the following API:

[driver-display-api]

The main method here is the `DrawBitmap` method, which has a X and Y coordinate and a pixel buffer.

The pixel buffer works a lot like a Go slice, except for pixels and in two dimensions instead of one. You can set and get pixels at particular coordinates, slice it to have a different width and height, and all the elements are pixels.

Some remarks on the design:

  * I initially started out using a plain Go slice, but needed to use something more special because of the weird 12 bits per pixel format. The nice thing is that this API is now better prepared for other weird pixel formats, like:
    * 1 bit per pixel for e-ink and some oled screens.
    * I've seen a display that uses a special 9 bit per pixel format.
    * Many of these screens actually support 18 bits per pixel, though I don't think this feature is commonly used. With this `Image` abstraction, it's fairly easy to support this format if a higher bit depth is needed.
  * This design may be a bit of an abuse of Go generics. Something like this would normally be done through interfaces instead of generics. However, using generics means that the code is in fact as performant as it would have been if it were written for a particular pixel format, except that a whole program can be changed by changing one or two lines of code.

There is also the special scroll feature, which is not directly available but can be made available using a simple Go type assert. This means that most code can ignore this feature except for the part that actually cares about scrolling.

I've added the same special scrolling support to the simulator, which means I could test the scrolling feature directly in the simulator. Which is important, because the way scrolling works makes my head hurt and it's very useful to have quick feedback instead of also needing to fiddle with hardware.

### TinyGl

Ok so now we have a nice base to build upon, but we need some sort of widget toolkit to actually draw something on the screen. I looked at several options but didn't find anything that really satisfied my needs. So I wrote my own.

  * The most well known open source library for this purpose is LVGL. It's a C library with lots of features to make nice displays, and is used by InfiniTime. It has a number of problems though:
      * It's written in C, which means that the entire library needs to be wrapped in CGo. Which is possible, but kind of frustrating.
      * More importantly, it requires a ton of compile time configuration through macros. However you're going to wrap this library, you're going to need to make assumptions. For example, about the pixel format of the displays you're going to use.
      * It only supports a limited number of pixel formats. This includes the common 16 bits per pixel format, but doesn't include RGB444 or other exotic pixel formats.
  * Adafruit has some C++ libraries for working with displays. They're nice, but assume that displays can be updated per pixel which is very slow. They also assume a fixed 16 bits per pixel format, which as we've seen is far from the only format out there.
  * TinyGo itself has the tinydraw and tinyfont packages, which are nice and Go-like but have the same limitations as the Adafruit libraries.

Therefore, I looked into writing my own, which I've called TinyGl. It works really well together with the board package though they are independent packages.

At the moment it has the following features:

  * Text label
  * Vertical box
  * List box
  * Scroll container
  * Canvas, with rectangle, circle, line, text, and images
  * Custom widget

All these track when they've been updated and will only draw the parts that changed, as far as possible. For example, a text box is redrawn when the text, background, or the container size changed. The canvas tracks changed areas in blocks of 16x16 pixels.

All these shapes are antialiased. Perhaps not as well as on desktop computers, but even a little antialiasing goes a long way to make a UI look a lot better.

The most difficult part of this was probably the line, which I've used in the analog watch for example. A line might seem simple but if you think about it, a line is really just a rectangle. The easiest way I've found to draw it is by turning it into a polygon and drawing the polygon.

I started working on a blog post on how I did the polygon which I hope to finish soon. Follow me on Mastodon if you want to know when I post it! But in short, the way it does antialiased rendering is based in part on a technique that was originally developed for a Star Trek movie, the one with the genesis device. This shows again that so many good computer science techniques were developed decades ago.

## Future

There are still many things that I'd like to implement that will benefit TinyGo as a whole. These are:

  * Bluetooth secure pairing. I've kind of avoided security before, because it's simply not yet implemented. Right now the Bluetooth package supports receiving connections, but it doesn't support any of the security or privacy properties of Bluetooth. This means using random MAC addresses and bonding so that the connection can't be tracked or intercepted.  
    I actually care a lot about security though, so this is definitely something I will investigate in the future.
  * Internationalization/localization. This is usually ignored in prototypes, but is really important if you want people to actually use your hardware outside your own language. But I'll need to see what's out there and how well they would fit in the small memory footprint of typical microcontrollers.

Anyway that's all I have. Anybody with questions?



---


In particular, it has a difficult to use C based toolchain. Of course, I'd like to use Go instead!

  * Not so great development experience (C toolchain, emulator hard to use)

Started writing my own in Go. But realized I could use this as an opportunity for improving TinyGo as a whole.

I wanted a device that has all the polish of a commercial product. Means it doesn't just work, but also:

  * Stable (no crashes, freezes, etc)
  * Fast - as fast as the hardware allows
  * Efficient - long battery life
  * Secure (for example BLE)
  * Extra:
    * translations, localization
    * Good UI/UX - again, as good as is possible on the hardware

And good emulation: unless you're writing device drivers or testing performance, there should be no need to test on device.

In the end the vast majority of the work I did was reusable. And that's going the main topic of this talk.

## Hardware

First let me explain what hardware we're dealing with.

  * Cortex-M4: 32-bit ARM, limited instructions, float32 but not float64
  * 64MHz CPU
  * 64kB RAM
  * 512kB executable flash, 4MB flash in external chip (not executable)
  * Display: 240x240 pixels, 12/16 bits/pixel  
    That means a framebuffer takes up about 84kB, larger than the entire RAM!  
    Display has its own RAM, but the connection is very slow: about 11fps at most
  * BLE support (no classic, so no music)
  * Various sensors

So this is quite limited, but typical for this kind of smartwatch.

## Harware abstraction

My goal with this project wasn't just to write a lot of code for this particular watch.
(board package...)

I could just write a one-off simulator, but could also make it generic.
Only bit of code that is simulator specific sets a few parameters! (Width, height, PPI, display speed, etc) - no build tags needed.

Also: Bluetooth!

## Display shenanigans

Display is important for UX so had to get the basics right.

Write software to fit the hardware, not the other way round.

  * Use VSync (hard on this particular hardware)
  * Only update parts that changed.
  * Use 12 bits/pixel for efficiency  
    8.5fps -> 11.5fps
  * Big buffer at a time, due to overhead. 32 lines at a time.

Required a new API for TinyGo.

  * DMA!

Like a coprocessor that copies data in the background, very fast (sometimes faster than doing it manually).

Still need to figure out a good API for TinyGo, WIP:

  * Async style API fits hardware best (send + flush), but not very Go style.
  * Alternative is hard to get right -> wrong priorities.

Scroll hardware! (explanation...) Use Go type assert on the driver to check available.

Many commonly used displays work like this, with some variations:

  * Line updates instead of blocks
  * Sometimes in-memory buffer
  * E-paper is special

## TinyGo improvements

  * RTT
  * Fast graphics API

## Widget toolkit

* basic widgets: text, vbox, etc
* custom widget
* canvas (rect, circle, line, image, future: polygon)
* image - various formats (no PNG due to RAM)
* font rendering

(some explanation on antialiased polygon rendering) -> check Mastodon for blog post

Not LVGL:

  * C
  * doesn't support RGB444
  * ...


## Future goals

 * Features: weather, activity tracking, sleep tracking, music control, etc
 * Nice & fast UI
 * Mature TinyGL
 * Secure/private BLE
