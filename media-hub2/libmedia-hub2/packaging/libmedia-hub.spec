Name:           libmedia-hub
Version:        %{_version}
Release:        %{_release}
Summary:        libmedia-hub
License:        COMERCIAL
Group:          middleware
Source:         %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
#BuildRequires:  expat-devel
%description

%package -n     libmedia-hub-tool
Provides:       libmedia-hub-tool
Summary:        libmedia-hub-tool
Group:          middleware
Requires:       libmedia-hub-runtime = %{version}
%description -n libmedia-hub-tool

%package -n     libmedia-hub-runtime
Provides:       libmedia-hub-runtime
Summary:        libmedia-hub-runtime
Group:          middleware
%description -n libmedia-hub-runtime

%package -n     libmedia-hub-devel
Provides:       libmedia-hub-devel 
Summary:        libmedia-hub-devel
Group:          middleware
Requires:       libmedia-hub-runtime = %{version}
%description -n libmedia-hub-devel

%prep
%setup -q

%build

%if "%{_amatarget}" == "F10"
	cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_TARGET_F10=1
%else
	cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_TARGET_TAFLUS=1
%endif

make -j 4

%install
rm -rf %{buildroot}
make  DESTDIR=%{buildroot} install
mkdir -p %{buildroot}/etc
%if "%{_amatarget}" == "F10"
    cp plugins/ipod/data/f10_ipod.conf %{buildroot}/etc/libmh-ipod.conf
    cp plugins/ipod/data/f10_ipod_logo_small.bmp %{buildroot}/etc/ipod_logo_small.bmp 
    cp plugins/ipod/data/f10_ipod_logo_large.bmp %{buildroot}/etc/ipod_logo_large.bmp 
	
	cp plugins/fs/conf/libmh-plugin-fs-F10.conf %{buildroot}/etc/libmh-plugin-fs.conf
	
	cp contents/conf/libmh-plugins-F10.conf %{buildroot}/etc/libmh-plugins.conf
%else
    cp plugins/ipod/data/taflus_ipod.conf %{buildroot}/etc/libmh-ipod.conf
    cp plugins/ipod/data/taflus_ipod_logo_small.bmp %{buildroot}/etc/ipod_logo_small.bmp 
    cp plugins/ipod/data/taflus_ipod_logo_large.bmp %{buildroot}/etc/ipod_logo_large.bmp 

	cp plugins/fs/conf/libmh-plugin-fs-TAflus.conf %{buildroot}/etc/libmh-plugin-fs.conf
	
	cp contents/conf/libmh-plugins-TAflus.conf %{buildroot}/etc/libmh-plugins.conf
%endif


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files -n libmedia-hub-runtime
%defattr(755,root,root)
%{_prefix}/lib/*.so.*
%{_prefix}/lib/mh-plugins/*.so.*
%{_prefix}/lib/mh-plugins/*.so
%{_prefix}/bin/mh-dbus-daemon
%{_prefix}/bin/mh-daemon
/etc/libmh-ipod.conf
/etc/ipod_logo_small.bmp
/etc/ipod_logo_large.bmp
/etc/libmh-plugin-fs.conf
/etc/libmh-plugins.conf

%files -n libmedia-hub-devel
%defattr(755,root,root)
%{_prefix}/include/
%{_prefix}/lib/pkgconfig/
%{_prefix}/lib/*.so
%{_prefix}/lib/mh-plugins/*.so

%files -n libmedia-hub-tool
%defattr(755,root,root)
%{_prefix}/bin/*-test
%{_prefix}/bin/*-demo
%{_prefix}/share/

%clean

