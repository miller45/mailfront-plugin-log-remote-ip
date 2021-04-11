Name:		mailfront-plugin-log-remote-ip
Summary:	A mailfront plugin to put remote ip into logs
Version:	0.01
Release:	1%{?dist}
License:	GPL
Source:		}-%{version}.tar.bzi2
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:	gcc >= 3.4.6, make, bglibs-devel >= 2.03, mailfront-devel >= 2.12
Requires:	bglibs >= 2.03, mailfront >= 2.12, ucspi-tcp >= 0.88
URL:	        https://github.com/miller45/mailfront-plugin-log-remote-ip	

%description
Plugin: log-remote-ip

The following text is obsolete

A mailfront plugin to provide an ability to add authenticated sender to
message header with each line header message prefixed if $TRACK_AUTH_SENDER
environment variable is set and not empty. Otherwise it will be set to
"X-AntiAbuse: " without quotes. If $TRACK_AUTH_SENDER_HEADER is set and not
empty, this value will be added to the mail header first once otherwise it
will use the following without any newlines:

X-AntiAbuse: This header was added to track abuse, please include it with any abuse report

IMPORTANT: This plugin is written for and tested with mailfront version 2.12.


%prep
%setup -q
# for bglibs
# conf-bgincs default is /usr/local/bglibs/include
echo %{_includedir}/bglibs > conf-bgincs
# conf-bglibs default is /usr/local/bglibs/lib
echo %{_libdir}/bglibs > conf-bglibs
# conf-ld default is gcc -s -L/usr/local/lib
echo "gcc -Wall -g -O2 -s -rdynamic" >conf-ld
# conf-mailfrontincs default is /usr/local/include/mailfront
echo %{_includedir}/mailfront > conf-mailfrontincs
# conf-modules default is /usr/local/lib/mailfront
echo %{_libdir}/mailfront > conf-modules
echo "gcc %{optflags}" >conf-cc
# conf-ccso default is gcc -W -Wall -Wshadow -O -g -I/usr/local/include -fPIC -shared
echo "gcc %{optflags} -fPIC -shared" >conf-ccso
echo "gcc -s -rdynamic" >conf-ld
#echo %{_bindir} >conf-bin
echo %{_libdir}/mailfront >conf-modules
echo %{_includedir} >conf-include

%build
make

%install
rm -fr %{buildroot}
mkdir -p %{buildroot}/%{_libdir}/mailfront
install -m0755 plugin-add-track-auth-sender.so %{buildroot}/%{_libdir}/mailfront/
#make install_prefix=%{buildroot} install

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc README COPYING CHANGES plugin-log-remote-ip.html
#%{_bindir}/*
%{_libdir}/mailfront/plugin-log-remote-ip.so

%changelog

* Apr 11 2021 miller45 - 0.01-1
- Initial Hack
