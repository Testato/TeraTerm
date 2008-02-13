# 
# マクロコマンドのドキュメントのひな形を作る
#

$macro = $ARGV[0];
print "$macro\n";

$idfile = '..\source\ttmacro\ttmparse.h';
$ret = read_keyword($idfile, $macro);
print "$ret\n";
if ($ret =~ /$macro\s+(\d+)/i) {
	$id = $1;
} else {
	die;
}
print "$id\n";


$idfile = '..\source\common\helpid.h';
$ret = read_keyword($idfile, $macro);
print "$ret\n";
if ($ret =~ /$macro\s+(\d+)/i) {
	$n = 92000 + $id;
	if ($n != $1) {
		print "$idfileのIDが一致していません ($n != $1)\n";
		exit(1);
	}
} else {
	die;
}


; TODO
; en/html/macro/commandにファイル作成
; jp/html/macro/commandにファイル作成
; en/html/macro/command/index.html にリンク挿入
; jp/html/macro/command/index.html にリンク挿入
; en/teraterm.hhcにリンク挿入
; en/teraterm.hhpにalias追加
; jp/teraterm.hhcにリンク挿入
; jp/teraterm.hhpにalias追加

exit(0);

sub read_keyword {
	my($file, $keyword) = @_;
	my($line) = '';
	
	open(FP, "$file") || die "Can't open $file.";
	while (<FP>) {
		$line = $_;
		if (/$keyword/i) {
			last;
		}
	}
	close(FP);
	
	return ($line);
}

