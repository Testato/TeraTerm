rem 
rem [no-capieng]
rem CAPI�G���W��(engines/e_capi.c)��Crypt32.lib(XP�ȍ~)�𗘗p���邽�߁A���O����B
rem 
rem [-D_WIN32_WINNT=0x0501]
rem VS2005�Ńr���h���邽�߂�workaround�B
rem cf. https://github.com/openssl/openssl/issues/1505
rem 

cd openssl

if exist "out32.dbg\libcrypto.lib" goto build_dbg_end
perl Configure no-asm no-async no-shared no-capieng -D_WIN32_WINNT=0x0501 VC-WIN32 --debug
perl -e "open(IN,'makefile');while(<IN>){s| /MDd| /MTd|;print $_;}close(IN);" > makefile.tmp
if exist "makefile.dbg" del makefile.dbg
ren makefile.tmp makefile.dbg
nmake -f makefile.dbg clean
nmake -f makefile.dbg
mkdir out32.dbg
move libcrypto.lib out32.dbg
move libssl.lib out32.dbg
:build_dbg_end

if exist "out32\libcrypto.lib" goto build_end
perl Configure no-asm no-async no-shared no-capieng -D_WIN32_WINNT=0x0501 VC-WIN32
perl -e "open(IN,'makefile');while(<IN>){s| /MD| /MT|;print $_;}close(IN);" > makefile.tmp
if exist "makefile" del makefile
ren makefile.tmp makefile
nmake clean
nmake
mkdir out32
move libcrypto.lib out32
move libssl.lib out32
:build_end

cd ..
