# hello
I am Ayke and I'm working on TinyGo.
You may have heard about TinyGo already. It's a new Go compiler like gccgo and the main Go compiler, but it specializes in that binaries are much smaller and that it uses less memory. These properties allow it to be used in places where Go was impractical or impossible before, such as small embedded systems and WebAssembly.

Therefore, it is not meant to replace any of the existing Go compiler, but rather is a complement to them.

In this talk I'm going to focus on embedded systems, not on WebAssembly. I'll start with some history, give some insight into how it works, the current status and what's next for TinyGo.

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

The first reason is the garbage collector. The main Go implementations use an advanced concurrent garbage collector that is well suited for desktop and server operating systems with virtual memory. Most microcontrollers however are single core and don't have virtual memory, so most of those benefits go away. Instead, TinyGo uses a textbook conservative mark/sweep implementation on most chips. It does not need to worry about growing or shrinking the heap as there is only one program running and it can use all remaining space. This garbage collector is very simple, but good enough for TinyGo and most importantly very small and simple.

The second reason is whole-program optimization. TinyGo compiles and optimizes a whole program at once. This means we can get all the benefits of link-time optimization. It is also very slow to compile for larger programs so I hope to get a more traditional LTO setup in place in the future which should keep most of the benefits of whole-program optimization while increasing compilation speed.

Another big reason is how TinyGo treats package initializers, that means globals and init functions. It tries to initialize them all at compile time instead of at runtime. For example, global map variables would normally be created and initialized at runtime, but in TinyGo this usually happens at compile time. The main benefit here is that if the map is never accessed, it is optimized away. This happens quite often, for example in a package that defines a global map but none of the functions that use the map are called.

There are more optimizations but these are some of the main ones.


# Current status

So how close are we to a usable Go compiler?

That depends on your use case. It is able to compile most Go code unmodified. However, the big caveat is "most" - many programs won't be able to compile because a dependency of a dependency somewhere uses a standard library package that is not yet supported. Over time, more and more of the Go standard library will become supported while we keep binary size small so this issue should disappear over time.

That said, it already works really well for many smaller programs, which I'll show at the end of this talk.

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
