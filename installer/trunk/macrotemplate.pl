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
# 1.2 (2009.03.03 Yutaka Hirata)
#

$macroidfile = '..\teraterm\ttpmacro\ttmparse.h';
$helpidfile = '..\teraterm\common\helpid.h';
$encmdfile = '..\doc\en\html\macro\command';
$jpcmdfile = '..\doc\jp\html\macro\command';
$enhhcfile = '..\doc\en\teraterm.hhc';
$jphhcfile = '..\doc\jp\teraterm.hhc';
$enhhpfile = '..\doc\en\teraterm.hhp';
$jphhpfile = '..\doc\jp\teraterm.hhp';
$keyfile = 'release\keyfile.ini';

if ($#ARGV != -1) {
	print "$ARGV[0]\n";
	do_main(lc($ARGV[0]));

} else {
	while (<DATA>) {
		chomp;
		if (/#define\s+Rsv(\w+)\s+.*/) {
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

	# 置換
	if ($macro eq 'else' ||
	    $macro eq 'elseif' ||
	    $macro eq 'endif'
	   ) {
		$macro = 'Ifthenelseif';
	}
	if ($macro eq 'endwhile'
	   ) {
		$macro = 'while';
	}
	if ($macro eq 'findfirst' ||
	    $macro eq 'findnext' ||
	    $macro eq 'findclose'
	   ) {
		$macro = 'Findoperations';
	}

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

# コマンド列は ttmparse.h から抜粋
__END__
#define RsvBeep         1
#define RsvBPlusRecv    2
#define RsvBPlusSend    3
#define RsvCall         4
#define RsvChangeDir    5
#define RsvClearScreen  6
#define RsvCloseSBox    7
#define RsvCloseTT      8
#define RsvCode2Str     9
#define RsvConnect      10
#define RsvDelPassword  11
#define RsvDisconnect   12
#define RsvElse         13
#define RsvElseIf       14
#define RsvEnableKeyb   15
#define RsvEnd          16
#define RsvEndIf        17
#define RsvEndWhile     18
#define RsvExec         19
#define RsvExecCmnd     20
#define RsvExit         21
#define RsvFileClose    22
#define RsvFileConcat   23
#define RsvFileCopy     24
#define RsvFileCreate   25
#define RsvFileDelete   26
#define RsvFileMarkPtr  27
#define RsvFileOpen     28
#define RsvFileReadln   29
#define RsvFileRename   30
#define RsvFileSearch   31
#define RsvFileSeek     32
#define RsvFileSeekBack 33
#define RsvFileStrSeek  34
#define RsvFileStrSeek2 35
#define RsvFileWrite    36
#define RsvFileWriteLn  37
#define RsvFindClose    38
#define RsvFindFirst    39
#define RsvFindNext     40
#define RsvFlushRecv    41
#define RsvFor          42
#define RsvGetDate      43
#define RsvGetDir       44
#define RsvGetEnv       45
#define RsvGetPassword  46
#define RsvGetTime      47
#define RsvGetTitle     48
#define RsvGoto         49
#define RsvIf           50
#define RsvInclude      51
#define RsvInputBox     52
#define RsvInt2Str      53
#define RsvKmtFinish    54
#define RsvKmtGet       55
#define RsvKmtRecv      56
#define RsvKmtSend      57
#define RsvLoadKeyMap   58
#define RsvLogClose     59
#define RsvLogOpen      60
#define RsvLogPause     61
#define RsvLogStart     62
#define RsvLogWrite     63
#define RsvMakePath     64
#define RsvMessageBox   65
#define RsvNext         66
#define RsvPasswordBox  67
#define RsvPause        68
#define RsvQuickVANRecv 69
#define RsvQuickVANSend 70
#define RsvRecvLn       71
#define RsvRestoreSetup 72
#define RsvReturn       73
#define RsvSend         74
#define RsvSendBreak    75
#define RsvSendFile     76
#define RsvSendKCode    77
#define RsvSendLn       78
#define RsvSetDate      79
#define RsvSetDir       80
#define RsvSetDlgPos    81
#define RsvSetEcho      82
#define RsvSetExitCode  83
#define RsvSetSync      84
#define RsvSetTime      85
#define RsvSetTitle     86
#define RsvShow         87
#define RsvShowTT       88
#define RsvStatusBox    89
#define RsvStr2Code     90
#define RsvStr2Int      91
#define RsvStrCompare   92
#define RsvStrConcat    93
#define RsvStrCopy      94
#define RsvStrLen       95
#define RsvStrScan      96
#define RsvTestLink     97
#define RsvThen         98
#define RsvUnlink       99
#define RsvWait         100
#define RsvWaitEvent    101
#define RsvWaitLn       102
#define RsvWaitRecv     103
#define RsvWhile        104
#define RsvXmodemRecv   105
#define RsvXmodemSend   106
#define RsvYesNoBox     107
#define RsvZmodemRecv   108
#define RsvZmodemSend   109
#define RsvWaitRegex    110   // add 'waitregex' (2005.10.5 yutaka)
#define RsvMilliPause   111   // add 'mpause' (2006.2.10 yutaka)
#define RsvRandom       112   // add 'random' (2006.2.11 yutaka)
#define RsvClipb2Var    113   // add 'clipb2var' (2006.9.17 maya)
#define RsvVar2Clipb    114   // add 'var2clipb' (2006.9.17 maya)
#define RsvIfDefined    115   // add 'ifdefined' (2006.9.23 maya)
#define RsvFileRead     116   // add 'fileread' (2006.11.1 yutaka)
#define RsvSprintf      117   // add 'sprintf' (2007.5.1 yutaka)
#define RsvToLower      118   // add 'tolower' (2007.7.12 maya)
#define RsvToUpper      119   // add 'toupper' (2007.7.12 maya)
#define RsvBreak        120   // add 'break' (2007.7.20 doda)
#define RsvRotateR      121   // add 'rotateright' (2007.8.19 maya)
#define RsvRotateL      122   // add 'rotateleft' (2007.8.19 maya)
#define RsvSetEnv       123   // reactivate 'setenv' (2007.8.31 maya)
#define RsvFilenameBox  124   // add 'filenamebox' (2007.9.13 maya)
#define RsvCallMenu     125   // add 'callmenu' (2007.11.18 maya)
#define RsvDo           126   // add 'do' (2007.11.20 doda)
#define RsvLoop         127   // add 'loop' (2007.11.20 doda)
#define RsvUntil        128   // add 'until' (2007.11.20 doda)
#define RsvEndUntil     129   // add 'enduntil' (2007.11.20 doda)
#define RsvCygConnect   130   // add 'cygconnect' (2007.12.17 doda)
#define RsvScpRecv      131   // add 'scprecv' (2008.1.1 yutaka)
#define RsvScpSend      132   // add 'scpsend' (2008.1.1 yutaka)
#define RsvGetVer       133   // add 'getver'  (2008.2.4 yutaka)
#define RsvSetBaud      134   // add 'setbaud' (2008.2.13 yutaka)
#define RsvStrMatch     135   // add 'strmatch' (2008.3.26 yutaka)
#define RsvSetRts       136   // add 'setrts'  (2008.3.12 maya)
#define RsvSetDtr       137   // add 'setdtr'  (2008.3.12 maya)
#define RsvCrc32        138   // add 'crc32'  (2008.9.12 yutaka)
#define RsvCrc32File    139   // add 'crc32file'  (2008.9.13 yutaka)
#define RsvGetTTDir     140   // add 'getttdir'  (2008.9.20 maya)
#define RsvGetHostname  141   // add 'gethostname'  (2008.12.15 maya)
#define RsvSprintf2     142   // add 'sprintf2'  (2008.12.18 maya)
#define RsvWaitN        143   // add 'waitn'  (2009.1.26 maya)
#define RsvSendBroadcast    144

