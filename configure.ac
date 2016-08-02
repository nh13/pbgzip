#  See LICENSE for details
#
#  The version number in AC_INIT is for the pbgzip package.
#  Developers - the version number should *NEVER* be changed except by, 
#  or with explicit permission from, Nils Homer.  

AC_PREREQ(2.61);
AC_INIT([pbgzip],[0.0.1],[https://github.com/nh13/pbgzip],[pbgzip])
AC_COPYRIGHT([See LICENSE for copyright information.])
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([dist-bzip2 subdir-objects foreign])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Sets build_cpu, build_vendor and build_os variables
AC_CANONICAL_BUILD

# Check for C and C++ compilers
CFLAGS_TMP=${CFLAGS}; # Save, as AC_PROG_CC sets CFLAGS
AC_PROG_CC
AM_PROG_CC_C_O
CFLAGS=${CFLAGS_TMP}; # Restore CFLAGS

AC_PROG_INSTALL
AC_GNU_SOURCE

# set CFLAGS 
default_CFLAGS="-g -Wall -O2 -pthread";
extended_CFLAGS="-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_USE_KNETFILE -DHAVE_LIBPTHREAD" # #-DDISABLE_BZ2

AC_MSG_CHECKING(whether the git revision is available)
gitrev="`git log 2>/dev/null | grep -m 1 commit | awk '{print $2}'`";
if test -n "${gitrev}"; then
    GITREV="${gitrev}";
	echo "${gitrev}" > pbgzip.git.rev;
elif test -f "pbgzip.git.rev"; then
	GITREV="`head -n 1 pbgzip.git.rev`";
fi
if test -n "${GITREV}"; then
    AC_REVISION(["${GITREV}"])
    AC_DEFINE_UNQUOTED(GIT_REV, "${GITREV}", [Define to be the git revision number])
    AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_ARG_ENABLE(intel64, [  --enable-intel64        optimize for Intel64 CPU such as Xeon and Core2], [extended_CFLAGS="${extended_CFLAGS} -mtune=nocona"], [])
AC_ARG_ENABLE(bz2,     [  --disable-bz2           use this option to disable bz2 support], [AC_DEFINE(DISABLE_BZ2,1,[Define 1 if we want to disable bz2 support])], [AC_CHECK_LIB([bz2], [BZ2_bzRead])])
AC_ARG_ENABLE(igzip,   [  --enable-igzip          use this option to enable igzip support], [AC_CHECK_LIB([igzip0c], [fast_lz])], [])
AC_ARG_ENABLE(ipp,     [  --enable-ipp            use this option to enable intel integrated performance primitives support (please set LDFLAGS and CFLAGS appropriately)], 
			  [
			   AC_CHECK_LIB([ippcore], [ippGetLibVersion], [], [AC_MSG_ERROR(["--enable-ipp used but ippcore not found"])]) 
			   AC_CHECK_LIB([ippdc], [ippdcGetLibVersion], [], [AC_MSG_ERROR(["--enable-ipp used but ippdc not found"])]) 
			   AC_CHECK_LIB([ippcc], [ippccGetLibVersion], [], [AC_MSG_ERROR(["--enable-ipp used but ippcc not found"])]) 
			   AC_CHECK_LIB([ippcv], [ippcvGetLibVersion], [], [AC_MSG_ERROR(["--enable-ipp used but ippcv not found"])]) 
			   AC_CHECK_LIB([ippch], [ippchGetLibVersion], [], [AC_MSG_ERROR(["--enable-ipp used but ippch not found"])]) 
			   AC_CHECK_LIB([ippvm], [ippvmGetLibVersion], [], [AC_MSG_ERROR(["--enable-ipp used but ippvm not found"])]) 
			   AC_CHECK_LIB([ippi], [ippiGetLibVersion], [], [AC_MSG_ERROR(["--enable-ipp used but ippi not found"])]) 
			   AC_CHECK_LIB([ipps], [ippsGetLibVersion], [], [AC_MSG_ERROR(["--enable-ipp used but ipps not found"])]) 
			  ], [])

CFLAGS="${CFLAGS} ${default_CFLAGS} ${extended_CFLAGS}";

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_VOLATILE
AC_C_INLINE
# Checks for library functions.
AC_FUNC_MALLOC # Turn this off or use ac_cv_func_malloc_0_nonnull=yes ./configure <your configure options>
AC_FUNC_REALLOC
AC_FUNC_FSEEKO
AC_CHECK_LIB([z], [gzread])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_FUNCS([gethostbyaddr gethostbyname memset select socket strchr strdup strerror strstr strtol])

# Check types
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_SIZE_T
AC_TYPE_OFF_T


# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h stddef.h sys/socket.h])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
