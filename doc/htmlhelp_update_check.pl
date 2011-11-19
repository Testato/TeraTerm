#! /usr/bin/perl

#
# HTML�w���v�\�[�X���ЂƂł��X�V����Ă��邩�`�F�b�N����
#
# Usage(ActivePerl):
#  perl htmlhelp_update_check.pl ja chm_file_name
#

use Cwd;
@dirstack = ();

$ret = do_main($ARGV[0], $ARGV[1]);
if ($ret) {
	print "updated";
}

exit(0);

sub do_main {
	my($path, $chm) = @_;
	my(@filestat) = stat "$path/$chm";

	return get_file_paths($path, $filestat[9]);
}


sub get_file_paths {
	my ($top_dir, $chmupdated)= @_;
	my @paths=();
	my @temp = ();

	#-- �J�����g�̈ꗗ���擾 --#
	opendir(DIR, $top_dir);
	@temp = readdir(DIR);
	closedir(DIR);
	foreach my $path (sort @temp) {
		next if( $path =~ /^\.{1,2}$/ );              # '.' �� '..' �̓X�L�b�v
		next if( $path =~ /^\.svn$/ );                # '.svn' �̓X�L�b�v
		next if( $path =~ /^.\.chm$/ );               # '*.chm' �̓X�L�b�v
		
		my $full_path = "$top_dir" . '/' . "$path";
		
		if( -d "$top_dir/$path" ){                      #-- �f�B���N�g���̏ꍇ�͎������g���Ăяo��
			if (&get_file_paths("$full_path", $chmupdated)) {
				return 1;
			}
		} else {
			if (&check_file($full_path, $chmupdated)) {
				return 1;
			}
		}
	}
	return 0;
}

sub check_file {
	my($filename, $chmupdated) = @_;
	my(@filestat) = stat $filename;
	
	if ($filestat[9] > $chmupdated) {
		return 1;
	}
	return 0;
}
