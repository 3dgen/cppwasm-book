# 4.7 Forget about filesystem

Section 3.3 introduces three synchronous file systems provided by Emscripten: `MEMFS`, `IDBFS`, `NODEFS`, and their features are as follows:

Feature | `MEMFS` | `IDBFS` | `NODEFS`
-|-|-|-
Access local file system | No | No | Yes
Emulate using memory | Yes | Yes | No
Persistent storage | No | Yes | Yes

`NODEFS` can only be used in Node.js, so the core reason for not recommending using file system in web applications is that `MEMFS`/`IDBFS` requires memory to emulate the file system.

Memory is a very precious hardware resource. The memory of iOS devices generally does not exceed 4GB. It is a very extravagant behavior to use the memory to simulate the file system.

In addition, the time consumed by the initialization of the virtual file system is also a factor to be considered. The bulk of the packaged files download takes a long time, which is very disadvantageous for web applications.

We don't mean that the virtual file system can't be used at all - it is feasible to save the front-end configurations with `IDBFS`. Some prototypes can also reduce the difficulty of porting by using the virtual file system. However, when using Emscripten to develop web application modules, you need to pay special attention to 32-bit memory space, low-speed network IO limits, which is vastly different from the resources available when developing native applications. When the data required for operation is very large, it is best to use on-demand loading and LOD strategy.

After abandoning the virtual file system, we can only use network IO as the primary data path, which will be further explored in Chapter 5.