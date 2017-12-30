# IncrementalGC
Incremental GC for use in latency-sensitive settings

Beginnings of a project to create an allocator that's friendly to latency-sensitive environments
like an object pool and friendly to the cache like a compacting GC.
The gist is that users manually specify times at which the compacting process can be run,
and the GC will incrementally compact the memory.

The algorithm is as such:

When scanning the graph (remember that the GC yields to main code during this):
  
  1. All objects are initially marked white.
  2. When the collector reachs an object, it marks it grey.
  4. Forwarding pointers will be placed in the old location and used to re-direct references
  5. The first forwarded reference will be fixed, this allows for an important optimization
     for objects that never have multiple references
  6. After compacting an object, the GC will non-incrementally check that all objects are scanned.
  7. Once all objects are moved, the object is marked black
  8. The scan-compact stage is now complete
  9. The GC checks if a 'written' flag is true. If not, the job is finished
  10. The GC iterates over the object graph re-scanning grey/white objects and cleaning up forwarding references
  11. This loop of compact-check continues until the GC is complete.
  
\* It's important that this is non-incremental, since otherwise the scanner would have concurrency problems with completely marking a single object
  
Concurrently with all of this, writers will mark on object as grey and set written flag when they move a currently living object from one location to another. The other cases, nullifying an object and adding a new object, don't require re-scanning since they do not move references in memory at danger of being relocated.

Furthermore, writers are required to load a forwarding reference
instead of a regular one to ensure that writes make it to the new address.
This can be avoided if the type in question will only have one live reference
that is fixed in the initial move

Readers will only have to take additional action with forwarded items if the type in question
requires special behavior based on address
