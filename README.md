<h1><img src="logo.svg" width="200" alt="getargv"></h1>

[![C++/make CI](https://github.com/getargv/getargv.cpp/actions/workflows/actions.yml/badge.svg)](https://github.com/getargv/getargv.cpp/actions/workflows/actions.yml)

`libgetargv++` is a library that allows you to get the arguments that were passed to another running process on macOS. It is intended to provide roughly the same functionality as reading from `/proc/<pid>/cmdline` on Linux. On macOS this is done by parsing the output of the `KERN_PROCARGS2` sysctl, which is <span title="always, in my observation">very often</span> implemented [incorrectly](https://getargv.narzt.cam/hallofshame.html), due to the overlooked possibility of leading empty arguments passed to the target process.

This is a library providing a more idiomatic C++ API wrapping the C API provided by libgetargv.

## Permissions

`libgetargv++` can only see processes running as the same user by default, so be sure your process runs as the desired user (`setuid`, [`launchd.plist`](x-man-page://launchd.plist), [`sudo`](x-man-page://sudo)) or can [elevate privileges](https://developer.apple.com/library/archive/documentation/Security/Conceptual/SecureCodingGuide/Articles/AccessControl.html); n.b. elevating privileges safely is [extremely complicated](https://developer.apple.com/forums/thread/708765), and will be a target of privilege escalation attacks on macOS so be extremely careful if you go this route, better to defer to the user to elevate privileges for you as needed.

## Installation

### Step 1: Downloading

You can download a [source bundle](https://github.com/getargv/getargv.cpp/archive/refs/tags/0.1.tar.gz), or an [installer package](https://github.com/getargv/getargv.cpp/releases/download/0.1/libgetargv++-macos-13.dmg) from GitHub.

### Step 2: Installing

The simplest way to install this lib is via Homebrew, just run `brew tap getargv/tap` and then `brew install libgetargv++`. If you don't use Homebrew, then the next easiest way to install this lib is if you downloaded the installer package, which you can simply double click to be guided through the installation via a wizard. If you want to have absolute control over the installation you can unpack the pre-built library downloaded from GitHub and put it somewhere your compiler will pick it up, such as `/usr/local/lib` and put the headers somewhere like `/usr/local/include/libgetargv++`.

### Step 3: Environment

Be sure that your compiler can find this lib by checking the library and header search paths:

```bash
echo | clang++ -c -v -x c++ - 2>&1 | sed -Ee '/search starts here/,/End of search list/!d;/End of search list/q'
```

> [!IMPORTANT]
> On Apple Silicon (ARM) Macs, Homebrew installs native packages in `/opt`, and puts Intel libraries in `/usr/local`, so your compiler will only automatically pick up Intel libraries, which is almost certainly not what you want. You need to explicitly exclude `/usr/local` from your search paths in this case and add `/opt/`.

### System Requirements

macOS is required as this is a macOS specific `sysctl`, even BSD does not implement it. Your system must support `sysctl` and `KERN_PROCARGS2`, which probably means macOS [10.3](https://github.com/apple-oss-distributions/xnu/blob/xnu-517/bsd/sys/sysctl.h#L332) or later, though I haven't tested versions older than 10.7. You'll also need a non-ancient Clang++ (C++11 is required, C++20 is the default standard on Clang++ versions > 13, otherwise C++17) you can override the C++ std by setting `CXXFLAGS="--std=c++11 -O3 -Iinclude"`.

## Building libgetargv++

To make `libgetargv++`:

 - Install `libgetargv` to your system (see below).
 - Clone [this repo](https://github.com/getargv/getargv.cpp) and run `make`.

## Building libgetargv

To make `libgetargv` Clone [the repo](https://github.com/getargv/getargv) and run `make dylib`.

I've built `libgetargv` on macOS 10.7-14, using only the CLT package, not the full Xcode install. If you need to override variables, do so inside the `make` command, eg: `make EXTRA_CPPFLAGS=-DMACRO EXTRA_CFLAGS=-std=c17 dylib`. If you are trying to build on a version of macOS earlier than 10.7, let me know how it goes.

## Testing

Run `make -C test`.

I've tested libgetargv++ on macOS 10.7-14, and run CI against all available GitHub hosted macOS runners, with plans to standup a CI cluster of VMs once I acquire appropriate hardware.

## Usage

### Step 1: Choosing a struct

Do you just want to print the arguments to `stdout` or look at the bytes of the arguments? Then you probably want the `Argv` struct, if you want to look at or parse the arguments, then you probably want the `ArgvArgc` struct. If you need to pass along the arguments to other functions that expect standard C++ types, then there are functions to give you those, however they are a bit less efficient as they involve additional copies.

### Step 2: Choosing a constructor

If you want to get a C++ type then `Argv::as_string()` or `ArgvArgc::as_string_array()` are your friends, if you want just the bytes, then `Argv()` or `Argv::as_bytes()` are for you, and if you want to look at the args individually then `ArgvArgc()` or `ArgvArgc::as_array()` are what you want.

### Step 3: Iterating

Once you have a struct, you can iterate over its contents in the usual way, for example:

```cpp
auto args = ArgvArgc(getpid());
for (auto arg : args) {
std::cout << arg << "\n";
}
auto bytes = Argv(getpid());
for (auto byte : bytes) {
std::cout << byte;
}
```

## Safety

This library attempts to provide guardrails where possible to avoid undefined behaviour, but C++ is not very well suited to the task, so care must still be taken.
