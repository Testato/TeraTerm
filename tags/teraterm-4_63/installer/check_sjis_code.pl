#! /usr/bin/perl

#
# �p��Ńh�L�������g�ɓ��{�ꂪ�܂܂�Ă��Ȃ����𒲂ׂ�B
#
# Usage(ActivePerl):
#  perl check_sjis_code.pl > result.txt
#

get_file_paths('../doc/en/html');
exit(0);

sub get_file_paths {
	my ($top_dir)= @_;
	my @paths=();
	my @temp = ();

	#-- �J�����g�̈ꗗ���擾 --#
	opendir(DIR, $top_dir);
	@temp = readdir(DIR);
	closedir(DIR);
	foreach my $path (sort @temp) {
		next if( $path =~ /^\.{1,2}$/ );                # '.' �� '..' �̓X�L�b�v
		next if( $path =~ /^\.svn$/ );                # '.svn' �̓X�L�b�v
		
		my $full_path = "$top_dir" . '/' . "$path";
		next if (-B $full_path);     # �o�C�i���t�@�C���̓X�L�b�v
		
#		print "$full_path\r\n";                     # �\�������Ȃ�S�Ă�\�����Ă����-------
		push(@paths, $full_path);                       # �f�[�^�Ƃ��Ď�荞��ł��O�̎�荞�݂������������
		if( -d "$top_dir/$path" ){                      #-- �f�B���N�g���̏ꍇ�͎������g���Ăяo��
			&get_file_paths("$full_path");
			
		} else {
			check_sjis_code($full_path);
		
		}
	}
	return \@paths;
}


# cf. http://charset.7jp.net/sjis.html
# ShiftJIS ����

sub check_sjis_code {
	my($filename) = shift;
	local(*FP);
	my($line, $no);
	
	open(FP, "<$filename") || return;
	$no = 1;
	while ($line = <FP>) {
#		$line = chomp($line);
#		print "$line\n";
		if ($line =~ /([\xA1-\xDF]|[\x81-\x9F\xE0-\xEF][\x40-\x7E\x80-\xFC])/) {
			print "$filename:$no: $1\n";
			print "$line\n";
		}
		$no++;
	}
	close(FP);
}

