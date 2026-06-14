#include "mlx.h"
#include "mlx_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

void	*mlx;
void	*win1;
void	*win2;
void	*canvas1;
void	*canvas2;
int		win1_w = 300;
int		win1_h = 300;
int		win2_w = 400;
int		win2_h = 400;
double	angle = 0.0;

int	close_program(void *param)
{
	(void)param;
	printf("Exiting new_win test.\n");
	if (canvas1)
		mlx_destroy_image(mlx, canvas1);
	if (canvas2)
		mlx_destroy_image(mlx, canvas2);
	if (win1)
		mlx_destroy_window(mlx, win1);
	if (win2)
		mlx_destroy_window(mlx, win2);
	mlx_destroy_display(mlx);
	exit(0);
	return (0);
}

int	gere_mouse(int button, int x, int y, void *toto)
{
	(void)button;
	(void)x;
	(void)y;
	(void)toto;
	printf("Mouse event - recreating win1\n");
	if (win1)
		mlx_destroy_window(mlx, win1);
	if (canvas1)
		mlx_destroy_image(mlx, canvas1);

	win1_w = 150 + random() % 350;
	win1_h = 150 + random() % 350;
	win1 = mlx_new_window(mlx, win1_w, win1_h, "new win1");
	canvas1 = mlx_new_image(mlx, win1_w, win1_h);
	mlx_mouse_hook(win1, gere_mouse, 0);
	return (0);
}

int	update_loop(void *param)
{
	(void)param;
	angle += 0.05;

	/* Update and render win1 */
	if (win1 && canvas1)
	{
		mlx_img_clear(canvas1, 0x1E1E2E);
		mlx_img_draw_rect(canvas1, 10, 10, win1_w - 20, win1_h - 20, 0x55FF55, 0);
		mlx_img_draw_circle(canvas1, win1_w / 2, win1_h / 2, 30 + 10 * sin(angle), 0xFF5555, 1);
		mlx_img_string_put(canvas1, 20, 30, 0xFFFFFF, "Win1 (Click to resize)");
		char size_info[32];
		sprintf(size_info, "Size: %dx%d", win1_w, win1_h);
		mlx_img_string_put(canvas1, 20, 50, 0x55FFFF, size_info);
		mlx_put_image_to_window(mlx, win1, canvas1, 0, 0);
	}

	/* Update and render win2 */
	if (win2 && canvas2)
	{
		mlx_img_clear(canvas2, 0x2E1E1E);
		mlx_img_draw_rect(canvas2, 10, 10, win2_w - 20, win2_h - 20, 0x5555FF, 0);
		
		/* Draw rotating/pulsing circle */
		int cx = win2_w / 2;
		int cy = win2_h / 2;
		mlx_img_draw_circle(canvas2, cx, cy, 40, 0xFFFF55, 0);
		mlx_img_draw_line(canvas2, cx, cy, cx + 40 * cos(angle), cy + 40 * sin(angle), 0xFF55FF);

		mlx_img_string_put_scaled(canvas2, 20, 30, 0xFFFFFF, "Win2 (Static)", 2);
		mlx_put_image_to_window(mlx, win2, canvas2, 0, 0);
	}

	/* Keyboard Polling to exit */
	if (mlx_ext_is_key_down(65307) || mlx_ext_is_key_down(53) || mlx_ext_is_key_down(0xFF1B))
		close_program(NULL);

	usleep(16666);
	return (0);
}

int	main(void)
{
	srandom(time(0));
	printf("Initializing multi-window test with new extensions...\n");
	mlx_ext_print_help("mlx_ext_init_keyboard");

	mlx = mlx_init();
	if (!mlx)
		return (1);

	win1 = mlx_new_window(mlx, win1_w, win1_h, "win1");
	win2 = mlx_new_window(mlx, win2_w, win2_h, "win2");
	if (!win1 || !win2)
		return (1);

	canvas1 = mlx_new_image(mlx, win1_w, win1_h);
	canvas2 = mlx_new_image(mlx, win2_w, win2_h);
	if (!canvas1 || !canvas2)
		return (1);

	mlx_ext_init_keyboard(win1);
	mlx_ext_init_keyboard(win2);

	mlx_mouse_hook(win1, gere_mouse, 0);
	mlx_mouse_hook(win2, gere_mouse, 0);

	mlx_hook(win1, 17, 0, close_program, NULL);
	mlx_hook(win2, 17, 0, close_program, NULL);

	mlx_loop_hook(mlx, update_loop, NULL);
	mlx_loop(mlx);
	return (0);
}
