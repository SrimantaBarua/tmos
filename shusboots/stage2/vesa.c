// (C) 2019 Srimanta Barua


#include <log.h>
#include <util.h>
#include <vesa.h>
#include <real.h>
#include <stddef.h>


// Internal state
static struct vbe_info      *_vbe_info = NULL;
static struct vbe_mode_info *_mode_info = NULL;
static uint16_t             _cur_mode = 0xffff;


extern void real_vesa_get_bios_info();
extern void real_vesa_get_mode_info();
extern void real_vesa_set_mode();


// Initialize VESA subsystem with a direct-color mode with given screen width, height
// and bits per pixel
// Returns pointer to vbe_mode_info struct on success, NULL on failure
struct vbe_mode_info* vesa_init(uint16_t width, uint16_t height, uint8_t bpp)  {
	// Get VESA BIOS info
	if (!(_vbe_info = (struct vbe_info*) real_call(real_vesa_get_bios_info, 0))) {
		log(LOG_ERR, "vesa_init: Could not get VESA BIOS info\n");
		return NULL;
	}
	// Set desired VESA mode (or a close approximate)
	return vesa_set_mode(width, height, bpp);
}


// Set the VESA mode
// Returns pointer to vbe_mode_info struct on success, NULL on failure
struct vbe_mode_info* vesa_set_mode(uint16_t width, uint16_t height, uint8_t bpp) {
	struct real_ptr rptr;
	uint16_t *ptr, best_mode = 0xffff;
	struct vbe_mode_info *mode_info;
	uint32_t pix_diff, best_pix_diff = DIFF(320 * 200, height * width);
	// Have we been initialized?
	if (!_vbe_info) {
		log(LOG_ERR, "vesa_set_mode: VESA not initialized yet\n");
		return NULL;
	}
	// Go over available modes
	rptr = _vbe_info->video_modes;
	for (ptr = (uint16_t *) real_ptr_to_linear(rptr); *ptr != 0xffff; ptr++) {
		// Get mode info
		if (!(mode_info = (struct vbe_mode_info*) real_call(real_vesa_get_mode_info, 1, *ptr))) {
			log(LOG_ERR, "vesa_set_mode: Could not get info for mode: %u\n", *ptr);
			return NULL;
		}
		// Does this support framebuffer?
		if (!(mode_info->attributes & 0x80)) {
			continue;
		}
		// Is this a direct color mode?
		if (mode_info->model != 6) {
			continue;
		}
		// Is BPP as required?
		if (mode_info->bpp != bpp) {
			continue;
		}
		// Compare  width, height for exact match
		if (mode_info->width == width && mode_info->height == height
			&& mode_info->bpp == bpp) {
			// Set mode
			if (!real_call(real_vesa_set_mode, 1, *ptr)) {
				log(LOG_ERR, "vesa_set_mode: Could not set mode: %u\n", *ptr);
				return NULL;
			}
			_cur_mode = *ptr;
			_mode_info = mode_info;
			return mode_info;
		}
		// Non-exact match. Look for best approximate
		pix_diff = DIFF(mode_info->width * mode_info->height, width * height);
		if (pix_diff < best_pix_diff) {
			best_pix_diff = pix_diff;
			best_mode = *ptr;
		}
	}
	if (best_mode == 0xffff) {
		return NULL;
	}
	// Get mode info again
	if (!(mode_info = (struct vbe_mode_info*) real_call(real_vesa_get_mode_info, 1, best_mode))) {
		log(LOG_ERR, "vesa_set_mode: Could not get info for best mode: %u\n", best_mode);
		return NULL;
	}
	// Set mode
	if (!real_call(real_vesa_set_mode,  1, best_mode)) {
		log(LOG_ERR, "vesa_set_mode: Could not set mode: %u\n", best_mode);
		return NULL;
	}
	_cur_mode = best_mode;
	_mode_info = mode_info;
	return mode_info;
}

