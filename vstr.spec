%define ver      1.0.11
%define libver  1.0
%define real_release_num 1
%define RELEASE %{real_release_num}
%define rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define prefix   /usr
%define name    vstr

# Do we want to create a debug build...
%define dbg_opt %{?dbg}%{!?dbg:0}
# Do we want to run "make check" after the build...
%define chk_opt %{?chk}%{!?chk:0}

%if %{dbg_opt}
%define debugopt --enable-debug
%define RELEASE %{real_release_num}debug
%else
%define debugopt %nil
%endif

%ifarch i386
%define fmtfloatopt --with-fmt-float=%{?float}%{!?float:glibc}
%else
%define fmtfloatopt --with-fmt-float=%{?float}%{!?float:host}
%endif

%define devdoco %{_datadir}/doc/%{name}-devel-%{ver}

Summary: String library, safe, quick and easy to use.
Name: %{name}
Version: %ver
Release: %rel
Copyright: LGPL
Group: Development/Libraries
Source: ftp://ftp.and.org/pub/james/%{name}/%{ver}/%{name}-%{ver}.tar.gz
BuildRoot: /var/tmp/%{name}-%{PACKAGE_VERSION}-root
BuildRequires: pkgconfig >= 0.8, autoconf, automake, libtool
URL: http://www.and.org/vstr/
Packager: James Antill <james@and.org>

%description
 Vstr is a string library designed for network communication, but applicable
in a number of other areas. It works on the idea of separate nodes of
information, and the length/ptr model and not the termination model of
"C strings". It does dynamic resizing of strings as you add/delete data.
 It can also do automatic referencing for mmap() areas, and includes a
portable version of a printf-like function (which is ISO 9899:1999 compliant,
and includes support for i18n parameter position modifiers).

 Development libs and headers are in %{name}-devel.

%package devel
Summary: String library, safe, quick and easy to use. Specilizes in IO.
Group: Development/Libraries
Requires: pkgconfig >= 0.8
Requires: %{name} = %{ver}

%description devel
 Static libraries and header files for the Vstr string library
 Also includes a %{name}.pc file for pkg-config.

%changelog
* Fri Aug  8 2003 James Antill <james@and.org>
- Extra example programs to be installed

* Sat Mar 15 2003 James Antill <james@and.org>
- Add dia diagram of internal structure.
- Added design and printf_comparison html doco.

* Fri Jan 31 2003 James Antill <james@and.org>
- Added chk option for doing a "make check"

* Mon Jan 13 2003 James Antill <james@and.org>
- Moved the includedir to /usr/include/%%{name}-*

* Sat Nov 16 2002 James Antill <james@and.org>
- Added wrap memcpy/memset to the default configure options.

* Sat Nov  9 2002 James Antill <james@and.org>
- Add comparison to installed documentation.

* Mon Sep 30 2002 James Antill <james@and.org>
- Remove files for check-files 8.0 rpm check.

* Mon Sep 30 2002 James Antill <james@and.org>
- Changed COPYING to COPYING.LIB

* Tue Sep 17 2002 James Antill <james@and.org>
- Add BuildRequires
- Add Requires to -devel package.
- Use glibc FLOAT by default on i386 arch.

* Mon Sep  2 2002 James Antill <james@and.org>
- Add `float' cmd line define.

* Tue May 21 2002 James Antill <james@and.org>
- Add linker script to configure.

* Wed May  6 2002 James Antill <james@and.org>
- Add man page.

* Wed Mar 20 2002 James Antill <james@and.org>
- Hack a spec file.

%prep
%setup

%build
# For devel releases
if [ ! -f configure ]; then
  ./scripts/autogen.sh
fi

%configure --prefix=%{prefix} \
  --mandir=%{_mandir} --datadir=%{_datadir} \
  --libdir=%{_libdir} --includedir=%{_includedir}/%{name}-%{libver} \
  --enable-linker-script \
  --enable-wrap-memcpy --enable-wrap-memset \
  %{fmtfloatopt} %{debugopt}

make

%if %{chk_opt}
make check
%endif

%install
rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT install

cp TODO BUGS $RPM_BUILD_ROOT/%{devdoco}/

# Copy the examples somewhere...
mkdir $RPM_BUILD_ROOT/%{devdoco}/examples
cp   \
 examples/Makefile \
 examples/ex_cat.c \
 examples/ex_csv.c \
 examples/ex_gmp_factorials.c \
 examples/ex_gmp_nums.c \
 examples/ex_hello_world.c \
 examples/ex_hello_world.h \
 examples/ex_hello_world_s.c \
 examples/ex_hello_world_m.c \
 examples/ex_hello_world_x.c \
 examples/ex_hexdump.c \
 examples/ex_highlight.c \
 examples/ex_lookup_ip.c \
 examples/ex_mon_cp.c \
 examples/ex_nl.c \
 examples/ex_perf.h \
 examples/ex_perf_nodesize.c \
 examples/ex_perf_nodesize_foreach.c \
 examples/ex_perf_sgopenssl.c \
 examples/ex_rot13.c \
 examples/ex_slowcat.c \
 examples/ex_ssi.c \
 examples/ex_utils.h \
 examples/ex_yes.c \
 examples/ex_zcat.c \
 examples/hexdump_data \
   $RPM_BUILD_ROOT/%{devdoco}/examples/

rm -f $RPM_BUILD_ROOT/usr/lib/lib%{name}.la
rm -f $RPM_BUILD_ROOT/usr/share/doc/%{name}-devel-%{ver}/functions.3
rm -f $RPM_BUILD_ROOT/usr/share/doc/%{name}-devel-%{ver}/constants.3


%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)

%doc AUTHORS COPYING ChangeLog NEWS README TODO

%{_libdir}/lib%{name}-%{libver}.so.*

%files devel
%defattr(-, root, root)

%{_libdir}/lib%{name}.so
%{_libdir}/lib%{name}.a
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/%{name}-%{libver}/*.h

%doc
%{devdoco}/BUGS
%{devdoco}/TODO
%{_mandir}/man3/vstr.3.gz
%{_mandir}/man3/vstr_const.3.gz
%{devdoco}/basic_cat.c.html
%{devdoco}/comparison.html
%{devdoco}/compare_licenses/SafeStr.txt
%{devdoco}/compare_licenses/bstring.txt
%{devdoco}/compare_licenses/cords.txt
%{devdoco}/compare_licenses/ossp-str.txt
%{devdoco}/compare_licenses/rope.txt
%{devdoco}/compare_licenses/sz.txt
%{devdoco}/compare_licenses/toolbox.txt
%{devdoco}/constants.txt
%{devdoco}/constants.html
%{devdoco}/design.html
%{devdoco}/f_c.css
%{devdoco}/functions.txt
%{devdoco}/functions.html
%{devdoco}/internal_structure.dia
%{devdoco}/namespace.html
%{devdoco}/overview.html
%{devdoco}/printf_comparison.html
%{devdoco}/req_trunc_io.html
%{devdoco}/security.html
%{devdoco}/security_problems.html
%{devdoco}/speed.html
%{devdoco}/structs.txt
%{devdoco}/structs.html
%{devdoco}/size_cmp.gnumeric
%{devdoco}/tutorial.html
%{devdoco}/examples/Makefile
%{devdoco}/examples/ex_cat.c
%{devdoco}/examples/ex_csv.c
%{devdoco}/examples/ex_gmp_factorials.c
%{devdoco}/examples/ex_gmp_nums.c
%{devdoco}/examples/ex_hello_world.c
%{devdoco}/examples/ex_hello_world.h
%{devdoco}/examples/ex_hello_world_s.c
%{devdoco}/examples/ex_hello_world_m.c
%{devdoco}/examples/ex_hello_world_x.c
%{devdoco}/examples/ex_hexdump.c
%{devdoco}/examples/ex_highlight.c
%{devdoco}/examples/ex_lookup_ip.c
%{devdoco}/examples/ex_mon_cp.c
%{devdoco}/examples/ex_nl.c
%{devdoco}/examples/ex_perf.h
%{devdoco}/examples/ex_perf_nodesize.c
%{devdoco}/examples/ex_perf_nodesize_foreach.c
%{devdoco}/examples/ex_perf_sgopenssl.c
%{devdoco}/examples/ex_rot13.c
%{devdoco}/examples/ex_slowcat.c
%{devdoco}/examples/ex_ssi.c
%{devdoco}/examples/ex_utils.h
%{devdoco}/examples/ex_yes.c
%{devdoco}/examples/ex_zcat.c
%{devdoco}/examples/hexdump_data
