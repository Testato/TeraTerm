perl 2sjis.pl --i ..\source\oniguruma\doc\RE    --o ..\release\RE.txt               --c euc-jp      --l unix
perl 2sjis.pl --i ..\source\oniguruma\doc\RE.ja --o ..\release\RE-ja.txt            --c euc-jp      --l unix
perl 2sjis.pl --i ..\..\ttssh2\openssl\LICENSE  --o ..\release\OpenSSL-LICENSE.txt  --c euc-jp      --l unix
perl 2sjis.pl --i ..\cygterm\README             --o ..\release\cygterm-README.txt   --c iso-2022-jp --l crlf
perl 2sjis.pl --i ..\cygterm\README-j           --o ..\release\cygterm-README-j.txt --c iso-2022-jp --l crlf

perl 2sjis.pl --i ..\source\oniguruma\doc\RE    --o ..\..\doc\en\html\reference\RE.txt     --c euc-jp      --l unix
perl 2sjis.pl --i ..\source\oniguruma\doc\RE.ja --o ..\..\doc\jp\html\reference\RE-ja.txt  --c euc-jp      --l unix
perl 2sjis.pl --i ..\..\ttssh2\openssl\LICENSE  --o ..\..\doc\en\html\reference\OpenSSL-LICENSE.txt  --c euc-jp      --l unix
perl 2sjis.pl --i ..\..\ttssh2\openssl\LICENSE  --o ..\..\doc\jp\html\reference\OpenSSL-LICENSE.txt  --c euc-jp      --l unix

rem chmÇ…äÆëSÇ…à⁄çsÇ∑ÇÈÇ‹Ç≈ÇÃébíË
copy ..\release\keycode.txt ..\..\doc\en\html\reference\
copy ..\release\keycodej.txt ..\..\doc\jp\html\reference\
