#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "mlx_ext.h"
#include "mlx_int.h"
#include "font8x8_basic.h"
#include <math.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>

/* Forward declarations of core MiniLibX functions used in these extensions */
int		mlx_hook(void *win_ptr, int x_event, int x_mask, int (*funct)(), void *param);
int		mlx_mouse_get_pos(void *mlx_ptr, void *win_ptr, int *x, int *y);
int		mlx_mouse_move(void *mlx_ptr, void *win_ptr, int x, int y);

/* 
** Writes a pixel to the image buffer at (x, y) with color conversion 
*/
void	mlx_img_put_pixel(void *img_ptr, int x, int y, int color)
{
	t_img	*img;
	char	*dst;
	int		bytes_per_pixel;

	img = (t_img *)img_ptr;
	if (!img || x < 0 || x >= img->width || y < 0 || y >= img->height)
		return ;
	bytes_per_pixel = img->bpp / 8;
	dst = img->data + (y * img->size_line + x * bytes_per_pixel);
	if (bytes_per_pixel == 4)
	{
		*(unsigned int *)dst = color;
	}
	else if (bytes_per_pixel == 3)
	{
		dst[0] = color & 0xFF;
		dst[1] = (color >> 8) & 0xFF;
		dst[2] = (color >> 16) & 0xFF;
	}
	else if (bytes_per_pixel == 2)
	{
		*(unsigned short *)dst = (unsigned short)color;
	}
}

/* 
** Reads a pixel color from the image buffer at (x, y) 
*/
unsigned int	mlx_img_get_pixel(void *img_ptr, int x, int y)
{
	t_img	*img;
	char	*src;
	int		bytes_per_pixel;

	img = (t_img *)img_ptr;
	if (!img || x < 0 || x >= img->width || y < 0 || y >= img->height)
		return (0);
	bytes_per_pixel = img->bpp / 8;
	src = img->data + (y * img->size_line + x * bytes_per_pixel);
	if (bytes_per_pixel == 4)
		return (*(unsigned int *)src);
	else if (bytes_per_pixel == 3)
		return ((unsigned char)src[0] | ((unsigned char)src[1] << 8) | ((unsigned char)src[2] << 16));
	else if (bytes_per_pixel == 2)
		return (*(unsigned short *)src);
	return (0);
}

/* 
** Bresenham's line algorithm to draw a line on an image buffer 
*/
void	mlx_img_draw_line(void *img_ptr, int x1, int y1, int x2, int y2, int color)
{
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy;
	int e2;

	while (1)
	{
		mlx_img_put_pixel(img_ptr, x1, y1, color);
		if (x1 == x2 && y1 == y2)
			break;
		e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y1 += sy;
		}
	}
}

/* 
** Draws a rectangle on an image buffer 
*/
void	mlx_img_draw_rect(void *img_ptr, int x, int y, int w, int h, int color, int fill)
{
	int i, j;

	if (fill)
	{
		for (j = 0; j < h; j++)
		{
			for (i = 0; i < w; i++)
			{
				mlx_img_put_pixel(img_ptr, x + i, y + j, color);
			}
		}
	}
	else
	{
		for (i = 0; i < w; i++)
		{
			mlx_img_put_pixel(img_ptr, x + i, y, color);
			mlx_img_put_pixel(img_ptr, x + i, y + h - 1, color);
		}
		for (j = 0; j < h; j++)
		{
			mlx_img_put_pixel(img_ptr, x, y + j, color);
			mlx_img_put_pixel(img_ptr, x + w - 1, y + j, color);
		}
	}
}

/* 
** Midpoint circle algorithm to draw a circle on an image buffer 
*/
void	mlx_img_draw_circle(void *img_ptr, int cx, int cy, int r, int color, int fill)
{
	int x = r;
	int y = 0;
	int err = 0;
	int i;

	if (fill)
	{
		while (x >= y)
		{
			for (i = cx - x; i <= cx + x; i++)
			{
				mlx_img_put_pixel(img_ptr, i, cy + y, color);
				mlx_img_put_pixel(img_ptr, i, cy - y, color);
			}
			for (i = cx - y; i <= cx + y; i++)
			{
				mlx_img_put_pixel(img_ptr, i, cy + x, color);
				mlx_img_put_pixel(img_ptr, i, cy - x, color);
			}

			if (err <= 0)
			{
				y += 1;
				err += 2 * y + 1;
			}
			if (err > 0)
			{
				x -= 1;
				err -= 2 * x + 1;
			}
		}
	}
	else
	{
		while (x >= y)
		{
			mlx_img_put_pixel(img_ptr, cx + x, cy + y, color);
			mlx_img_put_pixel(img_ptr, cx + y, cy + x, color);
			mlx_img_put_pixel(img_ptr, cx - y, cy + x, color);
			mlx_img_put_pixel(img_ptr, cx - x, cy + y, color);
			mlx_img_put_pixel(img_ptr, cx - x, cy - y, color);
			mlx_img_put_pixel(img_ptr, cx - y, cy - x, color);
			mlx_img_put_pixel(img_ptr, cx + y, cy - x, color);
			mlx_img_put_pixel(img_ptr, cx + x, cy - y, color);

			if (err <= 0)
			{
				y += 1;
				err += 2 * y + 1;
			}
			if (err > 0)
			{
				x -= 1;
				err -= 2 * x + 1;
			}
		}
	}
}

/* 
** Alpha Blending: blends src_img onto dst_img at (x_offset, y_offset) 
*/
void	mlx_blend_image_to_image(void *src_img, void *dst_img, int x_offset, int y_offset, float opacity)
{
	t_img	*src = (t_img *)src_img;
	t_img	*dst = (t_img *)dst_img;
	int		x, y;

	if (!src || !dst)
		return ;

	for (y = 0; y < src->height; y++)
	{
		int dst_y = y_offset + y;
		if (dst_y < 0 || dst_y >= dst->height)
			continue ;

		for (x = 0; x < src->width; x++)
		{
			int dst_x = x_offset + x;
			if (dst_x < 0 || dst_x >= dst->width)
				continue ;

			unsigned int src_color = mlx_img_get_pixel(src, x, y);
			int src_alpha = (src_color >> 24) & 0xFF;
			
			float blended_alpha = (src_alpha / 255.0f) * opacity;
			if (blended_alpha <= 0.0f)
				continue ;

			if (blended_alpha >= 1.0f)
			{
				mlx_img_put_pixel(dst, dst_x, dst_y, src_color);
			}
			else
			{
				unsigned int dst_color = mlx_img_get_pixel(dst, dst_x, dst_y);
				int src_r = (src_color >> 16) & 0xFF;
				int src_g = (src_color >> 8) & 0xFF;
				int src_b = src_color & 0xFF;

				int dst_r = (dst_color >> 16) & 0xFF;
				int dst_g = (dst_color >> 8) & 0xFF;
				int dst_b = dst_color & 0xFF;

				int out_r = (int)(src_r * blended_alpha + dst_r * (1.0f - blended_alpha));
				int out_g = (int)(src_g * blended_alpha + dst_g * (1.0f - blended_alpha));
				int out_b = (int)(src_b * blended_alpha + dst_b * (1.0f - blended_alpha));

				int dst_alpha = (dst_color >> 24) & 0xFF;
				int out_alpha = (int)(blended_alpha * 255.0f + dst_alpha * (1.0f - blended_alpha));

				unsigned int blended_color = (out_alpha << 24) | (out_r << 16) | (out_g << 8) | out_b;
				mlx_img_put_pixel(dst, dst_x, dst_y, blended_color);
			}
		}
	}
}

/* 
** PNG/JPEG Loader using stb_image.h 
*/
void	*mlx_png_file_to_image(void *mlx_ptr, char *filename, int *width, int *height)
{
	int				channels;
	unsigned char	*pixels;
	void			*img_ptr;
	int				x, y;

	pixels = stbi_load(filename, width, height, &channels, 4);
	if (!pixels)
		return (NULL);

	img_ptr = mlx_new_image(mlx_ptr, *width, *height);
	if (!img_ptr)
	{
		stbi_image_free(pixels);
		return (NULL);
	}

	for (y = 0; y < *height; y++)
	{
		for (x = 0; x < *width; x++)
		{
			int idx = (y * (*width) + x) * 4;
			unsigned char r = pixels[idx];
			unsigned char g = pixels[idx + 1];
			unsigned char b = pixels[idx + 2];
			unsigned char a = pixels[idx + 3];

			unsigned int color = (a << 24) | (r << 16) | (g << 8) | b;
			mlx_img_put_pixel(img_ptr, x, y, color);
		}
	}

	stbi_image_free(pixels);
	return (img_ptr);
}

/* 
** Image Property Getters 
*/
int	mlx_get_image_width(void *img_ptr)
{
	t_img	*img = (t_img *)img_ptr;
	return (img ? img->width : 0);
}

int	mlx_get_image_height(void *img_ptr)
{
	t_img	*img = (t_img *)img_ptr;
	return (img ? img->height : 0);
}

int	mlx_get_image_bpp(void *img_ptr)
{
	t_img	*img = (t_img *)img_ptr;
	return (img ? img->bpp : 0);
}

/* 
** Sub-image / Sprite Sheet Extraction 
*/
void	*mlx_get_sub_image(void *mlx_ptr, void *src_img, int x, int y, int w, int h)
{
	void	*dst_img;
	int		dx, dy;

	if (!src_img || w <= 0 || h <= 0)
		return (NULL);
	dst_img = mlx_new_image(mlx_ptr, w, h);
	if (!dst_img)
		return (NULL);
	for (dy = 0; dy < h; dy++)
	{
		for (dx = 0; dx < w; dx++)
		{
			unsigned int color = mlx_img_get_pixel(src_img, x + dx, y + dy);
			mlx_img_put_pixel(dst_img, dx, dy, color);
		}
	}
	return (dst_img);
}

/* 
** Fast Image Clearing 
*/
void	mlx_img_clear(void *img_ptr, int color)
{
	t_img	*img = (t_img *)img_ptr;
	int		x, y;

	if (!img)
		return ;
	if (img->bpp == 32 && img->size_line == img->width * 4)
	{
		int total_pixels = img->width * img->height;
		unsigned int *pixels = (unsigned int *)img->data;
		for (x = 0; x < total_pixels; x++)
			pixels[x] = color;
	}
	else
	{
		for (y = 0; y < img->height; y++)
		{
			for (x = 0; x < img->width; x++)
			{
				mlx_img_put_pixel(img_ptr, x, y, color);
			}
		}
	}
}

/* 
** Color Utilities 
*/
int	mlx_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return ((a << 24) | (r << 16) | (g << 8) | b);
}

unsigned char	mlx_get_r(int color)
{
	return ((color >> 16) & 0xFF);
}

unsigned char	mlx_get_g(int color)
{
	return ((color >> 8) & 0xFF);
}

unsigned char	mlx_get_b(int color)
{
	return (color & 0xFF);
}

unsigned char	mlx_get_a(int color)
{
	return ((color >> 24) & 0xFF);
}

/* 
** Draw Text Directly on Image Buffer (utilizing basic 8x8 font)
*/
void	mlx_img_string_put(void *img_ptr, int x, int y, int color, char *str)
{
	int		i;
	int		row, col;
	char	c;

	if (!img_ptr || !str)
		return ;
	for (i = 0; str[i] != '\0'; i++)
	{
		c = str[i];
		if (c < 0 || c >= 128)
			c = '?';
		for (row = 0; row < 8; row++)
		{
			char row_byte = font8x8_basic[(unsigned char)c][row];
			for (col = 0; col < 8; col++)
			{
				if (row_byte & (1 << col))
				{
					mlx_img_put_pixel(img_ptr, x + (i * 8) + col, y + row, color);
				}
			}
		}
	}
}

void	mlx_img_string_put_scaled(void *img_ptr, int x, int y, int color, char *str, int scale)
{
	int		i;
	int		row, col;
	int		dx, dy;
	char	c;

	if (!img_ptr || !str)
		return ;
	if (scale <= 1)
	{
		mlx_img_string_put(img_ptr, x, y, color, str);
		return ;
	}
	for (i = 0; str[i] != '\0'; i++)
	{
		c = str[i];
		if (c < 0 || c >= 128)
			c = '?';
		for (row = 0; row < 8; row++)
		{
			char row_byte = font8x8_basic[(unsigned char)c][row];
			for (col = 0; col < 8; col++)
			{
				if (row_byte & (1 << col))
				{
					for (dy = 0; dy < scale; dy++)
					{
						for (dx = 0; dx < scale; dx++)
						{
							mlx_img_put_pixel(img_ptr, x + (i * 8 * scale) + (col * scale) + dx, y + (row * scale) + dy, color);
						}
					}
				}
			}
		}
	}
}

/* 
** Keyboard State Tracking 
*/
static char g_key_states[65536] = {0};

static int	mlx_ext_key_press_hook(int keycode, void *param)
{
	if (keycode >= 0 && keycode < 65536)
		g_key_states[keycode] = 1;
	return (0);
}

static int	mlx_ext_key_release_hook(int keycode, void *param)
{
	if (keycode >= 0 && keycode < 65536)
		g_key_states[keycode] = 0;
	return (0);
}

void	mlx_ext_init_keyboard(void *win_ptr)
{
	mlx_hook(win_ptr, 2, 1L << 0, mlx_ext_key_press_hook, NULL);
	mlx_hook(win_ptr, 3, 1L << 1, mlx_ext_key_release_hook, NULL);
}

int	mlx_ext_is_key_down(int keycode)
{
	if (keycode >= 0 && keycode < 65536)
		return (g_key_states[keycode]);
	return (0);
}

/* 
** Scroll Wheel Detection 
*/
int	mlx_ext_is_scroll_up(int button)
{
	return (button == 4);
}

int	mlx_ext_is_scroll_down(int button)
{
	return (button == 5);
}

/* 
** Mouse Look Delta (centers pointer dynamically and returns pixel offsets) 
*/
void	mlx_ext_get_mouse_delta(void *mlx_ptr, void *win_ptr, int *dx, int *dy)
{
	t_xvar			*xvar;
	t_win_list		*win;
	int				mx;
	int				my;
	Window			root;
	int				rx;
	int				ry;
	unsigned int	w;
	unsigned int	h;
	unsigned int	border;
	unsigned int	depth;
	int				center_x;
	int				center_y;

	xvar = (t_xvar *)mlx_ptr;
	win = (t_win_list *)win_ptr;
	if (!xvar || !win)
		return ;
	XGetGeometry(xvar->display, win->window, &root, &rx, &ry, &w, &h, &border, &depth);
	center_x = (int)(w / 2);
	center_y = (int)(h / 2);
	mlx_mouse_get_pos(mlx_ptr, win_ptr, &mx, &my);
	if (dx)
		*dx = mx - center_x;
	if (dy)
		*dy = my - center_y;
	mlx_mouse_move(mlx_ptr, win_ptr, center_x, center_y);
}

/* 
** Image Transformations (Scaling and Rotation) 
*/
void	*mlx_scale_image(void *mlx_ptr, void *src_img, int new_width, int new_height)
{
	t_img	*src;
	void	*dst;
	int		x;
	int		y;
	int		src_x;
	int		src_y;
	unsigned int color;

	src = (t_img *)src_img;
	if (!src || new_width <= 0 || new_height <= 0)
		return (NULL);
	dst = mlx_new_image(mlx_ptr, new_width, new_height);
	if (!dst)
		return (NULL);
	for (y = 0; y < new_height; y++)
	{
		src_y = y * src->height / new_height;
		for (x = 0; x < new_width; x++)
		{
			src_x = x * src->width / new_width;
			color = mlx_img_get_pixel(src, src_x, src_y);
			mlx_img_put_pixel(dst, x, y, color);
		}
	}
	return (dst);
}

void	*mlx_rotate_image(void *mlx_ptr, void *src_img, double angle_rad)
{
	t_img	*src;
	void	*dst;
	int		w;
	int		h;
	double	cos_a;
	double	sin_a;
	double	cx;
	double	cy;
	int		x;
	int		y;
	double	dx;
	double	dy;
	int		sx;
	int		sy;
	unsigned int color;

	src = (t_img *)src_img;
	if (!src)
		return (NULL);
	w = src->width;
	h = src->height;
	dst = mlx_new_image(mlx_ptr, w, h);
	if (!dst)
		return (NULL);
	mlx_img_clear(dst, 0x00000000); // clear to transparent
	cos_a = cos(angle_rad);
	sin_a = sin(angle_rad);
	cx = w / 2.0;
	cy = h / 2.0;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			dx = x - cx;
			dy = y - cy;
			sx = (int)(dx * cos_a + dy * sin_a + cx);
			sy = (int)(-dx * sin_a + dy * cos_a + cy);
			if (sx >= 0 && sx < w && sy >= 0 && sy < h)
			{
				color = mlx_img_get_pixel(src, sx, sy);
				mlx_img_put_pixel(dst, x, y, color);
			}
		}
	}
	return (dst);
}


#if defined(__x86_64__)

static void	*make_thunk_1arg(void *self, void *impl)
{
	uint8_t	code[] = {
		0x48, 0x89, 0xFE,                          /* mov rsi, rdi        */
		0x48, 0xBF, 0,0,0,0, 0,0,0,0,             /* movabs rdi, <self>  */
		0x48, 0xB8, 0,0,0,0, 0,0,0,0,             /* movabs rax, <impl>  */
		0xFF, 0xE0                                 /* jmp rax             */
	};
	void	*thunk;

	memcpy(&code[5], &self, 8);
	memcpy(&code[15], &impl, 8);
	thunk = mmap(NULL, sizeof(code),
	             PROT_READ | PROT_WRITE | PROT_EXEC,
	             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (thunk == MAP_FAILED)
		return (NULL);
	memcpy(thunk, code, sizeof(code));
	__builtin___clear_cache(thunk, (char *)thunk + sizeof(code));
	return (thunk);
}

static void	*make_thunk_0arg(void *self, void *impl)
{
	uint8_t	code[] = {
		0x48, 0xBF, 0,0,0,0, 0,0,0,0,             /* movabs rdi, <self>  */
		0x48, 0xB8, 0,0,0,0, 0,0,0,0,             /* movabs rax, <impl>  */
		0xFF, 0xE0                                 /* jmp rax             */
	};
	void	*thunk;

	memcpy(&code[2], &self, 8);
	memcpy(&code[12], &impl, 8);
	thunk = mmap(NULL, sizeof(code),
	             PROT_READ | PROT_WRITE | PROT_EXEC,
	             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (thunk == MAP_FAILED)
		return (NULL);
	memcpy(thunk, code, sizeof(code));
	__builtin___clear_cache(thunk, (char *)thunk + sizeof(code));
	return (thunk);
}

static void	*make_thunk_3arg(void *self, void *impl)
{
	uint8_t	code[] = {
		0x48, 0x89, 0xD1,                          /* mov rcx, rdx        */
		0x48, 0x89, 0xF2,                          /* mov rdx, rsi        */
		0x48, 0x89, 0xFE,                          /* mov rsi, rdi        */
		0x48, 0xBF, 0,0,0,0, 0,0,0,0,             /* movabs rdi, <self>  */
		0x48, 0xB8, 0,0,0,0, 0,0,0,0,             /* movabs rax, <impl>  */
		0xFF, 0xE0                                 /* jmp rax             */
	};
	void	*thunk;

	memcpy(&code[11], &self, 8);
	memcpy(&code[21], &impl, 8);
	thunk = mmap(NULL, sizeof(code),
	             PROT_READ | PROT_WRITE | PROT_EXEC,
	             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (thunk == MAP_FAILED)
		return (NULL);
	memcpy(thunk, code, sizeof(code));
	__builtin___clear_cache(thunk, (char *)thunk + sizeof(code));
	return (thunk);
}

static void	*make_thunk_5arg(void *self, void *impl)
{
	uint8_t	code[] = {
		0x4D, 0x89, 0xC1,                          /* mov r9, r8          */
		0x49, 0x89, 0xC8,                          /* mov r8, rcx         */
		0x48, 0x89, 0xD1,                          /* mov rcx, rdx        */
		0x48, 0x89, 0xF2,                          /* mov rdx, rsi        */
		0x48, 0x89, 0xFE,                          /* mov rsi, rdi        */
		0x48, 0xBF, 0,0,0,0, 0,0,0,0,             /* movabs rdi, <self>  */
		0x48, 0xB8, 0,0,0,0, 0,0,0,0,             /* movabs rax, <impl>  */
		0xFF, 0xE0                                 /* jmp rax             */
	};
	void	*thunk;

	memcpy(&code[17], &self, 8);
	memcpy(&code[27], &impl, 8);
	thunk = mmap(NULL, sizeof(code),
	             PROT_READ | PROT_WRITE | PROT_EXEC,
	             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (thunk == MAP_FAILED)
		return (NULL);
	memcpy(thunk, code, sizeof(code));
	__builtin___clear_cache(thunk, (char *)thunk + sizeof(code));
	return (thunk);
}

#elif defined(__aarch64__)

static void	*make_thunk_1arg(void *self, void *impl)
{
	uint32_t	code[] = {
		0xaa0003e1,                              /* mov x1, x0          */
		0x58000060,                              /* ldr x0, [pc, #12]   */
		0x58000090,                              /* ldr x16, [pc, #16]  */
		0xd61f0200,                              /* br x16              */
		0, 0,                                    /* self literal        */
		0, 0                                     /* impl literal        */
	};
	void	*thunk;

	memcpy(&code[4], &self, 8);
	memcpy(&code[6], &impl, 8);
	thunk = mmap(NULL, sizeof(code),
	             PROT_READ | PROT_WRITE | PROT_EXEC,
	             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (thunk == MAP_FAILED)
		return (NULL);
	memcpy(thunk, code, sizeof(code));
	__builtin___clear_cache(thunk, (char *)thunk + sizeof(code));
	return (thunk);
}

static void	*make_thunk_0arg(void *self, void *impl)
{
	uint32_t	code[] = {
		0x58000040,                              /* ldr x0, [pc, #8]    */
		0x58000070,                              /* ldr x16, [pc, #12]  */
		0xd61f0200,                              /* br x16              */
		0, 0,                                    /* self literal        */
		0, 0                                     /* impl literal        */
	};
	void	*thunk;

	memcpy(&code[3], &self, 8);
	memcpy(&code[5], &impl, 8);
	thunk = mmap(NULL, sizeof(code),
	             PROT_READ | PROT_WRITE | PROT_EXEC,
	             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (thunk == MAP_FAILED)
		return (NULL);
	memcpy(thunk, code, sizeof(code));
	__builtin___clear_cache(thunk, (char *)thunk + sizeof(code));
	return (thunk);
}

static void	*make_thunk_3arg(void *self, void *impl)
{
	uint32_t	code[] = {
		0xaa0203e3,                              /* mov x3, x2          */
		0xaa0103e2,                              /* mov x2, x1          */
		0xaa0003e1,                              /* mov x1, x0          */
		0x58000060,                              /* ldr x0, [pc, #12]   */
		0x58000090,                              /* ldr x16, [pc, #16]  */
		0xd61f0200,                              /* br x16              */
		0, 0,                                    /* self literal        */
		0, 0                                     /* impl literal        */
	};
	void	*thunk;

	memcpy(&code[6], &self, 8);
	memcpy(&code[8], &impl, 8);
	thunk = mmap(NULL, sizeof(code),
	             PROT_READ | PROT_WRITE | PROT_EXEC,
	             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (thunk == MAP_FAILED)
		return (NULL);
	memcpy(thunk, code, sizeof(code));
	__builtin___clear_cache(thunk, (char *)thunk + sizeof(code));
	return (thunk);
}

static void	*make_thunk_5arg(void *self, void *impl)
{
	uint32_t	code[] = {
		0xaa0403e5,                              /* mov x5, x4          */
		0xaa0303e4,                              /* mov x4, x3          */
		0xaa0203e3,                              /* mov x3, x2          */
		0xaa0103e2,                              /* mov x2, x1          */
		0xaa0003e1,                              /* mov x1, x0          */
		0x58000060,                              /* ldr x0, [pc, #12]   */
		0x58000090,                              /* ldr x16, [pc, #16]  */
		0xd61f0200,                              /* br x16              */
		0, 0,                                    /* self literal        */
		0, 0                                     /* impl literal        */
	};
	void	*thunk;

	memcpy(&code[8], &self, 8);
	memcpy(&code[10], &impl, 8);
	thunk = mmap(NULL, sizeof(code),
	             PROT_READ | PROT_WRITE | PROT_EXEC,
	             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (thunk == MAP_FAILED)
		return (NULL);
	memcpy(thunk, code, sizeof(code));
	__builtin___clear_cache(thunk, (char *)thunk + sizeof(code));
	return (thunk);
}

#else

# error "Dynamic ASM thunks are not supported on this architecture!"

#endif

/* 
** Scene Graph Node System (Componentization)
*/

t_mlx_node	*mlx_node_create(int type, int x, int y, int w, int h)
{
	t_mlx_node	*node;

	node = malloc(sizeof(*node));
	if (!node)
		return (NULL);
	node->type = type;
	node->x = x;
	node->y = y;
	node->w = w;
	node->h = h;
	node->color = 0xFFFFFF;
	node->fill = 1;
	node->img = NULL;
	node->text = NULL;
	node->visible = 1;
	node->enabled = 1;
	node->parent = NULL;
	node->next = NULL;
	node->children = NULL;
	node->on_update = NULL;
	node->on_click = NULL;
	node->user_data = NULL;
	
	/* Bind dynamic ASM thunks */
	node->add_child = make_thunk_1arg(node, mlx_node_add_child);
	node->remove_child = make_thunk_1arg(node, mlx_node_remove_child);
	node->destroy = make_thunk_0arg(node, mlx_node_destroy);
	node->print_help = make_thunk_0arg(node, mlx_node_print_help);
	node->update = make_thunk_0arg(node, mlx_node_update);
	node->render = make_thunk_3arg(node, mlx_node_render);
	node->handle_click = make_thunk_5arg(node, mlx_node_handle_click);

	if (!node->add_child || !node->remove_child || !node->destroy || !node->print_help
		|| !node->update || !node->render || !node->handle_click)
	{
		if (node->add_child) munmap((void *)node->add_child, 64);
		if (node->remove_child) munmap((void *)node->remove_child, 64);
		if (node->destroy) munmap((void *)node->destroy, 64);
		if (node->print_help) munmap((void *)node->print_help, 64);
		if (node->update) munmap((void *)node->update, 64);
		if (node->render) munmap((void *)node->render, 64);
		if (node->handle_click) munmap((void *)node->handle_click, 64);
		free(node);
		return (NULL);
	}

	return (node);
}

void	mlx_node_destroy(t_mlx_node *node)
{
	t_mlx_node	*curr;
	t_mlx_node	*next;

	if (!node)
		return ;
	curr = node->children;
	while (curr)
	{
		next = curr->next;
		mlx_node_destroy(curr);
		curr = next;
	}
	if (node->text)
		free(node->text);

	/* Unmap executable thunk pages */
	if (node->add_child)
		munmap((void *)node->add_child, 64);
	if (node->remove_child)
		munmap((void *)node->remove_child, 64);
	if (node->destroy)
		munmap((void *)node->destroy, 64);
	if (node->print_help)
		munmap((void *)node->print_help, 64);
	if (node->update)
		munmap((void *)node->update, 64);
	if (node->render)
		munmap((void *)node->render, 64);
	if (node->handle_click)
		munmap((void *)node->handle_click, 64);

	free(node);
}

void	mlx_node_add_child(t_mlx_node *parent, t_mlx_node *child)
{
	t_mlx_node	*curr;

	if (!parent || !child)
		return ;
	child->parent = parent;
	child->next = NULL;
	if (!parent->children)
	{
		parent->children = child;
	}
	else
	{
		curr = parent->children;
		while (curr->next)
			curr = curr->next;
		curr->next = child;
	}
}

void	mlx_node_remove_child(t_mlx_node *parent, t_mlx_node *child)
{
	t_mlx_node	*curr;
	t_mlx_node	*prev;

	if (!parent || !child)
		return ;
	curr = parent->children;
	prev = NULL;
	while (curr)
	{
		if (curr == child)
		{
			if (prev)
				prev->next = curr->next;
			else
				parent->children = curr->next;
			child->parent = NULL;
			child->next = NULL;
			break ;
		}
		prev = curr;
		curr = curr->next;
	}
}

t_mlx_node	*mlx_node_create_sprite(void *img, int x, int y)
{
	t_mlx_node	*node;
	int			w = mlx_get_image_width(img);
	int			h = mlx_get_image_height(img);

	node = mlx_node_create(NODE_TYPE_SPRITE, x, y, w, h);
	if (node)
		node->img = img;
	return (node);
}

t_mlx_node	*mlx_node_create_rect(int x, int y, int w, int h, unsigned int color, int fill)
{
	t_mlx_node	*node;

	node = mlx_node_create(NODE_TYPE_RECT, x, y, w, h);
	if (node)
	{
		node->color = color;
		node->fill = fill;
	}
	return (node);
}

t_mlx_node	*mlx_node_create_button(char *text, int x, int y, int w, int h, void (*on_click)(t_mlx_node *, int, int, int))
{
	t_mlx_node	*node;

	node = mlx_node_create(NODE_TYPE_BUTTON, x, y, w, h);
	if (node)
	{
		node->text = text ? strdup(text) : NULL;
		node->on_click = (void *)on_click;
		node->color = 0x444454;
	}
	return (node);
}

t_mlx_node	*mlx_node_create_text(char *text, int x, int y, unsigned int color)
{
	t_mlx_node	*node;

	node = mlx_node_create(NODE_TYPE_TEXT, x, y, 0, 0);
	if (node)
	{
		node->text = text ? strdup(text) : NULL;
		node->color = color;
	}
	return (node);
}

void	mlx_node_update(t_mlx_node *root)
{
	t_mlx_node	*curr;

	if (!root || !root->enabled)
		return ;
	if (root->on_update)
		root->on_update(root);
	curr = root->children;
	while (curr)
	{
		mlx_node_update(curr);
		curr = curr->next;
	}
}

void	mlx_node_render(t_mlx_node *root, void *canvas_img, int parent_abs_x, int parent_abs_y)
{
	t_mlx_node	*curr;
	int			abs_x;
	int			abs_y;

	if (!root || !root->visible)
		return ;
	abs_x = parent_abs_x + root->x;
	abs_y = parent_abs_y + root->y;

	if (root->type == NODE_TYPE_RECT)
	{
		mlx_img_draw_rect(canvas_img, abs_x, abs_y, root->w, root->h, root->color, root->fill);
	}
	else if (root->type == NODE_TYPE_SPRITE && root->img)
	{
		mlx_blend_image_to_image(root->img, canvas_img, abs_x, abs_y, 1.0f);
	}
	else if (root->type == NODE_TYPE_BUTTON)
	{
		mlx_img_draw_rect(canvas_img, abs_x, abs_y, root->w, root->h, root->color, 1);
		mlx_img_draw_rect(canvas_img, abs_x, abs_y, root->w, root->h, 0x888898, 0);
		if (root->text)
		{
			int text_len = strlen(root->text);
			int tx = abs_x + (root->w - text_len * 8) / 2;
			int ty = abs_y + (root->h - 8) / 2;
			mlx_img_string_put(canvas_img, tx, ty, 0xFFFFFF, root->text);
		}
	}
	else if (root->type == NODE_TYPE_TEXT && root->text)
	{
		mlx_img_string_put(canvas_img, abs_x, abs_y, root->color, root->text);
	}

	curr = root->children;
	while (curr)
	{
		mlx_node_render(curr, canvas_img, abs_x, abs_y);
		curr = curr->next;
	}
}

int	mlx_node_handle_click(t_mlx_node *root, int button, int x, int y, int parent_abs_x, int parent_abs_y)
{
	t_mlx_node	*curr;
	int			abs_x;
	int			abs_y;

	if (!root || !root->visible || !root->enabled)
		return (0);
	abs_x = parent_abs_x + root->x;
	abs_y = parent_abs_y + root->y;

	curr = root->children;
	while (curr)
	{
		if (mlx_node_handle_click(curr, button, x, y, abs_x, abs_y))
			return (1);
		curr = curr->next;
	}

	if (x >= abs_x && x < abs_x + root->w && y >= abs_y && y < abs_y + root->h)
	{
		if (root->on_click)
		{
			root->on_click(root, button, x - abs_x, y - abs_y);
			return (1);
		}
	}
	return (0);
}

/* 
** Help & Documentation Utilities 
*/

static void	print_header(char *name)
{
	printf("\n================================================================================\n");
	printf(" HELP & USE: %s\n", name);
	printf("================================================================================\n");
}

void	mlx_ext_print_help(char *name)
{
	if (!name)
	{
		printf("Error: Function name or component name is NULL.\n");
		return ;
	}
	if (strcmp(name, "mlx_png_file_to_image") == 0)
	{
		print_header(name);
		printf("Prototype:   void *mlx_png_file_to_image(void *mlx_ptr, char *filename, int *width, int *height)\n\n");
		printf("Description: Loads a PNG or JPEG file into a MiniLibX image structure using stb_image.h.\n\n");
		printf("Arguments:\n");
		printf("  - mlx_ptr : Pointer returned by mlx_init().\n");
		printf("  - filename: Path to the PNG/JPEG image file.\n");
		printf("  - width   : Pointer to int to store the loaded image width.\n");
		printf("  - height  : Pointer to int to store the loaded image height.\n\n");
		printf("Returns:     A pointer to the new image structure (void *), or NULL if loading failed.\n");
	}
	else if (strcmp(name, "mlx_scale_image") == 0)
	{
		print_header(name);
		printf("Prototype:   void *mlx_scale_image(void *mlx_ptr, void *src_img, int new_width, int new_height)\n\n");
		printf("Description: Creates a new resized image from src_img using nearest-neighbor interpolation.\n\n");
		printf("Arguments:\n");
		printf("  - mlx_ptr   : Pointer returned by mlx_init().\n");
		printf("  - src_img   : Pointer to the source image structure.\n");
		printf("  - new_width : The desired width of the new scaled image.\n");
		printf("  - new_height: The desired height of the new scaled image.\n\n");
		printf("Returns:     A pointer to the new scaled image (void *), or NULL if failed.\n");
	}
	else if (strcmp(name, "mlx_rotate_image") == 0)
	{
		print_header(name);
		printf("Prototype:   void *mlx_rotate_image(void *mlx_ptr, void *src_img, double angle_rad)\n\n");
		printf("Description: Rotates src_img around its center and returns a new rotated image of the same size.\n");
		printf("             Uses backward mapping to prevent gaps/moiré patterns.\n\n");
		printf("Arguments:\n");
		printf("  - mlx_ptr  : Pointer returned by mlx_init().\n");
		printf("  - src_img  : Pointer to the source image.\n");
		printf("  - angle_rad: Rotation angle in radians (e.g. 3.14159/4 for 45 degrees).\n\n");
		printf("Returns:     A pointer to the new rotated image (void *), or NULL if failed.\n");
	}
	else if (strcmp(name, "mlx_blend_image_to_image") == 0)
	{
		print_header(name);
		printf("Prototype:   void mlx_blend_image_to_image(void *src_img, void *dst_img, int x_offset, int y_offset, float opacity)\n\n");
		printf("Description: Blends src_img onto dst_img at the given offsets using alpha-blending.\n");
		printf("             Combines both src pixel alpha and the global opacity factor.\n\n");
		printf("Arguments:\n");
		printf("  - src_img : Source image to read pixels from.\n");
		printf("  - dst_img : Destination image to draw pixels onto.\n");
		printf("  - x_offset: X coordinate on dst_img to start blending.\n");
		printf("  - y_offset: Y coordinate on dst_img to start blending.\n");
		printf("  - opacity : Global opacity multiplier (0.0 = transparent, 1.0 = opaque).\n");
	}
	else if (strcmp(name, "mlx_img_string_put_scaled") == 0)
	{
		print_header(name);
		printf("Prototype:   void mlx_img_string_put_scaled(void *img_ptr, int x, int y, int color, char *str, int scale)\n\n");
		printf("Description: Draws text onto an image buffer using an embedded 8x8 bitmap font.\n");
		printf("             Supports integer scaling (e.g. scale=2 draws 16x16 characters).\n\n");
		printf("Arguments:\n");
		printf("  - img_ptr: Pointer to the image structure.\n");
		printf("  - x, y   : Coordinates on the image.\n");
		printf("  - color  : Color integer (0xRRGGBB).\n");
		printf("  - str    : String to write.\n");
		printf("  - scale  : Scaled font multiplier (e.g., 1, 2, 3).\n");
	}
	else if (strcmp(name, "mlx_ext_get_mouse_delta") == 0)
	{
		print_header(name);
		printf("Prototype:   void mlx_ext_get_mouse_delta(void *mlx_ptr, void *win_ptr, int *dx, int *dy)\n\n");
		printf("Description: Gets mouse coordinate offset relative to the window center, and warps the mouse back.\n");
		printf("             Dynamically queries window geometry using XGetGeometry. Ideal for FPS cameras.\n\n");
		printf("Arguments:\n");
		printf("  - mlx_ptr: Pointer returned by mlx_init().\n");
		printf("  - win_ptr: Pointer to the window structure.\n");
		printf("  - dx     : Pointer to int to store the X movement offset.\n");
		printf("  - dy     : Pointer to int to store the Y movement offset.\n");
	}
	else if (strcmp(name, "t_mlx_node") == 0 || strcmp(name, "mlx_node_create") == 0)
	{
		print_header("t_mlx_node & Component UI System");
		printf("Description: Hierarchical Scene Graph element. Allows parent-child UI component trees.\n\n");
		printf("Node Types:\n");
		printf("  - NODE_TYPE_CONTAINER (0): Invisible panel container.\n");
		printf("  - NODE_TYPE_SPRITE (1)   : Node holding a drawable image texture.\n");
		printf("  - NODE_TYPE_RECT (2)     : Draws filled/outlined 2D rectangles.\n");
		printf("  - NODE_TYPE_BUTTON (3)   : Interactive button component with click callback.\n");
		printf("  - NODE_TYPE_TEXT (4)     : Label for text rendering inside image canvas.\n\n");
		printf("Constructors:\n");
		printf("  - t_mlx_node *mlx_node_create(int type, int x, int y, int w, int h)\n");
		printf("  - t_mlx_node *mlx_node_create_sprite(void *img, int x, int y)\n");
		printf("  - t_mlx_node *mlx_node_create_button(char *text, int x, int y, int w, int h, on_click_func)\n");
		printf("  - t_mlx_node *mlx_node_create_text(char *text, int x, int y, unsigned int color)\n\n");
		printf("Tree Operations:\n");
		printf("  - void mlx_node_add_child(t_mlx_node *parent, t_mlx_node *child)\n");
		printf("  - void mlx_node_destroy(t_mlx_node *root)  [Recursively frees entire tree]\n");
	}
	else
	{
		printf("Help: Unknown function or structure: '%s'. Try 't_mlx_node', 'mlx_scale_image', 'mlx_rotate_image', 'mlx_ext_get_mouse_delta', or 'mlx_png_file_to_image'.\n", name);
	}
	printf("================================================================================\n\n");
}

void	mlx_node_print_help(t_mlx_node *node)
{
	t_mlx_node	*curr;
	int			child_count;
	char		*type_name;

	if (!node)
	{
		printf("Node Inspector: Node pointer is NULL.\n");
		return ;
	}
	child_count = 0;
	curr = node->children;
	while (curr)
	{
		child_count++;
		curr = curr->next;
	}
	if (node->type == NODE_TYPE_CONTAINER)
		type_name = "CONTAINER";
	else if (node->type == NODE_TYPE_SPRITE)
		type_name = "SPRITE";
	else if (node->type == NODE_TYPE_RECT)
		type_name = "RECTANGLE";
	else if (node->type == NODE_TYPE_BUTTON)
		type_name = "BUTTON";
	else if (node->type == NODE_TYPE_TEXT)
		type_name = "TEXT_LABEL";
	else
		type_name = "UNKNOWN_NODE";

	printf("[Node Inspector]\n");
	printf("  Type:     %s (%d)\n", type_name, node->type);
	printf("  Position: x = %d, y = %d\n", node->x, node->y);
	printf("  Size:     w = %d, h = %d\n", node->w, node->h);
	printf("  Color:    0x%08X\n", node->color);
	printf("  Visible:  %s | Enabled: %s\n", node->visible ? "TRUE" : "FALSE", node->enabled ? "TRUE" : "FALSE");
	printf("  Text:     %s\n", node->text ? node->text : "(none)");
	printf("  Has Image:%s\n", node->img ? "YES" : "NO");
	printf("  Children: %d node(s)\n", child_count);
	if (child_count > 0)
	{
		printf("  Child List:\n");
		curr = node->children;
		while (curr)
		{
			printf("    - Type: %d, x = %d, y = %d, w = %d, h = %d\n", curr->type, curr->x, curr->y, curr->w, curr->h);
			curr = curr->next;
		}
	}
	printf("\n");
}



