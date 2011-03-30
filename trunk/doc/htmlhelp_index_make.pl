#! /usr/bin/perl

#
# HTML�w���v�̃C���f�b�N�X�t�@�C���𐶐�����
#
# Usage(ActivePerl):
#  perl htmlhelp_index_make.pl
#

use Cwd;
@dirstack = (); 

do_main($ARGV[0], $ARGV[1]);

exit(0);

sub do_main {
	my($path, $body) = @_;

	print << 'EOD';
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<HTML><HEAD>
<meta name="GENERATOR" content="Tera Term Project">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<UL>
EOD

	push @dirstack, getcwd; 
	chdir $path; 
	get_file_paths($body);
	chdir pop @dirstack; 

	print << 'EOD';
</UL>
</BODY></HTML>
EOD

}


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
			check_html_file($full_path);
		
		}
	}
	return \@paths;
}

sub check_html_file {
	my($filename) = shift;
	local(*FP);
	my($line, $no);
	
	if ($filename !~ /.html$/) {
		return;
	}
	
	open(FP, "<$filename") || return;
	$no = 1;
	while ($line = <FP>) {
#		$line = chomp($line);
#		print "$line\n";
		if ($line =~ /<TITLE>(.+)<\/TITLE>/i) {
#			print "$filename:$no: $1\n";
#			print "$line\n";
			write_add_index($filename, $1);
			last;
		}

		$no++;
	}
	close(FP);
}

sub write_add_index {
	my($filename, $title) = @_;
	
	print << "EOD";
<LI><OBJECT type="text/sitemap">
<param name="Name" value="$title">
<param name="Local" value="$filename">
</OBJECT>
EOD

}

