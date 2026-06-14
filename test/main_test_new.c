#include "mlx.h"
#include "mlx_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

void	*mlx;
void	*win;
void	*canvas;
void	*sprite;

int		player_x = 400;
int		player_y = 300;
int		sprite_w = 0;
int		sprite_h = 0;
double	sprite_angle = 0.0;

int	close_window(void *param)
{
	(void)param;
	printf("Exiting test program.\n");
	if (sprite)
		mlx_destroy_image(mlx, sprite);
	if (canvas)
		mlx_destroy_image(mlx, canvas);
	if (win)
		mlx_destroy_window(mlx, win);
	mlx_destroy_display(mlx);
	exit(0);
	return (0);
}

int	mouse_hook(int button, int x, int y, void *param)
{
	(void)param;
	if (mlx_ext_is_scroll_up(button))
		printf("Scroll UP event detected at mouse position: %dx%d\n", x, y);
	else if (mlx_ext_is_scroll_down(button))
		printf("Scroll DOWN event detected at mouse position: %dx%d\n", x, y);
	else
		printf("Mouse button %d pressed at mouse position: %dx%d\n", button, x, y);
	return (0);
}

int	update_and_render(void *param)
{
	(void)param;
	/* 1. Keyboard State Polling */
	// ESC
	if (mlx_ext_is_key_down(65307) || mlx_ext_is_key_down(53) || mlx_ext_is_key_down(0xFF1B))
		close_window(NULL);

	// WASD / Arrow Keys Movement
	if (mlx_ext_is_key_down(119) || mlx_ext_is_key_down(65362)) // W / Up
		player_y -= 4;
	if (mlx_ext_is_key_down(115) || mlx_ext_is_key_down(65364)) // S / Down
		player_y += 4;
	if (mlx_ext_is_key_down(97) || mlx_ext_is_key_down(65361))  // A / Left
		player_x -= 4;
	if (mlx_ext_is_key_down(100) || mlx_ext_is_key_down(65363)) // D / Right
		player_x += 4;

	// Boundaries check
	if (player_x < 15) player_x = 15;
	if (player_x > WIN_WIDTH - 15) player_x = WIN_WIDTH - 15;
	if (player_y < 15) player_y = 15;
	if (player_y > WIN_HEIGHT - 15) player_y = WIN_HEIGHT - 15;

	/* 2. Render Canvas Buffer (Double Buffering) */
	mlx_img_clear(canvas, 0x1E1E2E); // Clear screen to dark slate gray

	// Grid background
	for (int i = 0; i < WIN_WIDTH; i += 40)
		mlx_img_draw_line(canvas, i, 0, i, WIN_HEIGHT, 0x252535);
	for (int j = 0; j < WIN_HEIGHT; j += 40)
		mlx_img_draw_line(canvas, 0, j, WIN_WIDTH, j, 0x252535);

	// Static test shapes
	mlx_img_draw_rect(canvas, 50, 50, 200, 100, 0xFF5555, 1);   // Filled Red Rect
	mlx_img_draw_rect(canvas, 80, 80, 200, 100, 0x55FF55, 0);   // Outlined Green Rect
	mlx_img_draw_circle(canvas, 500, 130, 80, 0x5555FF, 0);     // Outlined Blue Circle
	mlx_img_draw_circle(canvas, 500, 130, 50, 0xFFFF55, 1);     // Filled Yellow Circle
	mlx_img_draw_line(canvas, 50, 480, 250, 580, 0xFF55FF);     // Diagonal Magenta Line

	// Alpha Blended Sprites
	if (sprite)
	{
		// 100% opacity
		mlx_blend_image_to_image(sprite, canvas, 80, 250, 1.0f);
		mlx_img_string_put(canvas, 80, 235, 0xFFFFFF, "Opacity: 1.0");

		// 50% opacity
		mlx_blend_image_to_image(sprite, canvas, 260, 250, 0.5f);
		mlx_img_string_put(canvas, 260, 235, 0xFFFFFF, "Opacity: 0.5");

		// 20% opacity
		mlx_blend_image_to_image(sprite, canvas, 440, 250, 0.2f);
		mlx_img_string_put(canvas, 440, 235, 0xFFFFFF, "Opacity: 0.2");

		// Spritesheet / Sub-image extraction (extracting 64x64 middle tile of sprite)
		void *sub_sprite = mlx_get_sub_image(mlx, sprite, sprite_w/4, sprite_h/4, sprite_w/2, sprite_h/2);
		if (sub_sprite)
		{
			mlx_blend_image_to_image(sub_sprite, canvas, 620, 250, 1.0f);
			mlx_img_string_put(canvas, 620, 235, 0xFFFFFF, "Sub-Sprite (1/2)");
			mlx_destroy_image(mlx, sub_sprite);
		}
	}

	// Update sprite angle for real-time rotation
	sprite_angle += 0.03;
	if (sprite_angle > 6.28318)
		sprite_angle -= 6.28318;

	// Scale and Rotate Sprite test
	if (sprite)
	{
		// Real-time rotated sprite
		void *rot_sprite = mlx_rotate_image(mlx, sprite, sprite_angle);
		if (rot_sprite)
		{
			mlx_blend_image_to_image(rot_sprite, canvas, 80, 390, 1.0f);
			mlx_img_string_put(canvas, 80, 375, 0xFFFFFF, "Rotating Sprite");
			mlx_destroy_image(mlx, rot_sprite);
		}

		// Real-time scaled sprite (128x128 scaled down to 64x64)
		void *sc_sprite = mlx_scale_image(mlx, sprite, 64, 64);
		if (sc_sprite)
		{
			mlx_blend_image_to_image(sc_sprite, canvas, 260, 390, 1.0f);
			mlx_img_string_put(canvas, 260, 375, 0xFFFFFF, "Scaled down (64x64)");
			mlx_destroy_image(mlx, sc_sprite);
		}
	}

	// Test Scaled Text rendering
	mlx_img_string_put_scaled(canvas, 380, 380, 0xFF5555, "LARGE TEXT (scale 2)", 2);
	mlx_img_string_put_scaled(canvas, 380, 405, 0x55FFFF, "HUGE TEXT (scale 3)", 3);

	// Draw Player (interactive green dot)
	mlx_img_draw_circle(canvas, player_x, player_y, 15, 0x55FF55, 1);
	mlx_img_draw_circle(canvas, player_x, player_y, 15, 0xFFFFFF, 0); // Outlined white border

	// Overlay text statistics on the image buffer
	mlx_img_string_put(canvas, 50, 20, 0xFFFFFF, "MiniLibX Extensions Test - Real-Time Game Loop (~60 FPS)");
	mlx_img_string_put(canvas, 50, 580, 0x888888, "Use WASD or Arrow keys to move the green player. Scroll mouse wheel. Press ESC to exit.");

	char player_info[64];
	sprintf(player_info, "Player X: %d | Y: %d", player_x, player_y);
	mlx_img_string_put(canvas, player_x - 70, player_y - 32, 0x55FF55, player_info);

	/* 3. Flush canvas buffer to window */
	mlx_put_image_to_window(mlx, win, canvas, 0, 0);

	/* 4. Sleep to cap framerate at ~60fps and reduce CPU usage */
	usleep(16666); 
	return (0);
}

int	main(int argc, char **argv)
{
	char	*png_path = "test_sprite.png";

	if (argc > 1)
		png_path = argv[1];

	printf("Initializing MiniLibX...\n");
	mlx = mlx_init();
	if (!mlx)
	{
		fprintf(stderr, "Failed to initialize mlx\n");
		return (1);
	}

	printf("Creating window %dx%d...\n", WIN_WIDTH, WIN_HEIGHT);
	win = mlx_new_window(mlx, WIN_WIDTH, WIN_HEIGHT, "MiniLibX Extensions Test - WASD Game Loop");
	if (!win)
	{
		fprintf(stderr, "Failed to create window\n");
		return (1);
	}

	printf("Creating canvas image %dx%d...\n", WIN_WIDTH, WIN_HEIGHT);
	canvas = mlx_new_image(mlx, WIN_WIDTH, WIN_HEIGHT);
	if (!canvas)
	{
		fprintf(stderr, "Failed to create canvas image\n");
		return (1);
	}

	/* Load PNG Sprite */
	printf("Loading PNG: %s...\n", png_path);
	sprite = mlx_png_file_to_image(mlx, png_path, &sprite_w, &sprite_h);
	if (!sprite)
	{
		printf("Warning: Could not load PNG file: %s (we will draw a placeholder instead)\n", png_path);
		sprite = mlx_new_image(mlx, 128, 128);
		if (sprite)
		{
			sprite_w = mlx_get_image_width(sprite);
			sprite_h = mlx_get_image_height(sprite);
			for (int y = 0; y < sprite_h; y++)
			{
				for (int x = 0; x < sprite_w; x++)
				{
					int dx = x - 64;
					int dy = y - 64;
					int dist = dx*dx + dy*dy;
					int alpha = 255 - dist * 255 / (64*64);
					if (alpha < 0) alpha = 0;
					if (alpha > 255) alpha = 255;
					unsigned int col = mlx_rgba(0, 180, 255, alpha);
					mlx_img_put_pixel(sprite, x, y, col);
				}
			}
		}
	}
	else
	{
		sprite_w = mlx_get_image_width(sprite);
		sprite_h = mlx_get_image_height(sprite);
		printf("Successfully loaded PNG image via getters: size %dx%d, BPP: %d\n", 
			sprite_w, sprite_h, mlx_get_image_bpp(sprite));
	}

	/* Initialize Keyboard State Tracking Cache */
	mlx_ext_init_keyboard(win);

	/* Register Mouse Hooks */
	mlx_mouse_hook(win, mouse_hook, NULL);
	mlx_hook(win, 17, 0, close_window, NULL); // Window close button (X)

	/* Register Frame Updates Loop Hook */
	mlx_loop_hook(mlx, update_and_render, NULL);

	printf("Entering main loop...\n");
	mlx_loop(mlx);

	return (0);
}
