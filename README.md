# IncrementalGC
Incremental GC for use in latency-sensitive settings

Beginnings of a project to create an allocator that's friendly to latency-sensitive environments like an object pool and friendly to the cache like a compacting GC. The gist is that users manually specify times at which the compacting process can be run, and the GC will incrementally compact the memory. The algorithm used is such that there are no read/write barriers required, although latency-wise it performs much worse on objects which have many references.
