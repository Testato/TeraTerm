# 
# ����t�@�C���̃`�F�b�N���s��
#
# [���s���@]
#   >perl langfile.pl
#
# [���ŗ���]
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
	
	print "===== $file �����ؒ�\n";
	open(FP, $file) || die;
	@lines = <FP>;
	$pos = 0;
	close(FP);
	
#	print @lines;

	print "�g�p�֎~�V���[�g�J�b�g��������\n";
	do {
		$section = read_section();
		print "$section �Z�N�V����\n";
		read_entry();
	} while ($pos < @lines);
	
	print "�V���[�g�J�b�g�̏d����������\n";
	check_conflict();

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
#			print "����\n";
			
		} elsif ($lines[$i] =~ /$secpattern/) {
			last;
			
		} elsif ($lines[$i] =~ /(\w+)\s*=\s*(.+)/) {
			$id = $1;
			$val = $2;
#			print "$id �� $val\n";
			
			# �g�b�v���j���[�̃`�F�b�N
			if ($id eq 'MENU_FILE' ||
				$id eq 'MENU_EDIT' ||
				$id eq 'MENU_SETUP' ||
				$id eq 'MENU_CONTROL' ||
				$id eq 'MENU_WINDOW' ||
				$id eq 'MENU_HELP' ||
				$id eq 'MENU_KANJI') {
				if (check_invalid_key($val)) {
					print "$id �G���g���� $val �ɂ͎g�p�֎~�̃V���[�g�J�b�g�L�[������܂�\n";
				}
			}
			
			# �n�b�V���֓o�^
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
	my($line, @lines2);
	my($section, $key, $val);
	my($line2, $shortcut, $samelevel);
	my($key2, $val2);
	
	@lines2 = @lines;
	foreach $line (@lines) {
		if ($line =~ /^\[(.+)\]$/) {
			$section = $1;
			next;
		}
		elsif ($line =~ /^(.+)=(.+)$/) {
			$key = $1;
			$val = $2;
			if ($val =~ /&(\w)/) {
				$shortcut = $1;
				$samelevel = $key;
				$samelevel =~ s/(\w+)_[a-zA-Z0-9]+/\1/;
				# print "$key $samelevel $shortcut\n";
				foreach $line2 (@lines2) {
					if ($line2 =~ /^\[(.+)\]$/) {
						$section2 = $1;
						next;
					}
					if ($section ne $section2) {
						next;
					}
					elsif ($line2 =~ /^(${samelevel}_[a-zA-Z0-9]+)=(.+)$/) {
						$key2 = $1;
						$val2 = $2;
						if ($key2 eq $key) {
							next;
						}
						if ($val2 =~ /&$shortcut/i) {
							print "[$key=$val] and [$key2=$val2] conflict [&$shortcut]\n";
						}
					}
				}
			}
		}
	}

}
