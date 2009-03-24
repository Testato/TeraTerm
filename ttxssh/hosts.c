/*
Copyright (c) 1998-2001, Robert O'Callahan
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.

The name of Robert O'Callahan may not be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
This code is copyright (C) 1998-1999 Robert O'Callahan.
See LICENSE.TXT for the license.
*/

#include "ttxssh.h"
#include "util.h"
#include "resource.h"
#include "matcher.h"

#include <openssl/bn.h>

#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <sys/stat.h>

static char FAR *FAR * parse_multi_path(char FAR * buf)
{
	int i;
	int ch;
	int num_paths = 1;
	char FAR *FAR * result;
	int last_path_index;

	for (i = 0; (ch = buf[i]) != 0; i++) {
		if (ch == ';') {
			num_paths++;
		}
	}

	result =
		(char FAR * FAR *) malloc(sizeof(char FAR *) * (num_paths + 1));

	last_path_index = 0;
	num_paths = 0;
	for (i = 0; (ch = buf[i]) != 0; i++) {
		if (ch == ';') {
			buf[i] = 0;
			result[num_paths] = _strdup(buf + last_path_index);
			num_paths++;
			buf[i] = ch;
			last_path_index = i + 1;
		}
	}
	if (i > last_path_index) {
		result[num_paths] = _strdup(buf + last_path_index);
		num_paths++;
	}
	result[num_paths] = NULL;
	return result;
}

void HOSTS_init(PTInstVar pvar)
{
	pvar->hosts_state.prefetched_hostname = NULL;
	pvar->hosts_state.key_exp = NULL;
	pvar->hosts_state.key_mod = NULL;
	pvar->hosts_state.hosts_dialog = NULL;
	pvar->hosts_state.file_names = NULL;
}

void HOSTS_open(PTInstVar pvar)
{
	pvar->hosts_state.file_names =
		parse_multi_path(pvar->session_settings.KnownHostsFiles);
}

static int begin_read_file(PTInstVar pvar, char FAR * name,
						   int suppress_errors)
{
	int fd;
	int length;
	int amount_read;
	char buf[2048];

	get_teraterm_dir_relative_name(buf, sizeof(buf), name);
	fd = _open(buf, _O_RDONLY | _O_SEQUENTIAL | _O_BINARY);
	if (fd == -1) {
		if (!suppress_errors) {
			if (errno == ENOENT) {
				notify_nonfatal_error(pvar,
									  "An error occurred while trying to read a known_hosts file.\n"
									  "The specified filename does not exist.");
			} else {
				notify_nonfatal_error(pvar,
									  "An error occurred while trying to read a known_hosts file.");
			}
		}
		return 0;
	}

	length = (int) _lseek(fd, 0, SEEK_END);
	_lseek(fd, 0, SEEK_SET);

	if (length >= 0 && length < 0x7FFFFFFF) {
		pvar->hosts_state.file_data = malloc(length + 1);
		if (pvar->hosts_state.file_data == NULL) {
			if (!suppress_errors) {
				notify_nonfatal_error(pvar,
									  "Memory ran out while trying to allocate space to read a known_hosts file.");
			}
			_close(fd);
			return 0;
		}
	} else {
		if (!suppress_errors) {
			notify_nonfatal_error(pvar,
								  "An error occurred while trying to read a known_hosts file.");
		}
		_close(fd);
		return 0;
	}

	amount_read = _read(fd, pvar->hosts_state.file_data, length);
	pvar->hosts_state.file_data[length] = 0;

	_close(fd);

	if (amount_read != length) {
		if (!suppress_errors) {
			notify_nonfatal_error(pvar,
								  "An error occurred while trying to read a known_hosts file.");
		}
		free(pvar->hosts_state.file_data);
		pvar->hosts_state.file_data = NULL;
		return 0;
	} else {
		return 1;
	}
}

static int end_read_file(PTInstVar pvar, int suppress_errors)
{
	free(pvar->hosts_state.file_data);
	pvar->hosts_state.file_data = NULL;
	return 1;
}

static int begin_read_host_files(PTInstVar pvar, int suppress_errors)
{
	pvar->hosts_state.file_num = 0;
	pvar->hosts_state.file_data = NULL;
	return 1;
}

static int eat_spaces(char FAR * data)
{
	int index = 0;
	int ch;

	while ((ch = data[index]) == ' ' || ch == '\t') {
		index++;
	}
	return index;
}

static int eat_digits(char FAR * data)
{
	int index = 0;
	int ch;

	while ((ch = data[index]) >= '0' && ch <= '9') {
		index++;
	}
	return index;
}

static int eat_to_end_of_line(char FAR * data)
{
	int index = 0;
	int ch;

	while ((ch = data[index]) != '\n' && ch != '\r' && ch != 0) {
		index++;
	}

	while ((ch = data[index]) == '\n' || ch == '\r') {
		index++;
	}

	return index;
}

static int eat_to_end_of_pattern(char FAR * data)
{
	int index = 0;
	int ch;

	while (ch = data[index], is_pattern_char(ch)) {
		index++;
	}

	return index;
}

static char FAR *parse_bignum(char FAR * data)
{
	uint32 digits = 0;
	BIGNUM *num = BN_new();
	BIGNUM *billion = BN_new();
	BIGNUM *digits_num = BN_new();
	BN_CTX *ctx = BN_CTX_new();
	char FAR *result;
	int ch;
	int leftover_digits = 1;

	BN_CTX_init(ctx);
	BN_set_word(num, 0);
	BN_set_word(billion, 1000000000L);

	while ((ch = *data) >= '0' && ch <= '9') {
		if (leftover_digits == 1000000000L) {
			BN_set_word(digits_num, digits);
			BN_mul(num, num, billion, ctx);
			BN_add(num, num, digits_num);
			leftover_digits = 1;
			digits = 0;
		}

		digits = digits * 10 + ch - '0';
		leftover_digits *= 10;
		data++;
	}

	BN_set_word(digits_num, digits);
	BN_set_word(billion, leftover_digits);
	BN_mul(num, num, billion, ctx);
	BN_add(num, num, digits_num);

	result = (char FAR *) malloc(2 + BN_num_bytes(num));
	set_ushort16_MSBfirst(result, BN_num_bits(num));
	BN_bn2bin(num, result + 2);

	BN_CTX_free(ctx);
	BN_free(digits_num);
	BN_free(num);
	BN_free(billion);

	return result;
}

static int check_host_key(PTInstVar pvar, char FAR * hostname,
						  char FAR * data)
{
	int index = eat_spaces(data);
	int matched = 0;
	int keybits = 0;

	if (data[index] == '#') {
		return index + eat_to_end_of_line(data + index);
	}

	/* if we find an empty line, then it won't have any patterns matching the hostname
	   and so we skip it */
	index--;
	do {
		int negated;

		index++;
		negated = data[index] == '!';

		if (negated) {
			index++;
			if (match_pattern(data + index, hostname)) {
				return index + eat_to_end_of_line(data + index);
			}
		} else if (match_pattern(data + index, hostname)) {
			matched = 1;
		}

		index += eat_to_end_of_pattern(data + index);
	} while (data[index] == ',');

	if (!matched) {
		return index + eat_to_end_of_line(data + index);
	} else {
		index += eat_spaces(data + index);

		pvar->hosts_state.key_bits = atoi(data + index);
		index += eat_digits(data + index);
		index += eat_spaces(data + index);

		pvar->hosts_state.key_exp = parse_bignum(data + index);
		index += eat_digits(data + index);
		index += eat_spaces(data + index);

		pvar->hosts_state.key_mod = parse_bignum(data + index);

		if (pvar->hosts_state.key_bits < 0
			|| pvar->hosts_state.key_exp == NULL
			|| pvar->hosts_state.key_mod == NULL) {
			pvar->hosts_state.key_bits = 0;
			free(pvar->hosts_state.key_exp);
			free(pvar->hosts_state.key_mod);
		}

		return index + eat_to_end_of_line(data + index);
	}
}

static int read_host_key(PTInstVar pvar, char FAR * hostname,
						 int suppress_errors)
{
	int i;

	for (i = 0; hostname[i] != 0; i++) {
		int ch = hostname[i];

		if (!is_pattern_char(ch) || ch == '*' || ch == '?') {
			if (!suppress_errors) {
				notify_fatal_error(pvar,
								   "The host name contains an invalid character.\n"
								   "This session will be terminated.");
			}
			return 0;
		}
	}

	if (i == 0) {
		if (!suppress_errors) {
			notify_fatal_error(pvar, "The host name should not be empty.\n"
							   "This session will be terminated.");
		}
		return 0;
	}

	pvar->hosts_state.key_bits = 0;
	free(pvar->hosts_state.key_exp);
	pvar->hosts_state.key_exp = NULL;
	free(pvar->hosts_state.key_mod);
	pvar->hosts_state.key_mod = NULL;

	do {
		if (pvar->hosts_state.file_data == NULL
			|| pvar->hosts_state.file_data[pvar->hosts_state.
										   file_data_index] == 0) {
			char FAR *filename;
			int keep_going = 1;

			if (pvar->hosts_state.file_data != NULL) {
				end_read_file(pvar, suppress_errors);
			}

			do {
				filename =
					pvar->hosts_state.file_names[pvar->hosts_state.
												 file_num];

				if (filename == NULL) {
					return 1;
				} else {
					pvar->hosts_state.file_num++;

					if (filename[0] != 0) {
						if (begin_read_file
							(pvar, filename, suppress_errors)) {
							pvar->hosts_state.file_data_index = 0;
							keep_going = 0;
						}
					}
				}
			} while (keep_going);
		}

		pvar->hosts_state.file_data_index +=
			check_host_key(pvar, hostname,
						   pvar->hosts_state.file_data +
						   pvar->hosts_state.file_data_index);
	} while (pvar->hosts_state.key_bits == 0);

	return 1;
}

static void finish_read_host_files(PTInstVar pvar, int suppress_errors)
{
	if (pvar->hosts_state.file_data != NULL) {
		end_read_file(pvar, suppress_errors);
	}
}

void HOSTS_prefetch_host_key(PTInstVar pvar, char FAR * hostname)
{
	if (!begin_read_host_files(pvar, 1)) {
		return;
	}

	if (!read_host_key(pvar, hostname, 1)) {
		return;
	}

	free(pvar->hosts_state.prefetched_hostname);
	pvar->hosts_state.prefetched_hostname = _strdup(hostname);

	finish_read_host_files(pvar, 1);
}

static BOOL equal_mp_ints(unsigned char FAR * num1,
						  unsigned char FAR * num2)
{
	if (num1 == NULL || num2 == NULL) {
		return FALSE;
	} else {
		uint32 bytes = (get_ushort16_MSBfirst(num1) + 7) / 8;

		if (bytes != (get_ushort16_MSBfirst(num2) + 7) / 8) {
			return FALSE;		/* different byte lengths */
		} else {
			return memcmp(num1 + 2, num2 + 2, bytes) == 0;
		}
	}
}

static BOOL match_key(PTInstVar pvar,
					  int bits, unsigned char FAR * exp,
					  unsigned char FAR * mod)
{
	/* just check for equal exponent and modulus */
	return equal_mp_ints(exp, pvar->hosts_state.key_exp)
		&& equal_mp_ints(mod, pvar->hosts_state.key_mod);
}

static void init_hosts_dlg(PTInstVar pvar, HWND dlg)
{
	char buf[1024];
	char buf2[2048];
	int i, j;
	int ch;

	GetDlgItemText(dlg, IDC_HOSTWARNING, buf, sizeof(buf));
	for (i = 0; (ch = buf[i]) != 0 && ch != '#'; i++) {
		buf2[i] = ch;
	}
	if (sizeof(buf2) - i - 1 > 0) {
		strncpy(buf2 + i, pvar->hosts_state.prefetched_hostname,
				sizeof(buf2) - i - 1);
	}
	j = i + strlen(buf2 + i);
	for (; buf[i] == '#'; i++) {
	}
	if (sizeof(buf2) - j - 1 > 0) {
		strncpy(buf2 + j, buf + i, sizeof(buf2) - j - 1);
	}
	buf2[sizeof(buf2) - 1] = 0;

	SetDlgItemText(dlg, IDC_HOSTWARNING, buf2);
}

static int print_mp_int(char FAR * buf, unsigned char FAR * mp)
{
	int i = 0, j, k;
	BIGNUM *num = BN_new();
	int ch;

	BN_bin2bn(mp + 2, (get_ushort16_MSBfirst(mp) + 7) / 8, num);

	do {
		buf[i] = (char) ((BN_div_word(num, 10)) + '0');
		i++;
	} while (!BN_is_zero(num));

	/* we need to reverse the digits */
	for (j = 0, k = i - 1; j < k; j++, k--) {
		ch = buf[j];
		buf[j] = buf[k];
		buf[k] = ch;
	}

	buf[i] = 0;
	return i;
}

static char FAR *format_host_key(PTInstVar pvar)
{
	int host_len = strlen(pvar->hosts_state.prefetched_hostname);
	char FAR *result = (char FAR *) malloc(host_len
										   + 50 +
										   get_ushort16_MSBfirst(pvar->
																 hosts_state.
																 key_exp) /
										   3 +
										   get_ushort16_MSBfirst(pvar->
																 hosts_state.
																 key_mod) /
										   3);
	int index;

	strcpy(result, pvar->hosts_state.prefetched_hostname);
	index = host_len;

	sprintf(result + index, " %d ", pvar->hosts_state.key_bits);
	index += strlen(result + index);
	index += print_mp_int(result + index, pvar->hosts_state.key_exp);
	result[index] = ' ';
	index++;
	index += print_mp_int(result + index, pvar->hosts_state.key_mod);
	strcpy(result + index, " \r\n");

	return result;
}

static void add_host_key(PTInstVar pvar)
{
	char FAR *name = pvar->hosts_state.file_names[0];

	if (name == NULL || name[0] == 0) {
		notify_nonfatal_error(pvar,
							  "The host and its key cannot be added, because no known-hosts file has been specified.\n"
							  "Restart Teraterm and specify a read/write known-hosts file in the TTSSH Setup dialog box.");
	} else {
		char FAR *keydata = format_host_key(pvar);
		int length = strlen(keydata);
		int fd =
			_open(name,
				  _O_APPEND | _O_CREAT | _O_WRONLY | _O_SEQUENTIAL |
				  _O_BINARY,
				  _S_IREAD | _S_IWRITE);
		int amount_written;
		int close_result;

		if (fd == -1) {
			if (errno == EACCES) {
				notify_nonfatal_error(pvar,
									  "An error occurred while trying to write the host key.\n"
									  "You do not have permission to write to the known-hosts file.");
			} else {
				notify_nonfatal_error(pvar,
									  "An error occurred while trying to write the host key.\n"
									  "The host key could not be written.");
			}
			return;
		}

		amount_written = _write(fd, keydata, length);
		free(keydata);
		close_result = _close(fd);

		if (amount_written != length || close_result == -1) {
			notify_nonfatal_error(pvar,
								  "An error occurred while trying to write the host key.\n"
								  "The host key could not be written.");
		}
	}
}

static BOOL CALLBACK hosts_dlg_proc(HWND dlg, UINT msg, WPARAM wParam,
									LPARAM lParam)
{
	PTInstVar pvar;

	switch (msg) {
	case WM_INITDIALOG:
		pvar = (PTInstVar) lParam;
		pvar->hosts_state.hosts_dialog = dlg;
		SetWindowLong(dlg, DWL_USER, lParam);

		init_hosts_dlg(pvar, dlg);
		return TRUE;			/* because we do not set the focus */

	case WM_COMMAND:
		pvar = (PTInstVar) GetWindowLong(dlg, DWL_USER);

		switch (LOWORD(wParam)) {
		case IDC_CONTINUE:
			if (IsDlgButtonChecked(dlg, IDC_ADDTOKNOWNHOSTS)) {
				add_host_key(pvar);
			}
			SSH_notify_host_OK(pvar);

			pvar->hosts_state.hosts_dialog = NULL;

			EndDialog(dlg, 1);
			return TRUE;

		case IDCANCEL:			/* kill the connection */
			pvar->hosts_state.hosts_dialog = NULL;
			notify_closed_connection(pvar);
			EndDialog(dlg, 0);
			return TRUE;

		default:
			return FALSE;
		}

	default:
		return FALSE;
	}
}

static char FAR *copy_mp_int(char FAR * num)
{
	int len = (get_ushort16_MSBfirst(num) + 7) / 8 + 2;
	char FAR *result = (char FAR *) malloc(len);

	if (result != NULL) {
		memcpy(result, num, len);
	}

	return result;
}

void HOSTS_do_unknown_host_dialog(HWND wnd, PTInstVar pvar)
{
	if (pvar->hosts_state.hosts_dialog == NULL) {
		HWND cur_active = GetActiveWindow();

		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SSHUNKNOWNHOST),
					   cur_active != NULL ? cur_active : wnd,
					   hosts_dlg_proc, (LPARAM) pvar);
	}
}

void HOSTS_do_different_host_dialog(HWND wnd, PTInstVar pvar)
{
	if (pvar->hosts_state.hosts_dialog == NULL) {
		HWND cur_active = GetActiveWindow();

		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SSHDIFFERENTHOST),
					   cur_active != NULL ? cur_active : wnd,
					   hosts_dlg_proc, (LPARAM) pvar);
	}
}

BOOL HOSTS_check_host_key(PTInstVar pvar, char FAR * hostname,
						  int bits, unsigned char FAR * exp,
						  unsigned char FAR * mod)
{
	int found_different_key = 0;

	if (pvar->hosts_state.prefetched_hostname != NULL
		&& stricmp(pvar->hosts_state.prefetched_hostname, hostname) == 0
		&& match_key(pvar, bits, exp, mod)) {
		SSH_notify_host_OK(pvar);
		return TRUE;
	}

	if (begin_read_host_files(pvar, 0)) {
		do {
			if (!read_host_key(pvar, hostname, 0)) {
				break;
			}

			if (pvar->hosts_state.key_bits > 0) {
				if (match_key(pvar, bits, exp, mod)) {
					finish_read_host_files(pvar, 0);
					SSH_notify_host_OK(pvar);
					return TRUE;
				} else {
					found_different_key = 1;
				}
			}
		} while (pvar->hosts_state.key_bits > 0);

		finish_read_host_files(pvar, 0);
	}

	pvar->hosts_state.key_bits = bits;
	pvar->hosts_state.key_exp = copy_mp_int(exp);
	pvar->hosts_state.key_mod = copy_mp_int(mod);
	free(pvar->hosts_state.prefetched_hostname);
	pvar->hosts_state.prefetched_hostname = _strdup(hostname);

	if (found_different_key) {
		PostMessage(pvar->NotificationWindow, WM_COMMAND,
					ID_SSHDIFFERENTHOST, 0);
	} else {
		PostMessage(pvar->NotificationWindow, WM_COMMAND,
					ID_SSHUNKNOWNHOST, 0);
	}

	return TRUE;
}

void HOSTS_notify_disconnecting(PTInstVar pvar)
{
	if (pvar->hosts_state.hosts_dialog != NULL) {
		PostMessage(pvar->hosts_state.hosts_dialog, WM_COMMAND, IDCANCEL,
					0);
		/* the main window might not go away if it's not enabled. (see vtwin.cpp) */
		EnableWindow(pvar->NotificationWindow, TRUE);
	}
}

void HOSTS_end(PTInstVar pvar)
{
	int i;

	free(pvar->hosts_state.prefetched_hostname);
	free(pvar->hosts_state.key_exp);
	free(pvar->hosts_state.key_mod);

	if (pvar->hosts_state.file_names != NULL) {
		for (i = 0; pvar->hosts_state.file_names[i] != NULL; i++) {
			free(pvar->hosts_state.file_names[i]);
		}
		free(pvar->hosts_state.file_names);
	}
}

/*
 * $Log: not supported by cvs2svn $
 */
