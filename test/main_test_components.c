#include "mlx.h"
#include "mlx_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

void		*mlx;
void		*win;
void		*canvas;
t_mlx_node	*root_node;
t_mlx_node	*counter_label;

int	click_counter = 0;

int	close_window(void *param)
{
	(void)param;
	printf("Exiting components test.\n");
	if (root_node)
		root_node->destroy();
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
	return (0);
}

int	mouse_press_hook(int button, int x, int y, void *param)
{
	(void)param;
	// Propagate clicks down the Node Hierarchy
	root_node->handle_click(button, x, y, 0, 0);
	return (0);
}

void	on_increment_click(t_mlx_node *self, int button, int lx, int ly)
{
	char	buf[64];

	(void)button;
	(void)lx;
	(void)ly;
	click_counter++;
	printf("Button increment clicked! Total clicks: %d\n", click_counter);
	
	// Dynamic Node Inspection via OOP member functions
	self->print_help();
	if (self->parent)
	{
		printf("--- Parent Panel Information ---\n");
		self->parent->print_help();
	}

	sprintf(buf, "Clicks: %d", click_counter);
	if (counter_label->text)
		free(counter_label->text);
	counter_label->text = strdup(buf);
}

void	on_exit_click(t_mlx_node *self, int button, int lx, int ly)
{
	(void)self;
	(void)button;
	(void)lx;
	(void)ly;
	close_window(NULL);
}

int	rect_direction = 1;
void	on_rect_update(t_mlx_node *self)
{
	self->x += rect_direction * 3;
	// Bounce between x = 400 and WIN_WIDTH - w - 50
	if (self->x > WIN_WIDTH - self->w - 50)
	{
		self->x = WIN_WIDTH - self->w - 50;
		rect_direction = -1;
		self->color = 0x55FFFF; // Cyan
	}
	else if (self->x < 400)
	{
		self->x = 400;
		rect_direction = 1;
		self->color = 0xFF5555; // Red
	}
}

int	update_and_render(void *param)
{
	(void)param;
	// 1. Keyboard ESC check via ext
	if (mlx_ext_is_key_down(65307) || mlx_ext_is_key_down(53) || mlx_ext_is_key_down(0xFF1B))
		close_window(NULL);

	// 2. Run Scene updates (triggers on_update callbacks)
	root_node->update();

	// 3. Clear base canvas
	mlx_img_clear(canvas, 0x181824); // Deep navy background

	// Draw decorative grids
	for (int i = 0; i < WIN_WIDTH; i += 40)
		mlx_img_draw_line(canvas, i, 0, i, WIN_HEIGHT, 0x20202E);
	for (int j = 0; j < WIN_HEIGHT; j += 40)
		mlx_img_draw_line(canvas, 0, j, WIN_WIDTH, j, 0x20202E);

	// 4. Render entire Node Hierarchy
	root_node->render(canvas, 0, 0);

	// 5. Flip canvas to window
	mlx_put_image_to_window(mlx, win, canvas, 0, 0);

	usleep(16666); // cap at 60 FPS
	return (0);
}

int	main()
{
	printf("Initializing Components Test...\n");
	mlx = mlx_init();
	if (!mlx)
		return (1);

	win = mlx_new_window(mlx, WIN_WIDTH, WIN_HEIGHT, "MiniLibX Scene Graph & UI Components Test");
	if (!win)
		return (1);

	canvas = mlx_new_image(mlx, WIN_WIDTH, WIN_HEIGHT);
	if (!canvas)
		return (1);

	/* 1. Create root Scene Node */
	root_node = mlx_node_create(NODE_TYPE_CONTAINER, 0, 0, WIN_WIDTH, WIN_HEIGHT);

	/* 2. Title and Description Texts */
	t_mlx_node *title = mlx_node_create_text("Scene Graph Node & UI Component System", 50, 40, 0xFFFFFF);
	root_node->add_child(title);

	t_mlx_node *sub = mlx_node_create_text("Hierarchical rendering and events built on top of MiniLibX", 50, 60, 0x888898);
	root_node->add_child(sub);

	/* 3. Panel Container Node */
	t_mlx_node *panel = mlx_node_create_rect(50, 100, 300, 450, 0x242434, 1);
	root_node->add_child(panel);

	// Panel Border Rect
	t_mlx_node *panel_border = mlx_node_create_rect(0, 0, 300, 450, 0x3F3F5F, 0);
	panel->add_child(panel_border);

	// Panel Title Label
	t_mlx_node *panel_title = mlx_node_create_text("INTERACTIVE CONTROL PANEL", 20, 20, 0x00FF88);
	panel->add_child(panel_title);

	/* 4. Interactive Increment Button (added as child of panel) */
	t_mlx_node *btn_inc = mlx_node_create_button("Click Me!", 30, 60, 240, 50, on_increment_click);
	panel->add_child(btn_inc);

	/* 5. Clicks Counter Display Label (added as child of panel) */
	counter_label = mlx_node_create_text("Clicks: 0", 30, 130, 0x55FF55);
	panel->add_child(counter_label);

	// Info label
	t_mlx_node *info = mlx_node_create_text("Clicks are captured relative to parent", 30, 160, 0x888888);
	panel->add_child(info);

	/* 6. Exit Button (added as child of panel) */
	t_mlx_node *btn_exit = mlx_node_create_button("Exit Program", 30, 370, 240, 50, on_exit_click);
	btn_exit->color = 0x5F2E2E; // Reddish exit button
	panel->add_child(btn_exit);

	/* 7. Moving rect node with a loop update logic callback */
	t_mlx_node *moving_rect = mlx_node_create_rect(420, 250, 100, 100, 0xFF5555, 1);
	moving_rect->on_update = on_rect_update;
	root_node->add_child(moving_rect);

	t_mlx_node *moving_label = mlx_node_create_text("Update Loop Object", 420, 230, 0xFFFFFF);
	root_node->add_child(moving_label);

	/* Initialize Keyboard Cache */
	mlx_ext_init_keyboard(win);

	/* Hooks */
	mlx_mouse_hook(win, mouse_press_hook, NULL);
	mlx_key_hook(win, key_hook, NULL);
	mlx_hook(win, 17, 0, close_window, NULL);

	/* Loop Updater */
	mlx_loop_hook(mlx, update_and_render, NULL);

	/* Demonstrate Built-in Documentation Help Reader */
	printf("\n========================================\n");
	printf("PRINTING HELP FOR SOME SYSTEM FUNCTIONS:\n");
	printf("========================================\n");
	mlx_ext_print_help("mlx_scale_image");
	mlx_ext_print_help("t_mlx_node");

	printf("Starting component scene graph loop...\n");
	mlx_loop(mlx);

	return (0);
}
