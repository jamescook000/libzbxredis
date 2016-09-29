# libzbxredis

This project provides comprehensive monitoring of Redis servers using a
natively compiled Zabbix agent module, written in C.

A preconfigured Zabbix 2.2, 2.4, 3.0 & 3.2 Template and Value-Maps are also included for your convenience.

Sources in this project are used to compile `libzbxredis.so` which may be
loaded by a Zabbix agent using the `LoadModule` directive.

## Prereqisites

* GNU Build Tools (make, gcc, autoconf, automake, libtool, m4, etc...)
* Hiredis libraries and headers (https://github.com/redis/hiredis or from precompiled packages via yum/apt etc...)
* Zabbix sources (http://www.zabbix.com/download.php)

## Installation

#### Module

* autoreconf -ifv
* ./configure --with-zabbix-headers=/usr/src/zabbix/include
* make
* make install

The libzbxredis.so library will be installed under /usr/local/lib and will need to be copied to the Zabbix module
directory which is usually /usr/lib64/zabbix/modules or /usr/lib/zabbix/modules.

#### Templates

* If Zabbix 2.2: Create Zabbix 2.2 Value Maps (./Zabbix-Resources/Zabbix-2.2/Value-Maps/README.md)
* If Zabbix 2.2: Import Zabbix 2.2 Templates  (./Zabbix-Resources/Zabbix-2.2/Templates/Template Redis Server.xml)
* If Zabbix 2.4: Create Zabbix 2.4 Value Maps (./Zabbix-Resources/Zabbix-2.4/Value-Maps/README.md)
* If Zabbix 2.4: Import Zabbix 2.4 Templates  (./Zabbix-Resources/Zabbix-2.4/Templates/Template Redis Server.xml)
* If Zabbix 3.0: Import Zabbix 3.0 Templates  (./Zabbix-Resources/Zabbix-3.0/Templates/Template Redis Server.xml)
* If Zabbix 3.2: Import Zabbix 3.2 Templates  (./Zabbix-Resources/Zabbix-3.2/Templates/Template Redis Server.xml)

## Tested Zabbix Agent Versions

* Zabbix 2.2 - Working
* Zabbix 2.4 - Working
* Zabbix 3.0 - Working
* Zabbix 3.2 - Working

## Tested Operating Systems

* CentOS 5/6/7 x86/x86_64 - Working
* Redhat 5/6/7 x86/x86_64 - Working
* Ubuntu 16.04 x86_64 - Working

## License

libzbxredis - A Redis monitoring module for Zabbix
Copyright (C) 2016 - James Cook <james.cook000@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.





