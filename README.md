# JTestMemProfiler

While numerous effective tools exist for automating latency measurements, there is a scarcity oftools dedicated to
profiling memory allocations, and those that exist are primarily commercial. JTestMemProfiler aims to address this gap.

The library is designed for ease of use with Java and is available on major platforms: Windows x64, Linux x64, OSX Intel
x64, and OSX ARM. Utilizing the JVMTI support within the JVM, it processes allocations in the target environment and is
compiled against headers for each long-term support version of Java.

JTestMemProfiler is intended for use with a single test at a time to measure memory allocation overhead accurately. It
is not designed for concurrent execution within a test suite or from multiple tests simultaneously within the same
process. Doing so may result in undefined behaviour, with allocations potentially directed to incorrect instances of
collectors and/or filters.

You can download and utilise the linked libraries directly, allowing you to work with the profiler as needed.
Alternatively, you can use the Java wrapper library for added convenience, available
at [JTestMemProfiler](https://github.com/hstuart/jtestmemprofiler-java).
