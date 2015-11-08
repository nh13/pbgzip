#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "bgzf.h"
#include "block.h"
#include "queue.h"
#include "reader.h"
#include "consumer.h"
#include "writer.h"
#include "util.h"
#include "pbgzf.h"

int 
write_open(const char *fn, int is_forced)
{
  int fd = -1;
  char c;
  if (!is_forced) {
      if ((fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, 0666)) < 0 && errno == EEXIST) {
          fprintf(stderr, "[pbgzip] %s already exists; do you wish to overwrite (y or n)? ", fn);
          scanf("%c", &c);
          if (c != 'Y' && c != 'y') {
              fprintf(stderr, "[pbgzip] not overwritten\n");
              exit(1);
          }
      }
  }
  if (fd < 0) {
      if ((fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
          fprintf(stderr, "[pbgzip] %s: Fail to write\n", fn);
          exit(1);
      }
  }
  return fd;
}

static int 
pbgzip_main_usage()
{
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage:   pbgzip [options] [file] ...\n\n");
  fprintf(stderr, "Options: -c        write on standard output, keep original files unchanged\n");
  fprintf(stderr, "         -d        decompress\n");
  fprintf(stderr, "         -f        overwrite files without asking\n");
  fprintf(stderr, "         -n INT    number of threads [%d]\n", detect_cpus());
#ifndef DISABLE_BZ2
  fprintf(stderr, "         -t INT    the compress type (0 - gz, 1 - bz2) [%d]\n", 0);
#endif
  fprintf(stderr, "         -1 .. -9  the compression level [%d]\n", Z_DEFAULT_COMPRESSION);
  fprintf(stderr, "         -S        the block size when reading uncompressed data (must be less than or equal to %d; -1 is auto) [%d]\n",
		  MAX_BLOCK_SIZE,
		  -1);
#ifdef HAVE_IGZIP
  fprintf(stderr, "         -i        use the intel igzip library for compression (deflation); not use for decompressoin (inflation)\n");
#endif
  fprintf(stderr, "         -h        give this help\n");
  fprintf(stderr, "\n");
  return 1;
}


int
main(int argc, char *argv[])
{
  int opt, f_src, f_dst;
  int32_t compress, compress_level, compress_type, pstdout, is_forced, queue_size, n_threads, uncompressed_block_size;
#ifdef HAVE_IGZIP
  int32_t use_igzip = 0;
#endif

  compress = 1; compress_level = -1; compress_type = 0;
  pstdout = 0; is_forced = 0; queue_size = 1000; n_threads = detect_cpus();
  uncompressed_block_size = -1;

#ifndef DISABLE_BZ2 // We should really find a better way
#ifdef HAVE_IGZIP
#define PBGZIP_ARG_STR "cdhfn:it:q:S:0123456789"
#else // HAVE_IGZIP
#define PBGZIP_ARG_STR "cdhfn:t:q:S:0123456789"
#endif // HAVE_IGZIP
#else // DISABLE_BZ2
#ifdef HAVE_IGZIP
#define PBGZIP_ARG_STR "cdhfn:iq:S:0123456789"
#else // HAVE_IGZIP
#define PBGZIP_ARG_STR "cdhfn:q:S:0123456789"
#endif // HAVE_IGZIP
#endif // DISABLE_BZ2

  while((opt = getopt(argc, argv, PBGZIP_ARG_STR)) >= 0) {
      if('0' <= opt && opt <= '9') {
          compress_level = opt - '0'; 
          continue;
      }
      switch(opt){
        case 'd': compress = 0; break;
        case 'c': pstdout = 1; break;
        case 'f': is_forced = 1; break;
        case 'q': queue_size = atoi(optarg); break;
        case 'n': n_threads = atoi(optarg); break;
#ifdef HAVE_IGZIP
		case 'i': use_igzip = 1; break;
#endif
#ifndef DISABLE_BZ2
        case 't': compress_type = atoi(optarg); break;
#endif
		case 'S': uncompressed_block_size = atoi(optarg); break;
        case 'h': 
        default:
                  return pbgzip_main_usage();
      }
  }

  if(argc <= 1) return pbgzip_main_usage();

  if(MAX_BLOCK_SIZE < uncompressed_block_size) {
      fprintf(stderr, "[pbgzip] -S (%d) was too big; must be less than or equal to %d.\n",
			  uncompressed_block_size, 
			  MAX_BLOCK_SIZE);
	  return 1;
  }


  if(pstdout) {
      f_dst = fileno(stdout);
  }
  else {
      if(1 == compress) {
          char *name = malloc(strlen(argv[optind]) + 5);
          strcpy(name, argv[optind]);
#ifndef DISABLE_BZ2
          if(0 == compress_type) strcat(name, ".gz");
          else strcat(name, ".bz2");
#else
          strcat(name, ".gz");
#endif
          f_dst = write_open(name, is_forced);
          free(name);
          if (f_dst < 0) return 1;
      }
      else {
          char *name = strdup(argv[optind]);
#ifndef DISABLE_BZ2
          if(0 == compress_type) {
              if(strlen(name) < 3 || 0 != strcmp(name + (strlen(name)-3), ".gz")) {
                  fprintf(stderr, "Error: the input file did not end in .gz");
                  return 1;
              }
              name[strlen(name) - 3] = '\0';
          }
          else {
              if(strlen(name) < 4 || 0 != strcmp(name + (strlen(name)-4), ".bz2")) {
                  fprintf(stderr, "Error: the input file did not end in .bz2");
                  return 1;
              }
              name[strlen(name) - 4] = '\0';
          }
#else 
          if(strlen(name) < 3 || 0 != strcmp(name + (strlen(name)-3), ".gz")) {
              fprintf(stderr, "Error: the input file did not end in .gz");
              return 1;
          }
          name[strlen(name) - 3] = '\0';
#endif
          f_dst = write_open(name, is_forced);
          free(name);
      }
  }

  // read from stdin if no more arguments, or a "-", are/is supplied. 
  if (optind == argc || 0 == strcmp(argv[optind], "-")) {
	  f_src = fileno(stdin);
  }
  else if ((f_src = open(argv[optind], O_RDONLY)) < 0) {
      fprintf(stderr, "[pbgzip] %s: %s\n", strerror(errno), argv[optind]);
      return 1;
  }

#ifdef HAVE_IGZIP
  pbgzf_main(f_src, f_dst, compress, compress_level, compress_type, queue_size, n_threads, uncompressed_block_size, use_igzip);
#else
  pbgzf_main(f_src, f_dst, compress, compress_level, compress_type, queue_size, n_threads, uncompressed_block_size);
#endif

  if(!pstdout) unlink(argv[optind]);

  return 0;
}
