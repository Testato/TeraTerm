BUILD INSTRUCTIONS

The source package contains a project workspace file for MS Visual C++ 5.0.
The following directory structure is used:

.               Workspace files
.\ttxssh        Files for the project that builds TTXSSH.DLL
.\ttssh         Files for the project that builds TTSSH.EXE
                (just a small wrapper that launches the original
                TTERMPRO.EXE)
.\matcher       Files for the project that builds TEST.EXE which tests
                the micro-regexp matcher used when scanning
                ssh_known_hosts
.\Teraterm-2.3  You should obtain the source for TeraTerm 2.3 and
                dump it here
.\SSLeay-0.8.1  SSLeay 0.8.1, which you need to obtain the source
                for and build in this directory
                (must build LIBEAY32.DLL so that the
                right header files are generated). The LIBEAY32.DLL
                that I ship was built with a slight
		        tweak to the makefile to statically link the C library
                (otherwise it requires MSVCRT.DLL which not all
                machines have).
.\zlib          zlib 1.1.3; you should unpack the sources in here

LICENSE

This entire package is licensed under the BSD license. Here is the license text:

Copyright (c) 1998-2001, Robert O'Callahan
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.

The name of Robert O'Callahan may not be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
