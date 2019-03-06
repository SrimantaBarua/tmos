// (C) 2019 Srimanta Barua


#include <log.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <boot_cfg.h>


#define PARSED_BOOT_CFG_ADDR 0x52000
#define PARSED_BOOT_CFG_SIZE 0x2000


// Skip spaces
#define _skip_space(s, l) \
	while (isspace(*(s))) { \
		if (*(s) == '\n') { \
			(l)++; \
		} \
		(s)++; \
	}


// Skip spaces except newline
#define _skip_space_nonl(s) \
	while (isspace(*(s))) { \
		if (*(s) == '\n') { \
			break; \
		} \
		(s)++; \
	}


// Parse bootloader config (null-terminated string). Returns pointer to parsed structure if valid,
// else return NULL
const struct boot_cfg* boot_cfg_parse(char *cfg) {
	uint32_t linum = 1;
	struct boot_cfg *ret = (struct boot_cfg*) PARSED_BOOT_CFG_ADDR;
	if (!cfg) {
		return NULL;
	}
	// Initialize boot_cfg struct
	memset(ret, 0, sizeof(struct boot_cfg));
	while (1) {
		_skip_space(cfg, linum);
		if (!*cfg) {
			break;
		}
		if (!strncmp(cfg, "video.", 6)) {
			cfg += 6;
			if (!strncmp(cfg, "width", 5)) {
				cfg += 5;
				_skip_space_nonl(cfg);
				if (*cfg++ != '=') {
					log(LOG_ERR, "boot_cfg_parse: Expected '=' on line %u\n",
						linum);
					return NULL;
				}
				_skip_space_nonl(cfg);
				if (!isdigit(*cfg)) {
					log(LOG_ERR, "boot_cfg_parse: Expected digit on line %u\n",
						linum);
					return NULL;
				}
				ret->video.width = (uint16_t) skip_atou((const char **) &cfg);
			} else if (!strncmp(cfg, "height", 6)) {
				cfg += 6;
				_skip_space_nonl(cfg);
				if (*cfg++ != '=') {
					log(LOG_ERR, "boot_cfg_parse: Expected '=' on line %u\n",
						linum);
					return NULL;
				}
				_skip_space_nonl(cfg);
				if (!isdigit(*cfg)) {
					log(LOG_ERR, "boot_cfg_parse: Expected digit on line %u\n",
						linum);
					return NULL;
				}
				ret->video.height = (uint16_t) skip_atou((const char **) &cfg);
			} else if (!strncmp(cfg, "bpp", 3)) {
				cfg += 3;
				_skip_space_nonl(cfg);
				if (*cfg++ != '=') {
					log(LOG_ERR, "boot_cfg_parse: Expected '=' on line %u\n",
						linum);
					return NULL;
				}
				_skip_space_nonl(cfg);
				if (!isdigit(*cfg)) {
					log(LOG_ERR, "boot_cfg_parse: Expected digit on line %u\n",
						linum);
					return NULL;
				}
				ret->video.bpp = (uint8_t) skip_atou((const char **) &cfg);
			} else {
				log(LOG_ERR, "boot_cfg_parse: Unrecognized item on line %u\n",
					linum);
				return NULL;
			}
			_skip_space_nonl(cfg);
			if (*cfg != '\n') {
				log(LOG_ERR, "boot_cfg_parse: Expected '\\n' at end of line %u\n",
					linum);
				return NULL;
			}
		} else {
			log(LOG_ERR, "boot_cfg_parse: Unrecognized item on line %u\n",
				linum);
			return NULL;
		}
	}
	return ret;
}

