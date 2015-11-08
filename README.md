# PBGZIP - Parallel Block GZIP

This tool and API implements parallel block gzip.
For many formats, in particular Genomics Data Formats, data are compressed in fixed-length blocks such that they can be easily indexed based on a (genomic) coordinate order, since typically each block is sorted according to this order.
This allows for each block to be individually compressed (deflated), or more importantly, decompressed (inflated), with the latter enabling random retrieval of data in large files (gigabytes to terabytes).
`pbgzip` is not limited to any particular format, but certain features are tailored to Genomics Data Formats when enabled (see below).
Parallel decompression is somewhat faster, but truly the speedup comes during compression.

Author: Nils Homer

## Installation

1. Compile PBGZIP:
  <pre lang="bash"><code>sh autogen.sh && ./configure && make</code></pre>
2. Install
  <pre lang="bash"><code>make install</code></pre>

## IGZIP Support

[igzip](https://software.intel.com/en-us/articles/igzip-a-high-performance-deflate-compressor-with-optimizations-for-genomic-data) is a high-performance compression library for gzip or deflate compression for Genome data.
For BAM (or SAM) formats (see the [SAM/BAM Specification](https://samtools.github.io/hts-specs/SAMv1.pdf)), it can significantly speed up the compression, with minimal loss in compression ratio when compared to the fastest level of ```zlib```.
In `pbgzip`, we can utilize the BAM-specific speedup, so using `igzip` in this context may not yield good results for non-BAM data.

#### Intalling IGZIP

The BAM-specific `igzip` libraries must installed prior to compiling `pbgzip`.
We have included the `igzip` source ready for installing the bam-specific `igzip` libraries.
Please compile and install `igzip`:

```
  cd igzip
  make
  make install
  cd ..
```

To override the install path, set the `INSTALL_PATH` environment variable before compiling and installing `igzip`.
You may also need to updated your `LD_LIBRARY_PATH` variable to include the `INSTALL_PATH`:

```
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lib
```

If the compilation fails, you may be on a system that is not supported by `igzip`; please contact the original authors for `igzip` support.

#### Enable IGZIP when configuring PBGZIP

Make sure to enable `igzip` when running configure:

```
  ./configure --enable-igzip
```

#### IGZIP Acknowledgments
Please thank the original authors of `igzip`: Jim Guilford, Vinodh Gopal, Sean Gulley and Wajdi Feghali.
Also thank Paolo Narvaez, Mishali Naik and Gil Wolrich for their contributions.
Finally, thank Intel for sponsoring the `igzip` work.

## Current Issues

For developer issues, see: https://github.com/nh13/pbgzip/issues

#### Compression Limitations

Due to the requirement that we must fit a chunk of compressed data into a maximum size block, we may fail at compression when the data has sufficiently high entropy (is too random).
For example, try compressing a very random set of data:

```
head -c 10M </dev/urandom | pbgzip -c > myfile.gz
```

The compression should fail.  
Fortunately, ```bgzip``` should succeed on this data, but ```pbgzip``` currently does not support this case.
Developer Note: we would need to keep a buffer of data that was fed to consumers, such that we rewind back to try to compress fewer bytes in the given block that failed deflation.

#### ```undefined reference to `rpl_malloc'```

In same cases, you may get: ```undefined reference to `rpl_malloc'```. This is due to not being able to find the `malloc` function.

Try the following:

```
ac_cv_func_malloc_0_nonnull=yes ./configure <your configure options>
```
