# hello
I am Ayke and I'm working on TinyGo.
You may have heard about TinyGo already. It's a new Go compiler like gccgo and the main Go compiler, but it specializes in that binaries are much smaller and that it uses less memory. These properties allow it to be used in places where Go was impractical or impossible before, such as small embedded systems and WebAssembly.

Therefore, it is not meant to replace any of the existing Go compiler, but rather is a complement to them.

In this talk I'm going to focus on embedded systems, not on WebAssembly. I'll start with some history, give some insight into how it works, the current status and what's next for TinyGo.

# Example

Here is an example. This is a very small program, that simply turns an LED on and off. This is like the "hello world" of hardware. The program configures the pin as an output pin, and then turns it on and off in a loop.

As you can see, TinyGo adds a new package to the standard library: the machine package. It implements low-level but portable access to hardware. It already has some things declared as a constant, for example here you can see the `machine.LED` constant declared. Most development boards include one or multiple LEDs that TinyGo will expose in this way.

The machine package will also provide access to other things which embedded programmers are familiar with. These are SPI and I2C for communication between chips on a board, I2S usually used for transferring sound, and of course UART which can be used to transfer log messages from the microcontroller to the host computer for example.

# embedded?

First, let's get some definitions out of the way. What does embedded mean? I personally define it as anything that runs code but does not look like a computer. This is a very broad range of devices.

A more specific category of embedded devices is microcontrollers. These are small programmable chips with a CPU, memory, and a huge range of peripherals to interact with the real world. Programs usually run directly from flash instead of being loaded into RAM first. And while they may be relatively fast, they are extremely constrained on memory.

# some history

First some history.
I started looking into Go on embedded systems back when I was still using MicroPython. MicroPython is an interesting project that allows you to run Python on embedded devices which would normally be unable to run an interpreter. But while controlling some RGB LED strips, I was running into speed limitations, which I already kind of expected. I could write MicroPython modules in C but that kind of defeated the purpose of using an easy scripting language. So, I started looking into running Go on these chips.

My first attempt was to use gccgo. GCC is normally used to compile C like languages but it can in fact also compile Go in the form of gccgo. I realized that I could enable the gccgo compiler in the toolchain I was using for the ESP32 (the chip I was using at the time). Surprisingly, this worked. I managed to get a LED to blink, but didn't go much beyond that: I hit several limitations with GCCGO.

So I did the naïve thing and tried how far I'd get writing an actual Go compiler. Pretty far, it turns out. Over the last two and a half years the project has gone from a small proof-of-concept to a whole community around this compiler with contributions from many different people.

The project includes now not just a compiler, but also a repository of over 50 device drivers and a Bluetooth package that has support for Linux, macOS, and some embedded chips. And a very active Slack channel which you're welcome to join, if you want.

# Other projects

But before I wrote my own compiler, I searched the internet for something similar. I found a few project, but none really fit my needs.

- The first one I found was emgo. What emgo does is convert Go to C and compile that. Unfortunately, while it keeps the Go syntax it uses the C memory model so for example you can't return a pointer to a local variable: something that's allowed in the Go spec. This means the language is incompatible with the wider Go ecosystem.
- Other than that, I found various projects that attempt to use Go as a kernel language. Most notably gopher-os, which was presented at Golang UK 2017 but appears to be dead now.

Since then, a new and interesting project has emerged: TamaGo, which was presented here at GoLab earlier this day and also allows running Go bare metal. However, it is in fact quite different from TinyGo: it is a modified version of the Go compiler instead of being a new compiler and runtime. This allows it to be more compatible with the rest of the Go ecosystem, but it also means that it limits itself to much more powerful system-on-chips like the Raspberry Pi, which has orders of magnitude more RAM than some of the chips that TinyGo supports. In short, it's a different compiler with different goals, although it looks superficially similar.

# How it works

So, how does TinyGo work?
TinyGo is not a compiler written from scratch, it uses many existing packages. For example, it uses the same packages used by Go tools and IDEs.
If you think about it, what an IDE does to give you autocomplete and inline errors is already half of a compiler. To be able to give good suggestions, it needs to parse and typecheck the source code.
And in fact this is what TinyGo uses for the first half of the compiler, the so-called frontend.

Of course, a compiler is much more than that and that's where a project called LLVM comes in.
What's LLVM, you may ask?
Well, LLVM is a toolkit for compilers. If you know the Gorilla toolkit for web applications, it's like that but for compilers. It provides all the language independent things a compiler needs to do, in particular language independent optimizations and generating machine code. Many well-known compilers such as Clang, and the Rust and Swift compilers are based on it. LLVM also provides extra tools such as a linker. All these things are used by TinyGo and are one reason it can produce these small binaries for so many different platforms.

Apart from just a compiler, TinyGo also replaces the runtime package. This is where a large part of the code size changes come from, as you'll see next.

# Reducing binary sizes

So, how does TinyGo lower binary sizes so much?

There are a few reasons to this, which I'll explain. In short, it's a smaller garbage collector, whole-program optimization, and a different way of doing package initialization.

## Garbage collector

Okay first the garbage collector. In Go, there is no explicit way of freeing memory. And while the specification is silent on this, in practice implementations will use a garbage collector.
Most tracing garbage collectors work in two phases.
The first phase marks all reachable objects as reachable. It looks at all global variables and the whole call stack of each goroutine, so each function that is currently running and all functions that will eventually be returned to. And then it continues marking all these variables recursively, so for example for each pointer, slice, map or channel it looks at the values they point to.
The second phase goes through all the objects on the heap and frees all objects that were not marked in the previous phase.

As you can imagine, this is quite expensive. If you'd have a naïve implementation, you'd need to stop all goroutines, do the scan, free all objets, and then resume all goroutines. This is called a stop-the-world, as the program is stopped entirely.
The Go runtime is smarter than this. It does almost all of this in parallel and only very briefly stops each goroutine to scan its stack, but otherwise the program continues running.

For TinyGo, this is much too complex: it costs a lot in code size to do this. It also doesn't need all this complexity.

TinyGo uses a very simple implementation in which it simply continues allocating from the heap until it runs out of space, and then it runs the garbage collector. It can of course use all available memory as it is the only program that's running. No need to worry about swapping or an operating system that might kill the process when it runs out of memory.

This garbage collector design is almost a copy of the garbage collector in MicroPython. The implementation is different but I decided to copy the design because I was already familiar with it.

## Whole program optimization

Next up is whole program optimization. TinyGo compiles and optimizes a whole program at once. This means we can get all the benefits of link-time optimization, and some more.

### interfaces

One optimization allowed by this are much more efficient interfaces.

Interface methods in Go are normally implemented in a way that's almost identical to C++ virtual methods.

TinyGo however implements them in a very different way. Imagine you have code like is shown on the slide.

TinyGo transforms the code to basically a type switch. This is only possible because it knows all the possible implementations of Stringer and just checks them all.

#

This might seem like it increases code size, however it also allows many optimizations. For example, it becomes possible to perform escape analysis across interface method calls, to reduce pressure on the garbage collector. It also allows inlining functions if there is only one caller. This means that if there is only one implementation of an interface, all interface method calls will be converted to direct calls, thus making interfaces less expensive.


Unfortunately, these kind of whole program optimizations are very slow to compile, especially for larger programs. Therefore, I hope to get a more traditional LTO setup in place in the future which should keep most of the benefits of whole-program optimization while increasing compilation speed.

## package initializers

The last optimization I'm going to talk about are package initializers.

Say you have a global map like you can see here.
You might expect this to be set at compile time, after all all the values are already known and the compiler could just dump a ready-made map in the binary.
Unfortunately, this is not the case.

#

This is how the compiler sees it. As you can see, the map is not initialized at compile time but at runtime. There are technical reasons for this in the main Go toolchain, but for TinyGo this behavior is a big problem as this initialization code cannot be optimized away even if the map is never used.

Therefore, TinyGo implements partial evaluation of package initializers.

This means it tries to run all the initialization code at compile time, including map initializers. If it finds a function it cannot interpret at compile time (for example, if it uses some form of I/O) it runs this function at runtime.
This is difficult to get right and I am aware of bugs in the current implementation, but it is maybe the biggest reason why TinyGo binaries can be so much smaller, therefore it is worth the investment.


There are more optimizations but these are some of the main ones.


# Current status

So how close are we to a usable Go compiler?

That depends on your use case. It is able to compile most Go code unmodified. However, the big caveat is "most" - many programs won't be able to compile because a dependency of a dependency somewhere uses a standard library package that is not yet supported. Over time, more and more of the Go standard library will become supported while we keep binary size small so this issue should disappear over time.

That said, it already works really well for many smaller programs, which I'll show at the end of this talk.

# Upcoming features

There is one thing which I would like to announce here which I haven't talked about much. You might know that TinyGo now supports IDE integration. But I'm working on making the IDE integration much, much better. See here.

This is a Go program written for TinyGo. As you can see, it blinks an LED like before but this time it does that in a new goroutine.

Further, it also controls an LED strip.

The program here animates some LED strips, by first picking a color using a noise function. Noise functions are often used in computer generated graphics. You could consider this strip a very small image of just 8 pixels total, the 8 LEDs.

After calculating the colors, it writes them to the LED strip and waits a few milliseconds before doing the next update.

Let's flash it to the board to see what it does.

[...]

Okay, great, it works.
However, flashing to a board can be inconvenient during development. Some boards are slow to program or require manual action to program. So wouldn't it be nice to see what would happen during development?

That's where the preview feature comes in. You might have noticed this button here.

It compiles...

And it runs! This is the same program as on the left but running in simulation.

Okay, what happens when we change something? Let's change a parameter of the noise function.

It compiles...

And it runs again. This time, there is less difference between the LEDs, as if the animation had been stretched out.

How does this magic work?

In this case, there is no emulation. Visual Studio Code has not suddenly gained the ability to simulate this board. Instead, the same program is compiled but using the WebAssembly target of TinyGo. In WebAssembly, the machine package gets a different purpose. All calls to it are propagated and the TinyGo extension for Visual Studio Code intercepts them to show what the board would do.

This is not a perfect simulation of course. For example, it has the speed and resources of a much more powerful computer, so it will not avoid the need for testing on real hardware. But if it works for most quick edit-compile-test cycles, that can be a big productivity boost.

# Why Go on embedded?

Why not C/C++? C is what almost everybody uses in the industry, and sometimes C++.
Of course I don't have to convince you Go is a nice language and it would be great to run it bare metal. But there are good reasons to use something other than the status quo.

~~ Security ~~
One reason is security
More than 2/3 of all security vulnerabilities are memory safety issues like buffer overflows. Any modern language prevents this, including Go. Switching to another language thus has a huge impact on safety.
This is becoming more and more important now with lots of devices getting connected to the internet in the Internet of Things.

~~ Ease of programming ~~
The second reason is ease of programming. Go is inherently more portable than C, for example because the Go language specification leaves less room for variation and because of the strong standard library which is the same across all operating systems.

If you have ever used a cross compiling toolchain, you know how difficult it is to set them up correctly. Both the main Go compiler and TinyGo avoid all this, for example by using static linking by default.

One exception I should add here is the Arduino environment. While it was designed for beginners, it does manage to avoid most of these issues and I think that's one of the reasons it became so popular.

~~ Concurrency ~~

The next one is concurrency, and this is where Go really shines.

While most embedded systems are single core, they are very much concurrent. There are many inputs, many outputs, and lots of things running concurrently. That's why most bigger projects use what's called a real-time operating system, or RTOS. This is more like a library than an OS, actually.
They provide lightweight tasks, and locks and channels to communicate between them.
RTOSes also often provide memory management, a HAL, and sometimes a network stack.

All of this is already provided by the Go runtime or standard library. This means that unlike in C where you have tens of different RTOSes, concurrent code is easily shared between projects and even between server and embedded systems.

~~ Testing ~~

And the last reason why I think Go would be a good fit for embedded systems is testing.

Testing is perhaps the best way to improve software reliability. This is especially important for embedded devices as they often run for years without interruption.
Unfortunately testing these systems is hard.
How do you fire up continuous integration for custom designed hardware that you only have a few of?
Also psychology: the harder it is to test software the fewer tests you will write.

# Testing in Go

So how do you fix this?
- Well, you separate out hardware specific code into device drivers, manually test those once, and re-use these drivers across projects
  This works, because drivers usually don't require a lot of maintenance.
- Then you use mocks or stubs to test the rest on standard hardware like your laptop and CI.

Replacing drivers with mocks or stubs is easy in Go thanks to interfaces.

~~ Current status of testing ~~

The current status of `tinygo test` is that it works, but not everywhere. TinyGo has replaced the testing package with a custom version that is more memory efficient, and it can run these tests both natively on a desktop computer and under emulation, but not yet on actual hardware. I'm working on getting that last bit fixed, and it should be supported in the near future. That said, if you're testing application logic instead of drivers you should be fine using `go test` for running tests and using `tinygo build` to build firmware images - after all, it's the same language.

# Demo time!

Now we get to the fun part. I would like to give a demo of something I wrote. It is a so-called _poi_. A poi is usually a small weight on a tether that you can spin around - most people will probably know this from fire spinning although fire spinning is actually a relatively recent development.

What is an even more recent development is using LEDs in these things.

What I've used in these poi is:

  - a Nordic Semiconductor Bluetooth-enabled microcontroller,
  - a BMI160 accelerometer and gyroscope, which measures the rotation speed
  - and two LED strips. For that I use SK9822 chips, which are in fact better for this purpose than the original.

Most of the code is in reusable dependencies. I've talked before about the bluetooth package, which is part of the TinyGo project. The motion sensor and the LED strips are both controlled with drivers that are also part of the TinyGo project. I also used an external hashing library for randomness. This external library was not written for TinyGo but worked as intended on the first try.

Thank you for listening.
