#! /usr/bin/perl

#
# �p��Ńh�L�������g�ɓ��{�ꂪ�܂܂�Ă��Ȃ����𒲂ׂ�B
#
# Usage(ActivePerl):
#  perl check_sjis_code.pl > result.txt
#

use Encode::Guess qw/shift-jis 7bit-jis/;

#my @exclude_files = qw(sourcecode.html);
my @exclude_files = qw();

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
			
		} elsif (&check_exclude_file($path)) {
			print "$full_path skipped\n";
			next;
			
		} else {
			check_sjis_code($full_path);
		
		}
	}
	return \@paths;
}


# �����ΏۊO�̃t�@�C�����𒲂ׂ�
sub check_exclude_file {
	my($fn) = shift;
	my($s);
	
	foreach $s (@exclude_files) {
		if ($fn eq $s) {
			return 1;
		}
	}
	return 0;
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
		
		 my $enc = guess_encoding( $line, qw/ euc-jp shiftjis 7bit-jis utf8 / );

		if (ref $enc) {
#			printf "%s\n", $enc->name;
			if ($enc->name !~ /ascii/) {
#				printf "%s\n", $enc->name;
				print "$filename:$no: $1\n";
				print "$line\n";
			}
		}
#		if ($line =~ /([\xA1-\xDF]|[\x81-\x9F\xE0-\xEF][\x40-\x7E\x80-\xFC])/) {
#			print "$filename:$no: $1\n";
#			print "$line\n";
#		}
		$no++;
	}
	close(FP);
}

