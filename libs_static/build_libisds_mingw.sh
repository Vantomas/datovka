#!/usr/bin/env sh

SCRIPT=$(readlink -f "$0")
SCRIPT_LOCATION=$(dirname $(readlink -f "$0"))

. "${SCRIPT_LOCATION}"/../scripts/dependency_sources.sh

adjust_sources "mingw"

WIN_VER="0x0501" #https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745%28v=vs.85%29.aspx
MAKEOPTS="-j 4"

SRCDIR="${SCRIPT_LOCATION}/srcs"
PATCHDIR="${SCRIPT_LOCATION}/patches"
WORKDIR="${SCRIPT_LOCATION}/work"
BUILTDIR="${SCRIPT_LOCATION}/built"


X86_MINGV_HOST=i586-mingw32msvc
#X86_MINGV_HOST=i686-w64-mingw32 # Generated usable debugging information.
#X86_MINGV_HOST=i686-pc-mingw32
X86_MINGW_PREFIX=${X86_MINGV_HOST}-
X86_MINGW_CC=${X86_MINGV_HOST}-gcc
X86_MINGW_LD=${X86_MINGV_HOST}-ld
X86_MINGW_STRIP=${X86_MINGV_HOST}-strip
X86_MINGW_RANLIB=${X86_MINGV_HOST}-ranlib

BUILTDIR="${BUILTDIR}-${X86_MINGV_HOST}"

if [ ! -d "${SRCDIR}" ]; then
	mkdir "${SRCDIR}"
fi

if [ ! -d "${WORKDIR}" ]; then
	mkdir "${WORKDIR}"
fi

if [ ! -d "${BUILTDIR}" ]; then
	mkdir "${BUILTDIR}"
fi

ZLIB_ARCHIVE="${_ZLIB_ARCHIVE}"
EXPAT_ARCHIVE="${_EXPAT_ARCHIVE}"
LIBTOOL_ARCHIVE="${_LIBTOOL_ARCHIVE}"

LIBICONV_ARCHIVE="${_LIBICONV_ARCHIVE}"
LIBXML2_ARCHIVE="${_LIBXML2_ARCHIVE}"
#GETTEXT_ARCHIVE="${_GETTEXT_ARCHIVE}" # Disable NLS.

LIBCURL_ARCHIVE="${_LIBCURL_ARCHIVE}"
OPENSSL_ARCHIVE="${_OPENSSL_ARCHIVE}"

LIBISDS_ARCHIVE="${_LIBISDS_ARCHIVE}"
LIBISDS_ARCHIVE_PATCHES="${_LIBISDS_ARCHIVE_PATCHES}"
#LIBISDS_GIT="https://gitlab.labs.nic.cz/kslany/libisds.git"
#LIBISDS_BRANCH="feature-openssl" # Use master.


if [ ! -z "${ZLIB_ARCHIVE}" ]; then
	ARCHIVE="${SRCDIR}/${ZLIB_ARCHIVE}"
	if [ ! -f "${ARCHIVE}" ]; then
		echo "Missing ${ARCHIVE}" >&2
		exit 1
	fi
	# zlib
	rm -rf "${WORKDIR}"/zlib*
	cd "${WORKDIR}"
	tar -xJf "${ARCHIVE}"
	cd "${WORKDIR}"/zlib*

	make ${MAKEOPTS} -f win32/Makefile.gcc SHARED_MODE=0 PREFIX=${X86_MINGW_PREFIX} BINARY_PATH=${BUILTDIR}/bin INCLUDE_PATH=${BUILTDIR}/include LIBRARY_PATH=${BUILTDIR}/lib install || exit 1
fi


if [ ! -z "${EXPAT_ARCHIVE}" ]; then
	ARCHIVE="${SRCDIR}/${EXPAT_ARCHIVE}"
	if [ ! -f "${ARCHIVE}" ]; then
		echo "Missing ${ARCHIVE}" >&2
		exit 1
	fi
	# expat
	rm -rf "${WORKDIR}"/expat*
	cd "${WORKDIR}"
	tar -xjf "${ARCHIVE}"
	cd "${WORKDIR}"/expat*

	CONFOPTS=""
	CONFOPTS="${CONFOPTS} --prefix=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --disable-shared"

	./configure ${CONFOPTS} --host="${X86_MINGV_HOST}"
	make ${MAKEOPTS} && make install || exit 1

	unset CONFOPTS
fi


if [ ! -z "${LIBTOOL_ARCHIVE}" ]; then
	ARCHIVE="${SRCDIR}/${LIBTOOL_ARCHIVE}"
	if [ ! -f "${ARCHIVE}" ]; then
		echo "Missing ${ARCHIVE}" >&2
		exit 1
	fi
	# libtool
	rm -rf "${WORKDIR}"/libtool*
	cd "${WORKDIR}"
	tar -xJf "${ARCHIVE}"
	cd "${WORKDIR}"/libtool*

	CONFOPTS=""
	CONFOPTS="${CONFOPTS} --prefix=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --disable-shared"

	./configure ${CONFOPTS} --host="${X86_MINGV_HOST}"
	make ${MAKEOPTS} && make install || exit 1

	unset CONFOPTS
fi


if [ ! -z "${LIBICONV_ARCHIVE}" ]; then
	ARCHIVE="${SRCDIR}/${LIBICONV_ARCHIVE}"
	if [ ! -f "${ARCHIVE}" ]; then
		echo "Missing ${ARCHIVE}" >&2
		exit 1
	fi
	# libiconv
	rm -rf "${WORKDIR}"/libiconv*
	cd "${WORKDIR}"
	tar -xzf "${ARCHIVE}"
	cd "${WORKDIR}"/libiconv*

	CONFOPTS=""
	CONFOPTS="${CONFOPTS} --prefix=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --disable-shared"

	#
	./configure ${CONFOPTS} --host="${X86_MINGV_HOST}"
	make ${MAKEOPTS} && make install || exit 1

	unset CONFOPTS
fi


if [ ! -z "${LIBXML2_ARCHIVE}" ]; then
	ARCHIVE="${SRCDIR}/${LIBXML2_ARCHIVE}"
	if [ ! -f "${ARCHIVE}" ]; then
		echo "Missing ${ARCHIVE}" >&2
		exit 1
	fi
	# libxml2
	rm -rf "${WORKDIR}"/libxml2*
	cd "${WORKDIR}"
	tar -xzf "${ARCHIVE}"
	cd "${WORKDIR}"/libxml2*

	CONFOPTS=""
	CONFOPTS="${CONFOPTS} --prefix=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --disable-shared"
	CONFOPTS="${CONFOPTS} --without-python"
	CONFOPTS="${CONFOPTS} --with-iconv=${BUILTDIR}"

	#
	./configure ${CONFOPTS} --host="${X86_MINGV_HOST}"
	make ${MAKEOPTS} && make install || exit 1

	unset CONFOPTS
fi


if [ ! -z "${GETTEXT_ARCHIVE}" ]; then
	ARCHIVE="${SRCDIR}/${GETTEXT_ARCHIVE}"
	if [ ! -f "${ARCHIVE}" ]; then
		echo "Missing ${ARCHIVE}" >&2
		exit 1
	fi
	# gettext
	rm -rf "${WORKDIR}"/gettext*
	cd "${WORKDIR}"
	tar -xJf "${ARCHIVE}"
	cd "${WORKDIR}"/gettext*

	CONFOPTS=""
	CONFOPTS="${CONFOPTS} --prefix=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --disable-shared"
	CONFOPTS="${CONFOPTS} --with-libxml2-prefix=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --with-libiconv-prefix=${BUILTDIR}"

	#
	./configure ${CONFOPTS} --host="${X86_MINGV_HOST}" \
	    CPPFLAGS="-I${BUILTDIR}/include" \
	    LDFLAGS="-L${BUILTDIR}/lib"
	make ${MAKEOPTS} && make install || exit 1

	unset CONFOPTS
fi


if [ ! -z "${LIBCURL_ARCHIVE}" ]; then
	ARCHIVE="${SRCDIR}/${LIBCURL_ARCHIVE}"
	if [ ! -f "${ARCHIVE}" ]; then
		echo "Missing ${ARCHIVE}" >&2
		exit 1
	fi
	# libcurl
	rm -rf "${WORKDIR}"/curl*
	cd "${WORKDIR}"
	tar -xjf "${ARCHIVE}"
	cd "${WORKDIR}"/curl*

	CONFOPTS=""
	CONFOPTS="${CONFOPTS} --prefix=${BUILTDIR}"
	#CONFOPTS="${CONFOPTS} --disable-shared"
	CONFOPTS="${CONFOPTS} --enable-ipv6"
	CONFOPTS="${CONFOPTS} --with-winssl"
	CONFOPTS="${CONFOPTS} --without-axtls"
	CONFOPTS="${CONFOPTS} --without-zsh-functions-dir"
	CONFOPTS="${CONFOPTS} --disable-ldap"
	CONFOPTS="${CONFOPTS} --disable-ldaps"
	CONFOPTS="${CONFOPTS} --disable-rtsp"
	#CONFOPTS="${CONFOPTS} --disable-sspi"

	# --disable-shared
	./configure ${CONFOPTS} --host="${X86_MINGV_HOST}" \
	    CPPFLAGS="-DWINVER=${WIN_VER}"
	#make ${MAKEOPTS} && make install || exit 1
	make ${MAKEOPTS}; make install

	unset CONFOPTS
fi


if [ ! -z "${OPENSSL_ARCHIVE}" ]; then
	ARCHIVE="${SRCDIR}/${OPENSSL_ARCHIVE}"
	if [ ! -f "${ARCHIVE}" ]; then
		echo "Missing ${ARCHIVE}" >&2
		exit 1
	fi
	# OpenSSL
	rm -rf "${WORKDIR}"/openssl*
	cd "${WORKDIR}"
	tar -xzf "${ARCHIVE}"
	cd "${WORKDIR}"/openssl*

	# no-asm
	./Configure mingw enable-static-engine shared no-krb5 \
	    --prefix="${BUILTDIR}" --cross-compile-prefix="${X86_MINGW_PREFIX}"
	make ${MAKEOPTS} && make install_sw || exit 1

	cp libeay32.dll "${BUILTDIR}/bin/"
	cp ssleay32.dll "${BUILTDIR}/bin/"
fi


if [ ! -z "${LIBISDS_ARCHIVE}" -a ! -z "${LIBISDS_GIT}" ]; then
	echo "Select libisds archive or git repository." >&2
	exit 1
elif [ ! -z "${LIBISDS_ARCHIVE}" ]; then
	# libisds with OpenSSL back-end
	ARCHIVE="${SRCDIR}/${LIBISDS_ARCHIVE}"
	if [ ! -f "${ARCHIVE}" ]; then
		echo "Missing ${ARCHIVE}" >&2
		exit 1
	fi
	# libisds
	rm -rf "${WORKDIR}"/libisds*
	cd "${WORKDIR}"
	tar -xJf "${ARCHIVE}"
	cd "${WORKDIR}"/libisds*

	if [ "x${LIBISDS_ARCHIVE_PATCHES}" != "x" ]; then
		# Apply patches.
		for f in ${LIBISDS_ARCHIVE_PATCHES}; do
			PATCHFILE="${PATCHDIR}/${f}"
			if [ ! -f "${PATCHFILE}" ]; then
				echo "Missing ${PATCHFILE}" >&2
				exit 1
			fi
			cp "${PATCHFILE}" ./
			echo "Applying ${f}"
			patch -p1 < ${f}
		done
	fi

	CONFOPTS=""
	CONFOPTS="${CONFOPTS} --prefix=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --disable-shared"
	CONFOPTS="${CONFOPTS} --enable-debug"
	CONFOPTS="${CONFOPTS} --enable-openssl-backend"
	CONFOPTS="${CONFOPTS} --disable-fatalwarnings"
	CONFOPTS="${CONFOPTS} --with-xml-prefix=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --with-libcurl=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --with-libiconv-prefix=${BUILTDIR}"

	LINTL=""
	NLS="--disable-nls"
	if [ ! -z "${GETTEXT_ARCHIVE}" ]; then
		LINTL="-lintl"
		NLS=""
	fi
	CONFOPTS="${CONFOPTS} ${NLS}"

	#
	./configure ${CONFOPTS} --host="${X86_MINGV_HOST}" \
	    CPPFLAGS="-I${BUILTDIR}/include -I${BUILTDIR}/include/libxml2" \
	    LDFLAGS="-L${BUILTDIR}/lib"
	make ${MAKEOPTS} && make install || exit 1

	mv "${BUILTDIR}/bin/libcurl-4.dll" "${BUILTDIR}/bin/libcurl-4.dll_x"
	mv "${BUILTDIR}/bin/libeay32.dll" "${BUILTDIR}/bin/libeay32.dll_x"
	mv "${BUILTDIR}/bin/ssleay32.dll" "${BUILTDIR}/bin/ssleay32.dll_x"

	mv "${BUILTDIR}/lib/libcrypto.dll.a" "${BUILTDIR}/lib/libcrypto.dll.a_x"
	mv "${BUILTDIR}/lib/libcurl.dll.a" "${BUILTDIR}/lib/libcurl.dll.a_x"
	mv "${BUILTDIR}/lib/libssl.dll.a" "${BUILTDIR}/lib/libssl.dll.a_x"

	unset CONFOPTS
elif [ ! -z "${LIBISDS_GIT}" ]; then
	# libisds with OpenSSL back-end
	rm -rf "${WORKDIR}"/libisds*
	cd "${WORKDIR}"
	git clone "${LIBISDS_GIT}" libisds-git
	cd "${WORKDIR}"/libisds*
	if [ ! -z "${LIBISDS_BRANCH}" ]; then
		git checkout "${LIBISDS_BRANCH}"
	fi

	CONFOPTS=""
	CONFOPTS="${CONFOPTS} --prefix=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --disable-shared"
	CONFOPTS="${CONFOPTS} --enable-debug"
	CONFOPTS="${CONFOPTS} --enable-openssl-backend"
	CONFOPTS="${CONFOPTS} --disable-fatalwarnings"
	CONFOPTS="${CONFOPTS} --with-xml-prefix=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --with-libcurl=${BUILTDIR}"
	CONFOPTS="${CONFOPTS} --with-libiconv-prefix=${BUILTDIR}"

	LINTL=""
	NLS="--disable-nls"
	if [ ! -z "${GETTEXT_ARCHIVE}" ]; then
		LINTL="-lintl"
		NLS=""
	fi
	CONFOPTS="${CONFOPTS} ${NLS}"

	cat configure.ac | sed -e 's/AC_FUNC_MALLOC//g' > nomalloc_configure.ac
	mv nomalloc_configure.ac configure.ac
	autoheader && libtoolize -c --install && aclocal -I m4 && automake --add-missing --copy && autoconf && echo configure build ok
	./configure ${CONFOPTS} --host="${X86_MINGV_HOST}" \
	    CPPFLAGS="-I${BUILTDIR}/include -I${BUILTDIR}/include/libxml2" \
	    LDFLAGS="-L${BUILTDIR}/lib"
	make ${MAKEOPTS} && make install || exit 1

	mv "${BUILTDIR}/bin/libcurl-4.dll" "${BUILTDIR}/bin/libcurl-4.dll_x"
	mv "${BUILTDIR}/bin/libeay32.dll" "${BUILTDIR}/bin/libeay32.dll_x"
	mv "${BUILTDIR}/bin/ssleay32.dll" "${BUILTDIR}/bin/ssleay32.dll_x"

	mv "${BUILTDIR}/lib/libcrypto.dll.a" "${BUILTDIR}/lib/libcrypto.dll.a_x"
	mv "${BUILTDIR}/lib/libcurl.dll.a" "${BUILTDIR}/lib/libcurl.dll.a_x"
	mv "${BUILTDIR}/lib/libssl.dll.a" "${BUILTDIR}/lib/libssl.dll.a_x"

	unset CONFOPTS
fi
