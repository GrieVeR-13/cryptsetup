#include "integrity.h"
#include "string.h"

int INTEGRITY_read_sb(struct crypt_device *cd,
		      struct crypt_params_integrity *params,
		      uint32_t *flags){ abort(); }

int INTEGRITY_dump(struct crypt_device *cd, struct device *device, uint64_t offset){ abort(); }

int INTEGRITY_data_sectors(struct crypt_device *cd,
			   struct device *device, uint64_t offset,
			   uint64_t *data_sectors){ abort(); }
int INTEGRITY_key_size(struct crypt_device *cd, const char *integrity){ abort();
}
int INTEGRITY_tag_size(struct crypt_device *cd __attribute__((unused)),
					   const char *integrity,
					   const char *cipher,
					   const char *cipher_mode)
{
	int iv_tag_size = 0, auth_tag_size = 0;

	if (!cipher_mode)
		iv_tag_size = 0;
	else if (!strcmp(cipher_mode, "xts-random"))
		iv_tag_size = 16;
	else if (!strcmp(cipher_mode, "gcm-random"))
		iv_tag_size = 12;
	else if (!strcmp(cipher_mode, "ccm-random"))
		iv_tag_size = 8;
	else if (!strcmp(cipher_mode, "ctr-random"))
		iv_tag_size = 16;
	else if (!strcmp(cipher, "aegis256") && !strcmp(cipher_mode, "random"))
		iv_tag_size = 32;
	else if (!strcmp(cipher_mode, "random"))
		iv_tag_size = 16;

	//FIXME: use crypto backend hash size
	if (!integrity || !strcmp(integrity, "none"))
		auth_tag_size = 0;
	else if (!strcmp(integrity, "aead"))
		auth_tag_size = 16; /* gcm- mode only */
	else if (!strcmp(integrity, "cmac(aes)"))
		auth_tag_size = 16;
	else if (!strcmp(integrity, "hmac(sha1)"))
		auth_tag_size = 20;
	else if (!strcmp(integrity, "hmac(sha256)"))
		auth_tag_size = 32;
	else if (!strcmp(integrity, "hmac(sha512)"))
		auth_tag_size = 64;
	else if (!strcmp(integrity, "poly1305")) {
		if (iv_tag_size)
			iv_tag_size = 12;
		auth_tag_size = 16;
	}

	return iv_tag_size + auth_tag_size;
}

int INTEGRITY_hash_tag_size(const char *integrity){ abort(); }

int INTEGRITY_format(struct crypt_device *cd,
		     const struct crypt_params_integrity *params,
		     struct volume_key *journal_crypt_key,
		     struct volume_key *journal_mac_key) { abort(); }

int INTEGRITY_activate(struct crypt_device *cd,
		       const char *name,
		       const struct crypt_params_integrity *params,
		       struct volume_key *vk,
		       struct volume_key *journal_crypt_key,
		       struct volume_key *journal_mac_key,
		       uint32_t flags, uint32_t sb_flags) { abort(); }

int INTEGRITY_create_dmd_device(struct crypt_device *cd,
		       const struct crypt_params_integrity *params,
		       struct volume_key *vk,
		       struct volume_key *journal_crypt_key,
		       struct volume_key *journal_mac_key,
		       struct crypt_dm_active_device *dmd,
		       uint32_t flags, uint32_t sb_flags) { abort(); }

int INTEGRITY_activate_dmd_device(struct crypt_device *cd,
		       const char *name,
		       const char *type,
		       struct crypt_dm_active_device *dmd,
		       uint32_t sb_flags) { abort(); }

