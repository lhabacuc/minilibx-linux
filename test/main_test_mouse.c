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
void	*paint_layer; // Persistent drawing layer

int		mouse_x = 0;
int		mouse_y = 0;
int		last_click_button = 0;
int		last_click_x = 0;
int		last_click_y = 0;
int		is_drawing = 0;
int		prev_draw_x = -1;
int		prev_draw_y = -1;

// Mouse look variables
int		mouse_look_enabled = 0;
int		accumulated_dx = 0;
int		accumulated_dy = 0;
int		current_delta_x = 0;
int		current_delta_y = 0;

int	close_window(void *param)
{
	(void)param;
	printf("Exiting mouse test.\n");
	if (paint_layer)
		mlx_destroy_image(mlx, paint_layer);
	if (canvas)
		mlx_destroy_image(mlx, canvas);
	if (win)
		mlx_destroy_window(mlx, win);
	mlx_destroy_display(mlx);
	exit(0);
	return (0);
}

int	key_hook(int keycode, void *param)
{
	(void)param;
	if (keycode == 65307 || keycode == 53 || keycode == 0xFF1B) // ESC key
		close_window(NULL);
	if (keycode == 32) // SPACE key to toggle mouse look
	{
		mouse_look_enabled = !mouse_look_enabled;
		if (mouse_look_enabled)
		{
			mlx_mouse_hide(mlx, win);
			// Warp mouse to center initially
			mlx_mouse_move(mlx, win, WIN_WIDTH / 2, WIN_HEIGHT / 2);
		}
		else
		{
			mlx_mouse_show(mlx, win);
		}
		printf("Mouse look: %s\n", mouse_look_enabled ? "ENABLED" : "DISABLED");
	}
	return (0);
}

int	mouse_press_hook(int button, int x, int y, void *param)
{
	(void)param;
	last_click_button = button;
	last_click_x = x;
	last_click_y = y;

	if (button == 1) // Left Click: Start drawing
	{
		is_drawing = 1;
		prev_draw_x = x;
		prev_draw_y = y;
	}
	else if (button == 2) // Middle Click: Clear drawing layer
	{
		// Fill paint layer with transparency (alpha = 0)
		mlx_img_clear(paint_layer, 0x00000000);
	}
	return (0);
}

int	mouse_release_hook(int button, int x, int y, void *param)
{
	(void)x;
	(void)y;
	(void)param;
	if (button == 1)
	{
		is_drawing = 0;
		prev_draw_x = -1;
		prev_draw_y = -1;
	}
	return (0);
}

int	mouse_move_hook(int x, int y, void *param)
{
	(void)param;
	if (!mouse_look_enabled)
	{
		mouse_x = x;
		mouse_y = y;

		if (is_drawing && prev_draw_x != -1 && prev_draw_y != -1)
		{
			// Draw line on the paint layer (Orange brush color)
			mlx_img_draw_line(paint_layer, prev_draw_x, prev_draw_y, x, y, 0xFFFF5500);
			prev_draw_x = x;
			prev_draw_y = y;
		}
	}
	return (0);
}

int	update_and_render(void *param)
{
	(void)param;
	// 1. Handle keyboard ESC check directly via cache
	if (mlx_ext_is_key_down(65307) || mlx_ext_is_key_down(53) || mlx_ext_is_key_down(0xFF1B))
		close_window(NULL);

	// 2. Handle Mouse Look Delta if enabled
	if (mouse_look_enabled)
	{
		int dx = 0, dy = 0;
		mlx_ext_get_mouse_delta(mlx, win, &dx, &dy);
		current_delta_x = dx;
		current_delta_y = dy;
		accumulated_dx += dx;
		accumulated_dy += dy;
	}

	// 3. Render base canvas
	mlx_img_clear(canvas, 0x1E1E2E); // slate background

	// Grid background
	for (int i = 0; i < WIN_WIDTH; i += 40)
		mlx_img_draw_line(canvas, i, 0, i, WIN_HEIGHT, 0x252535);
	for (int j = 0; j < WIN_HEIGHT; j += 40)
		mlx_img_draw_line(canvas, 0, j, WIN_WIDTH, j, 0x252535);

	// Render paint layer (drawn lines) over the canvas with alpha transparency
	mlx_blend_image_to_image(paint_layer, canvas, 0, 0, 1.0f);

	// Draw HUD box on top right
	mlx_img_draw_rect(canvas, WIN_WIDTH - 260, 20, 240, 160, 0x3F3F5F, 1);
	mlx_img_draw_rect(canvas, WIN_WIDTH - 260, 20, 240, 160, 0xFFFFFF, 0);

	char hud_text[128];
	mlx_img_string_put(canvas, WIN_WIDTH - 250, 30, 0x00FF88, "MOUSE STATUS HUD");
	
	sprintf(hud_text, "Mouse X: %d  Y: %d", mouse_x, mouse_y);
	mlx_img_string_put(canvas, WIN_WIDTH - 250, 50, 0xFFFFFF, hud_text);

	sprintf(hud_text, "Last Button Click: %d", last_click_button);
	mlx_img_string_put(canvas, WIN_WIDTH - 250, 70, 0xFFFFFF, hud_text);

	sprintf(hud_text, "Click Pos: %dx%d", last_click_x, last_click_y);
	mlx_img_string_put(canvas, WIN_WIDTH - 250, 90, 0xFFFFFF, hud_text);

	sprintf(hud_text, "Mouse Look: %s", mouse_look_enabled ? "ON" : "OFF");
	mlx_img_string_put(canvas, WIN_WIDTH - 250, 110, mouse_look_enabled ? 0x55FF55 : 0xFF5555, hud_text);

	sprintf(hud_text, "Delta X: %d  Y: %d", current_delta_x, current_delta_y);
	mlx_img_string_put(canvas, WIN_WIDTH - 250, 130, 0x55FFFF, hud_text);

	sprintf(hud_text, "Accumulated: %dx%d", accumulated_dx, accumulated_dy);
	mlx_img_string_put(canvas, WIN_WIDTH - 250, 155, 0x55FFFF, hud_text);

	// Draw target crosshair if mouse look is disabled (to show mouse pos)
	if (!mouse_look_enabled)
	{
		// Draw crosshair at cursor
		mlx_img_draw_line(canvas, mouse_x - 10, mouse_y, mouse_x + 10, mouse_y, 0xFF55FF);
		mlx_img_draw_line(canvas, mouse_x, mouse_y - 10, mouse_x, mouse_y + 10, 0xFF55FF);
		mlx_img_draw_circle(canvas, mouse_x, mouse_y, 6, 0xFF55FF, 0);
	}
	else
	{
		// Draw a circle centered at (400, 300) that moves based on accumulated delta
		int target_x = WIN_WIDTH / 2 + accumulated_dx;
		int target_y = WIN_HEIGHT / 2 + accumulated_dy;
		// Bound within screen
		if (target_x < 10) target_x = 10;
		if (target_x > WIN_WIDTH - 10) target_x = WIN_WIDTH - 10;
		if (target_y < 10) target_y = 10;
		if (target_y > WIN_HEIGHT - 10) target_y = WIN_HEIGHT - 10;

		mlx_img_draw_circle(canvas, target_x, target_y, 20, 0x55FFFF, 1);
		mlx_img_draw_circle(canvas, target_x, target_y, 20, 0xFFFFFF, 0);
		mlx_img_string_put(canvas, target_x - 45, target_y - 32, 0x55FFFF, "Mouse Look Obj");

		// Draw fixed center crosshair
		mlx_img_draw_line(canvas, WIN_WIDTH/2 - 15, WIN_HEIGHT/2, WIN_WIDTH/2 + 15, WIN_HEIGHT/2, 0x888888);
		mlx_img_draw_line(canvas, WIN_WIDTH/2, WIN_HEIGHT/2 - 15, WIN_WIDTH/2, WIN_HEIGHT/2 + 15, 0x888888);
	}

	// General instructions
	mlx_img_string_put(canvas, 30, 20, 0xFFFFFF, "MiniLibX Mouse Extensions Test");
	mlx_img_string_put(canvas, 30, 45, 0xAAAAAA, "Left Click + Drag: Draw on Canvas");
	mlx_img_string_put(canvas, 30, 65, 0xAAAAAA, "Middle Click: Clear Drawing");
	mlx_img_string_put(canvas, 30, 85, 0xAAAAAA, "Scroll Mouse Wheel: Register event on HUD");
	mlx_img_string_put(canvas, 30, 105, 0xAAAAAA, "SPACE: Toggle Mouse Look (centers cursor & tracks deltas)");
	mlx_img_string_put(canvas, 30, 570, 0x888888, "Press ESC to exit");

	// Flip canvas to window
	mlx_put_image_to_window(mlx, win, canvas, 0, 0);

	usleep(16666);
	return (0);
}

int	main()
{
	printf("Initializing Mouse Test...\n");
	mlx = mlx_init();
	if (!mlx)
		return (1);

	win = mlx_new_window(mlx, WIN_WIDTH, WIN_HEIGHT, "MiniLibX Mouse Extensions Test");
	if (!win)
		return (1);

	canvas = mlx_new_image(mlx, WIN_WIDTH, WIN_HEIGHT);
	paint_layer = mlx_new_image(mlx, WIN_WIDTH, WIN_HEIGHT);
	if (!canvas || !paint_layer)
		return (1);

	// Clear paint layer to fully transparent (alpha = 0)
	mlx_img_clear(paint_layer, 0x00000000);

	// Initialize keyboard cache
	mlx_ext_init_keyboard(win);

	// Hook events
	mlx_mouse_hook(win, mouse_press_hook, NULL);
	
	// Hook button release (5) and motion notify (6)
	mlx_hook(win, 5, 1L << 3, mouse_release_hook, NULL); // ButtonReleaseMask
	mlx_hook(win, 6, 1L << 6, mouse_move_hook, NULL);    // PointerMotionMask

	mlx_hook(win, 17, 0, close_window, NULL); // Close button
	mlx_key_hook(win, key_hook, NULL);

	// Loop hook
	mlx_loop_hook(mlx, update_and_render, NULL);

	printf("Starting Loop...\n");
	mlx_loop(mlx);

	return (0);
}
