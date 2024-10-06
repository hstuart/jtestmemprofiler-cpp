# JTestMemProfiler

While a lot of great tools exist for automating latency measurements, very few, and primarily commercial graphical user interface-based
tools exist to profile memory allocations. This is what JTestMemProfiler seeks to remedy.

The library is designed to be easily useable from Java and is released for the major platforms, i.e. Windows x64, Linux x64,
OSX Intel x64, and OSX ARM. The profiler is using the JVMTI support in the JVM to process allocations in the target JVM and
is compiled against headers for each of the long term support Java versions in use.

The library is intended to be used from a single test at a time to measure memory allocation overhead of that
test. It is not designed to be run in a concurrent test suite or from multiple tests at a time in the same process. Behavior here may
be undefined or allocations may arrive at the wrong instance of the collectors and/or filters.

You can download and use the linked libraries directly to work with the profiler in your own way, or you can use the
Java wrapper library that makes this more convenient to work with at [JTestMemProfiler](https://github.com/hstuart/jtestmemprofiler-java).
