# 
# マクロコマンドのドキュメント類のチェックを行う
#
# [実行方法]
#   __END__以降のマクロ列を検証する
#   >perl macrotemplate.pl
#
#   指定したマクロを検証する
#   >perl macrotemplate.pl scpsend
#
# [改版履歴]
# 1.0 (2008.02.16 Yutaka Hirata)
# 1.1 (2008.02.23 Yutaka Hirata)
#

$macroidfile = '..\source\ttmacro\ttmparse.h';
$helpidfile = '..\source\common\helpid.h';
$encmdfile = '..\..\doc\en\html\macro\command';
$jpcmdfile = '..\..\doc\jp\html\macro\command';
$enhhcfile = '..\..\doc\en\teraterm.hhc';
$jphhcfile = '..\..\doc\jp\teraterm.hhc';
$enhhpfile = '..\..\doc\en\teraterm.hhp';
$jphhpfile = '..\..\doc\jp\teraterm.hhp';
$keyfile = '..\release\keyfile.ini';

if ($#ARGV != -1) {
	print "$ARGV[0]\n";
	do_main(lc($ARGV[0]));

} else {
	while (<DATA>) {
		chomp;
		if (/(.+)=.*/) {
			$key = lc($1);
			print "==== $key マクロを検証中...\n";
			do_main($key);
		}
	}
}
exit(0);


#; TODO
#; en/teraterm.hhcにリンク挿入
#; en/teraterm.hhpにalias追加
#; jp/teraterm.hhcにリンク挿入
#; jp/teraterm.hhpにalias追加

sub do_main {
	my($macro) = @_;
	my($ret, $id, $s, $pat);
	my($idline, $helpline);
#	print "$macro\n";

	$s = "Rsv$macro\\b";
	$ret = read_keyword($macroidfile, $s);
	$idline = $ret;
#	print "$ret\n";
	if ($ret =~ /$macro\s+(\d+)/i) {
		$id = $1;
	} else {
		print "IDファイル($macroidfile)からマクロ定義が見つかりません\n";
		print "$idline\n";
		return;
	}
#	print "$id\n";

	$s = "Command$macro\\b";
	$ret = read_keyword($helpidfile, $s);
	$helpline = $ret;
#	print "$ret\n";
	if ($ret =~ /$macro\s+(\d+)/i) {
		$n = 92000 + $id;
		if ($n != $1) {
			print "$helpidfile のIDが一致していません ($n != $1)\n";
			print "$idline\n";
			print "$ret\n";
			return;
		}
	} else {
		print "HELPIDファイル($helpidfile)からマクロ定義が見つかりません\n";
		print "$idline\n";
		print "$ret\n";
		return;
	}
	
	$s = "$encmdfile\\$macro.html";
	if (!(-e $s)) {
		print "マクロコマンドの英語版説明文($s)がありません\n";
	}
	
	$s = "$jpcmdfile\\$macro.html";
	if (!(-e $s)) {
		print "マクロコマンドの日本語版説明文($s)がありません\n";
	}

	$s = "$encmdfile\\index.html";
	$pat = "$macro.html";
	$ret = read_keyword($s, $pat);
	if ($ret eq '') {
		print "$s ファイルに $pat へのリンクがありません\n";
	}
	
	$s = "$jpcmdfile\\index.html";
	$pat = "$macro.html";
	$ret = read_keyword($s, $pat);
	if ($ret eq '') {
		print "$s ファイルに $pat へのリンクがありません\n";
	}
	

	$pat = "$macro.html";
	$ret = read_keyword($enhhcfile, $pat);
	if ($ret eq '') {
		print "$enhhcfile ファイルに $pat へのリンクがありません\n";
	}

	$pat = "$macro.html";
	$ret = read_keyword($jphhcfile, $pat);
	if ($ret eq '') {
		print "$jphhcfile ファイルに $pat へのリンクがありません\n";
	}
	
	
	$pat = "$macro.html";
	$ret = read_keyword($enhhpfile, $pat);
	if ($ret eq '') {
		print "$enhhpfile ファイルに $pat へのALIASリンクがありません\n";
	}

	$pat = "$macro.html";
	$ret = read_keyword($jphhpfile, $pat);
	if ($ret eq '') {
		print "$jphhpfile ファイルに $pat へのALIASリンクがありません\n";
	}


	$pat = "\\b$macro\\b";
	$ret = read_keyword($keyfile, $pat);
	if ($ret eq '') {
		print "$keyfile ファイルに $pat コマンドがありません\n";
	}

}

sub read_keyword {
	my($file, $keyword) = @_;
	my($line) = '';
	my($found) = 0;
	
	open(FP, "$file") || die "Can't open $file.";
	while (<FP>) {
		chomp;
		$line = $_;
		if (/$keyword/i) {
			$found = 1;
			last;
		}
	}
	close(FP);
	
	if ($found == 0) {
		$line = '';
	}
	return ($line);
}

# コマンド列はkeyfile.iniから抜粋
__END__
Beep=92001
Bplusrecv=92002
Bplussend=92003
Break=92120
Call=92004
Callmenu=92125
Changedir=92005
Clearscreen=92006
Clipb2var=92113
Closesbox=92007
Closett=92008
Code2str=92009
Connect=92010
CygConnect=92130
Delpassword=92011
Disconnect=92012
Do=92126
Enablekeyb=92015
End=92016
EndUntil=92129
Exec=92019
Execcmnd=92020
Exit=92021
Fileclose=92022
Fileconcat=92023
Filecopy=92024
Filecreate=92025
Filedelete=92026
Filemarkptr=92027
Filenamebox=92124
Fileopen=92028
Fileread=92116
Filereadln=92029
Filerename=92030
Filesearch=92031
Fileseek=92032
Fileseekback=92033
Filestrseek=92034
Filestrseek2=92035
Filewrite=92036
Filewriteln=92037
Findoperations=92039
Flushrecv=92041
Fornext=92042
Getdate=92043
Getdir=92044
Getenv=92045
Getpassword=92046
Gettime=92047
Gettitle=92048
Getver=92133
Goto=92049
Ifdefined=92115
Ifthenelseif=92050
Include=92051
Inputbox=92052
Int2str=92053
Kmtfinish=92054
Kmtget=92055
Kmtrecv=92056
Kmtsend=92057
Loadkeymap=92058
Logclose=92059
Logopen=92060
Logpause=92061
Logstart=92062
Logwrite=92063
Loop=92127
Makepath=92064
Messagebox=92065
Mpause=92111
Passwordbox=92067
Pause=92068
Quickvanrecv=92069
Quickvansend=92070
Random=92112
Recvln=92071
Restoresetup=92072
Return=92073
Rotateleft=92122
Rotateright=92121
Scprecv=92131
Scpsend=92132
Send=92074
Sendbreak=92075
Sendfile=92076
Sendkcode=92077
Sendln=92078
Setbaud=92134
Setdate=92079
Setdir=92080
Setdlgpos=92081
Setenv=92123
Setecho=92082
Setexitcode=92083
Setsync=92084
Settime=92085
Settitle=92086
Show=92087
Showtt=92088
Sprintf=92117
Statusbox=92089
Str2code=92090
Str2int=92091
Strcompare=92092
Strconcat=92093
Strcopy=92094
Strlen=92095
Strscan=92096
Testlink=92097
Tolower=92118
Toupper=92119
Unlink=92099
Until=92128
Var2clipb=92114
Wait=92100
Waitevent=92101
Waitln=92102
Waitrecv=92103
Waitregex=92110
While=92104
Xmodemrecv=92105
Xmodemsend=92106
Yesnobox=92107
Zmodemrecv=92108
Zmodemsend=92109

