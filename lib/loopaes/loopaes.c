/*
 * loop-AES compatible volume handling
 *
 * Copyright (C) 2011, Red Hat, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypto_backend.h"
#include "loopaes.h"

static const char *get_hash(unsigned int key_size)
{
	char *hash;

	switch (key_size) {
		case 16: hash = "sha256"; break;
		case 24: hash = "sha384"; break;
		case 32: hash = "sha512"; break;
		default: hash = NULL;
	}

	return hash;
}

static unsigned char get_tweak(unsigned int keys_count)
{
	switch (keys_count) {
		case 64: return 0x55;
		case 65: return 0xF4;
		default: break;
	}
	return 0x00;
}

static int hash_key(const char *src, size_t src_len,
		    char *dst, size_t dst_len,
		    const char *hash_name)
{
	struct crypt_hash *hd = NULL;
	int r;

	if (crypt_hash_init(&hd, hash_name))
		return -EINVAL;

	r = crypt_hash_write(hd, src, src_len);
	if (!r)
		r = crypt_hash_final(hd, dst, dst_len);
out:
	crypt_hash_destroy(hd);
	return r;
}

static int hash_keys(struct volume_key **vk,
		     const char **input_keys,
		     unsigned int keys_count,
		     unsigned int key_len_output)
{
	const char *hash_name;
	char tweak, *key_ptr;
	int r, i, key_len_input;

	hash_name = get_hash(key_len_output);
	tweak = get_tweak(keys_count);
	key_len_input = strlen(input_keys[0]);

	if (!keys_count || !key_len_output || !hash_name || !key_len_input)
		return -EINVAL;

	*vk = crypt_alloc_volume_key(key_len_output * keys_count, NULL);
	if (!*vk)
		return -ENOMEM;

	for (i = 0; i < keys_count; i++) {
		key_ptr = &(*vk)->key[i * key_len_output];
		r = hash_key(input_keys[i], key_len_input, key_ptr,
			     key_len_output, hash_name);
		if (r < 0)
			break;

		key_ptr[0] ^= tweak;
	}

	if (r < 0 && *vk) {
		crypt_free_volume_key(*vk);
		*vk = NULL;
	}
	return r;
}

static int keyfile_is_gpg(char *buffer, unsigned int buffer_len)
{
	int r = 0;
	int index = buffer_len < 100 ? buffer_len - 1 : 100;
	char eos = buffer[index];

	buffer[index] = '\0';
	if (strstr(buffer, "BEGIN PGP MESSAGE"))
		r = 1;
	buffer[index] = eos;
	return r;
}

int LOOPAES_parse_keyfile(struct crypt_device *cd,
			  struct volume_key **vk,
			  unsigned int *keys_count,
			  char *buffer,
			  unsigned int buffer_len)

{
	const char *keys[LOOPAES_KEYS_MAX];
	int i, key_index, key_len, offset;

	log_dbg("Parsing loop-AES keyfile of size %d.", buffer_len);

	if (buffer_len < LOOPAES_KEYFILE_MINSIZE) {
		log_err(cd, _("Incompatible loop-AES keyfile detected.\n"));
		return -EINVAL;
	}

	if (keyfile_is_gpg(buffer, buffer_len)) {
		log_err(cd, "Detected not yet supported GPG encrypted keyfile.\n");
		log_std(cd, "Please use gpg --decrypt <KEYFILE> | cryptsetup --keyfile=- ...\n");
		return -EINVAL;
	}

	/* Remove EOL in buffer */
	for (i = 0; i < buffer_len; i++)
		if (buffer[i] == '\n' || buffer[i] == '\r')
			buffer[i] = '\0';

	offset = 0;
	key_index = 0;
	while (offset < buffer_len && key_index < LOOPAES_KEYS_MAX) {
		keys[key_index++] = &buffer[offset];
		while (offset < buffer_len && buffer[offset])
			offset++;
		while (offset < buffer_len && !buffer[offset])
			offset++;
	}

	/* All keys must be the same length */
	key_len = key_index ? strlen(keys[0]) : 0;
	for (i = 0; i < key_index; i++)
		if (key_len != strlen(keys[i])) {
			log_dbg("Unexpected length %d of key #%d (should be %d).",
				strlen(keys[i]), i, key_len);
			key_len = 0;
			break;
		}

	log_dbg("Keyfile: %d keys of length %d.", key_index, key_len);
	if (offset != buffer_len || key_len == 0 ||
	   (key_index != 1 && key_index !=64 && key_index != 65)) {
		log_err(cd, _("Incompatible loop-AES keyfile detected.\n"));
		return -EINVAL;
	}

	*keys_count = key_index;
	return hash_keys(vk, keys, key_index, crypt_get_volume_key_size(cd));
}

int LOOPAES_activate(struct crypt_device *cd,
		     const char *name,
		     const char *base_cipher,
		     unsigned int keys_count,
		     struct volume_key *vk,
		     uint32_t flags)
{
	uint64_t size, offset;
	char *cipher;
	const char *device;
	int read_only, r;

	size = 0;
	offset = crypt_get_data_offset(cd);
	device = crypt_get_device_name(cd);
	read_only = flags & CRYPT_ACTIVATE_READONLY;

	r = device_check_and_adjust(cd, device, 1, &size, &offset, &read_only);
	if (r)
		return r;

	if (keys_count == 1)
		r = asprintf(&cipher, "%s-%s", base_cipher, "cbc-plain");
	else
		r = asprintf(&cipher, "%s:%d-%s", base_cipher, 64, "cbc-lmk");
	if (r < 0)
		return -ENOMEM;

	log_dbg("Trying to activate loop-AES device %s using cipher %s.", name, cipher);
	r = dm_create_device(name, device,
			     cipher, CRYPT_LOOPAES,
			     crypt_get_uuid(cd),
			     size, 0, offset, vk->keylength, vk->key,
			     read_only, 0);

	if (!r && keys_count != 1 && !(dm_flags() & DM_LMK_SUPPORTED)) {
		log_err(cd, _("Kernel doesn't support loop-AES compatible mapping.\n"));
		r = -ENOTSUP;
	}

	free(cipher);
	return r;
}
