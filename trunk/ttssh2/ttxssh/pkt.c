/*
 * Copyright (c) 1998-2001, Robert O'Callahan
 * (C) 2004-2017 TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
This code is copyright (C) 1998-1999 Robert O'Callahan.
See LICENSE.TXT for the license.
*/

#include "ttxssh.h"
#include "util.h"
#include "pkt.h"

#define READAMOUNT CHAN_SES_WINDOW_DEFAULT

void PKT_init(PTInstVar pvar)
{
	buf_create(&pvar->pkt_state.buf, &pvar->pkt_state.buflen);
	pvar->pkt_state.datastart = 0;
	pvar->pkt_state.datalen = 0;
	pvar->pkt_state.seen_server_ID = FALSE;
	pvar->pkt_state.seen_newline = FALSE;
	pvar->pkt_state.predecrypted_packet = FALSE;
}

/* Read some data, leave no more than up_to_amount bytes in the buffer,
   return the number of bytes read or -1 on error or blocking. */
static int recv_data(PTInstVar pvar, unsigned long up_to_amount)
{
	int amount_read;

	/* Shuffle data to the start of the buffer */
	if (pvar->pkt_state.datastart != 0) {
		memmove(pvar->pkt_state.buf,
		        pvar->pkt_state.buf + pvar->pkt_state.datastart,
		        pvar->pkt_state.datalen);
		pvar->pkt_state.datastart = 0;
	}

	buf_ensure_size(&pvar->pkt_state.buf, &pvar->pkt_state.buflen, up_to_amount);

	_ASSERT(pvar->pkt_state.buf != NULL);

	amount_read = (pvar->Precv) (pvar->socket,
	                             pvar->pkt_state.buf + pvar->pkt_state.datalen,
	                             up_to_amount - pvar->pkt_state.datalen,
	                             0);

	if (amount_read > 0) {
		/* Update seen_newline if necessary */
		if (!pvar->pkt_state.seen_server_ID && !pvar->pkt_state.seen_newline) {
			int i;

			for (i = 0; i < amount_read; i++) {
				if (pvar->pkt_state.buf[pvar->pkt_state.datalen + i] == '\n') {
					pvar->pkt_state.seen_newline = 1;
				}
			}
		}
		pvar->pkt_state.datalen += amount_read;
	}

	return amount_read;
}

// ���s�R�[�h���o�Ă���܂œǂ�
static int recv_line_data(PTInstVar pvar)
{
	int amount_read;
	char buf[256];
	size_t up_to_amount = sizeof(buf);
	int i;

	/* Shuffle data to the start of the buffer */
	if (pvar->pkt_state.datastart != 0) {
		memmove(pvar->pkt_state.buf,
		        pvar->pkt_state.buf + pvar->pkt_state.datastart,
		        pvar->pkt_state.datalen);
		pvar->pkt_state.datastart = 0;
	}

	buf_ensure_size(&pvar->pkt_state.buf, &pvar->pkt_state.buflen, up_to_amount);

	for (i = 0 ; i < (int)up_to_amount ; i++) {
		amount_read = (pvar->Precv) (pvar->socket, &buf[i], 1, 0);
		if (amount_read != 1) {
			return 0; // error
		} 

		pvar->pkt_state.datalen += amount_read;

		if (buf[i] == '\n') { // 0x0a
			buf[i+1] = 0;
			break;
		}
	}
	amount_read = i + 1; // �ǂݍ��݃T�C�Y�iLF���܂ށj
	memcpy(pvar->pkt_state.buf, buf, amount_read);

	pvar->pkt_state.seen_newline = 1;

	return amount_read;
}

/* This function does two things:
   -- reads data from the sshd and feeds the SSH protocol packets to ssh.c
   -- copies any available decrypted session data into the application buffer
*/
int PKT_recv(PTInstVar pvar, char *buf, int buflen)
{
	int amount_in_buf = 0;
	BOOL connection_closed = FALSE;

	while (SSH_is_any_payload(pvar) ? buflen > 0 : !connection_closed) {
		if (SSH_is_any_payload(pvar)) {
			/* ssh.c has some session data for us to give to Tera Term. */
			int grabbed = SSH_extract_payload(pvar, buf, buflen);

			amount_in_buf += grabbed;
			buf += grabbed;
			buflen -= grabbed;
		}
		else if (!pvar->pkt_state.seen_server_ID && (pvar->pkt_state.seen_newline || pvar->pkt_state.datalen >= 255)) {
			/*
			 * We're looking for the initial ID string and either we've seen the
			 * terminating newline, or we've exceeded the limit at which we should see a newline.
			 */
			unsigned int i;

			for (i = 0; pvar->pkt_state.buf[i] != '\n' && i < pvar->pkt_state.datalen; i++) {
			}
			if (pvar->pkt_state.buf[i] == '\n') {
				i++;
			}

			// SSH�T�[�o�̃o�[�W�����`�F�b�N���s��
			if (SSH_handle_server_ID(pvar, pvar->pkt_state.buf, i)) {
				pvar->pkt_state.seen_server_ID = 1;

				if (SSHv2(pvar)) {
					// send Key Exchange Init
					SSH2_send_kexinit(pvar);
				}
			} else {
				// reset flag to re-read server ID (2008.1.24 yutaka)
				pvar->pkt_state.seen_newline = 0;
			}

			pvar->pkt_state.datastart += i;
			pvar->pkt_state.datalen -= i;
		}
		else if (pvar->pkt_state.seen_server_ID && pvar->pkt_state.datalen >= SSH_get_min_packet_size(pvar)) {
			char *data = pvar->pkt_state.buf + pvar->pkt_state.datastart;
			uint32 padding;
			uint32 pktsize;
			uint32 total_packet_size;
			struct Mac *mac = &pvar->ssh2_keys[MODE_IN].mac;
			int etm;

			etm = mac && mac->enabled && mac->etm;

			/*
			 * �ʏ�� MAC ���� (E&M: Encrypt & MAC) �ł̓p�P�b�g���������Í�������Ă��邽�߁A
			 * �擪�� 1 �u���b�N�𕜍�����BMAC ������ EtM (Encrypt then MAC) �̎���
			 * �p�P�b�g�������͈Í�������Ă��Ȃ��̂ŕ����͕K�v�����B
			 */
			if (!pvar->pkt_state.predecrypted_packet && !etm) {
				SSH_predecrpyt_packet(pvar, data);
				pvar->pkt_state.predecrypted_packet = TRUE;
			}

			if (SSHv1(pvar)) {
				uint32 realpktsize = get_uint32_MSBfirst(data);

				padding = 8 - (realpktsize % 8);
				pktsize = realpktsize + padding;
			} else {
				// SSH2 �ł̓p�P�b�g�̐擪�� uint32 (4�o�C�g) �̃p�P�b�g��������
				pktsize = get_uint32_MSBfirst(data);

				// ���� 1 �o�C�g�� padding �̒���
				if (etm) {
					// EtM �ł� padding length �ȍ~�͈Í�������Ă���B
					// ���̎��_�ł͂܂��������Ă��Ȃ��̂� padding length ��������Ȃ��B
					// ���� 0 �����Ēu���B
					padding = 0;
				}
				else {
					// E&M �ł͕����ς�
					padding = (unsigned char) data[4];
				}
			}

			// �p�P�b�g(TCP�y�C���[�h)�̑S�̂̃T�C�Y�́ASSH�y�C���[�h+4�i+MAC�j�ƂȂ�B
			// +4�́ASSH�y�C���[�h�̃T�C�Y���i�[���Ă��镔���iint�^�j�B
			total_packet_size = pktsize + 4 + SSH_get_clear_MAC_size(pvar);

			if (total_packet_size <= pvar->pkt_state.datalen) {
				// ��M�ς݃f�[�^���\���L��ꍇ�̓p�P�b�g�̎��������s��
				if (SSHv1(pvar)) {
					// SSH1 �� EtM ��Ή�
					SSH1_handle_packet(pvar, data, pktsize, padding);
				}
				else {
					SSH2_handle_packet(pvar, data, pktsize, padding, etm);
				}

				pvar->pkt_state.predecrypted_packet = FALSE;
				pvar->pkt_state.datastart += total_packet_size;
				pvar->pkt_state.datalen -= total_packet_size;

			} else if (total_packet_size > PACKET_MAX_SIZE) {
				// �p�P�b�g�����傫������ꍇ�ُ͈�I������B
				// ���ۂɂ͉��炩�̗v���ŕ������s�˃p�P�b�g�����������Ă��鎖�������B
				UTIL_get_lang_msg("MSG_PKT_OVERSIZED_ERROR", pvar,
				                  "Oversized packet received from server; connection will close.");
				notify_fatal_error(pvar, pvar->ts->UIMsg, TRUE);
			} else {
				int amount_read = recv_data(pvar, max(total_packet_size, READAMOUNT));

				if (amount_read == SOCKET_ERROR) {
					if (amount_in_buf == 0) {
						return SOCKET_ERROR;
					} else {
						return amount_in_buf;
					}
				} else {
					if (amount_read == 0) {
						connection_closed = TRUE;
					}
				}
			}
		} else {
			// �p�P�b�g�̎�M
			int amount_read;

			amount_read = recv_data(pvar, READAMOUNT);

			if (amount_read == SOCKET_ERROR) {
				if (amount_in_buf == 0) {
					return SOCKET_ERROR;
				} else {
					return amount_in_buf;
				}
			} else if (amount_read == 0) {
				connection_closed = TRUE;
			}
		}

		if (pvar->fatal_error) {
			return amount_in_buf;
		}
	}

	if (SSH_is_any_payload(pvar)) {
		PostMessage(pvar->NotificationWindow, WM_USER_COMMNOTIFY, pvar->socket, MAKELPARAM(FD_READ, 0));
	}

	return amount_in_buf;
}

void PKT_end(PTInstVar pvar)
{
	buf_destroy(&pvar->pkt_state.buf, &pvar->pkt_state.buflen);
}
