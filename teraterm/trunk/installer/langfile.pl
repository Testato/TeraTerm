# 
# 言語ファイルのチェックを行う
#
# [実行方法]
#   >perl langfile.pl
#
# [改版履歴]
# 1.0 (2008.02.23 Yutaka Hirata)
#

$langfile = '..\release\lang\*.lng';
$secpattern = '^\[(.+)\]';

while (glob($langfile)) {
	do_main($_);
#	last;
}

exit(0);

sub do_main {
	my($file) = @_;
	local(@lines, $pos, %hash);
	my($section);
	
	print "===== $file を検証中\n";
	open(FP, $file) || die;
	@lines = <FP>;
	$pos = 0;
	close(FP);
	
#	print @lines;

	do {
		$section = read_section();
		print "$section セクション\n";
		read_entry();
	} while ($pos < @lines);
	
#	check_conflict();

}

sub read_section {
	my($line, $i, $s);
	
	$s = '';
	for ($i = $pos ; $i < @lines ; $i++) {
#		chomp($lines[$i]);
		if ($lines[$i] =~ /$secpattern/) {
			$s = $1;
#			print "Section: $s\n";
			$pos = $i + 1;
			last;
		}
	}
	return ($s);
}

sub read_entry {
	my($line, $i, $s);
	my($id, $val);
	
	for ($i = $pos ; $i < @lines ; $i++) {
#		chomp($lines[$i]);
#		print "$i: $lines[$i]";
		if ($lines[$i] =~ /^[\s\n]+$/ || 
			$lines[$i] =~ /^\s*;/
			) {
			# ignore
#			print "無視\n";
			
		} elsif ($lines[$i] =~ /$secpattern/) {
			last;
			
		} elsif ($lines[$i] =~ /(\w+)\s*=\s*(.+)/) {
			$id = $1;
			$val = $2;
#			print "$id と $val\n";
			
			# トップメニューのチェック
			if ($id eq 'MENU_FILE' ||
				$id eq 'MENU_EDIT' ||
				$id eq 'MENU_SETUP' ||
				$id eq 'MENU_CONTROL' ||
				$id eq 'MENU_WINDOW' ||
				$id eq 'MENU_HELP' ||
				$id eq 'MENU_KANJI') {
				if (check_invalid_key($val)) {
					print "$id エントリの $val には使用禁止のショートカットキーがあります\n";
				}
			}
			
			# ハッシュへ登録
			$hash{$id} = $val;
			
		} else {
			print "Unknown error.\n";
		}
	}
	
	$pos = $i;
}

sub check_invalid_key {
	my($arg) = @_;
	my($keys) = "TCIQNVRPBGD";
	my($key);
	
	if ($arg =~ /&(\w)/) {
		$key = uc($1);
		if (index($keys, $key) != -1) {  # NG!
			return 1;
		}
	}
	return 0;  # safe
}

sub check_conflict {
	my($key, $val);
	my(@pat) = qw(MENU_FILE MENU_TRANS_KERMIT MENU_TRANS_X MENU_TRANS_Z MENU_TRANS_BP MENU_TRANS_QV MENU_EDIT MENU_SETUP MENU_CONTROL MENU_WINDOW MENU_HELP TEKMENU_FILE TEKMENU_EDIT TEKMENU_SETUP TEKMENU_HELP MENU_SSH DLG_ABOUT DLG_AUTH DLG_AUTHSETUP );
	my($p);
	my(%char);
	
	foreach $p (@pat) {
		print "$p のショートカットキーの重複を調べています。\n";
		%char = ();
		foreach $key (keys %hash) {
			if ($key =~ /^$p/) {
				$val = $hash{$key};
#				print "$key ($val)\n";
				if ($val =~ /&(\w)/) {
					if (exists($char{$1})) {  # conflict
						print "$key の $val が他と重複しています\n";
					} else {
						$char{$1} = 1;
					}
				}
			}
		}
	}

}


