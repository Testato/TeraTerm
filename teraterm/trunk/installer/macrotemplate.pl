# 
# �}�N���R�}���h�̃h�L�������g�̂ЂȌ`�����
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
		print "$idfile��ID����v���Ă��܂��� ($n != $1)\n";
		exit(1);
	}
} else {
	die;
}


; TODO
; en/html/macro/command�Ƀt�@�C���쐬
; jp/html/macro/command�Ƀt�@�C���쐬
; en/html/macro/command/index.html �Ƀ����N�}��
; jp/html/macro/command/index.html �Ƀ����N�}��
; en/teraterm.hhc�Ƀ����N�}��
; en/teraterm.hhp��alias�ǉ�
; jp/teraterm.hhc�Ƀ����N�}��
; jp/teraterm.hhp��alias�ǉ�

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

