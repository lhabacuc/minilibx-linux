#ifndef MLX_EXT_H
# define MLX_EXT_H

/* 
** 2D Drawing Utilities directly on Image buffers 
*/
void			mlx_img_put_pixel(void *img_ptr, int x, int y, int color);
unsigned int	mlx_img_get_pixel(void *img_ptr, int x, int y);
void			mlx_img_draw_line(void *img_ptr, int x1, int y1, int x2, int y2, int color);
void			mlx_img_draw_rect(void *img_ptr, int x, int y, int w, int h, int color, int fill);
void			mlx_img_draw_circle(void *img_ptr, int cx, int cy, int r, int color, int fill);

/* 
** Alpha Blending (Transparency) 
*/
void			mlx_blend_image_to_image(void *src_img, void *dst_img, int x_offset, int y_offset, float opacity);

/* 
** PNG/JPEG Loader 
*/
void			*mlx_png_file_to_image(void *mlx_ptr, char *filename, int *width, int *height);

/* 
** Image Property Getters 
*/
int				mlx_get_image_width(void *img_ptr);
int				mlx_get_image_height(void *img_ptr);
int				mlx_get_image_bpp(void *img_ptr);

/* 
** Sub-image / Sprite Sheet Extraction 
*/
void			*mlx_get_sub_image(void *mlx_ptr, void *src_img, int x, int y, int w, int h);

/* 
** Fast Image Clearing 
*/
void			mlx_img_clear(void *img_ptr, int color);

/* 
** Color Utilities 
*/
int				mlx_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
unsigned char	mlx_get_r(int color);
unsigned char	mlx_get_g(int color);
unsigned char	mlx_get_b(int color);
unsigned char	mlx_get_a(int color);

/* 
** Draw Text Directly on Image Buffer 
*/
void			mlx_img_string_put(void *img_ptr, int x, int y, int color, char *str);
void			mlx_img_string_put_scaled(void *img_ptr, int x, int y, int color, char *str, int scale);

/* 
** Keyboard State Cache (Polling) 
*/
void			mlx_ext_init_keyboard(void *win_ptr);
int				mlx_ext_is_key_down(int keycode);

/* 
** Mouse Scroll Helpers 
*/
int				mlx_ext_is_scroll_up(int button);
int				mlx_ext_is_scroll_down(int button);

/* 
** Mouse Look Delta (For 3D camera look - dynamically centers mouse) 
*/
void			mlx_ext_get_mouse_delta(void *mlx_ptr, void *win_ptr, int *dx, int *dy);

/* 
** Image Transformations (Scaling and Rotation) 
*/
void			*mlx_scale_image(void *mlx_ptr, void *src_img, int new_width, int new_height);
void			*mlx_rotate_image(void *mlx_ptr, void *src_img, double angle_rad);

/* 
** Scene Graph Node System (Componentization) 
*/
# define NODE_TYPE_CONTAINER 0
# define NODE_TYPE_SPRITE    1
# define NODE_TYPE_RECT      2
# define NODE_TYPE_BUTTON    3
# define NODE_TYPE_TEXT      4

typedef struct s_mlx_node
{
	int					type;
	int					x;
	int					y;
	int					w;
	int					h;
	unsigned int		color;
	int					fill;
	void				*img;
	char				*text;
	int					visible;
	int					enabled;
	struct s_mlx_node	*parent;
	struct s_mlx_node	*next;
	struct s_mlx_node	*children;
	void				(*on_update)(struct s_mlx_node *self);
	void				(*on_click)(struct s_mlx_node *self, int button, int local_x, int local_y);
	void				*user_data;

	/* OOP-style Member Functions */
	void				(*add_child)(struct s_mlx_node *child);
	void				(*remove_child)(struct s_mlx_node *child);
	void				(*destroy)(void);
	void				(*print_help)(void);
	void				(*update)(void);
	void				(*render)(void *canvas_img, int parent_abs_x, int parent_abs_y);
	int					(*handle_click)(int button, int x, int y, int parent_abs_x, int parent_abs_y);
}	t_mlx_node;

/* Node Constructors and Lifecycle */
t_mlx_node		*mlx_node_create(int type, int x, int y, int w, int h);
void			mlx_node_destroy(t_mlx_node *node);
void			mlx_node_add_child(t_mlx_node *parent, t_mlx_node *child);
void			mlx_node_remove_child(t_mlx_node *parent, t_mlx_node *child);

/* Specialized Constructors */
t_mlx_node		*mlx_node_create_sprite(void *img, int x, int y);
t_mlx_node		*mlx_node_create_rect(int x, int y, int w, int h, unsigned int color, int fill);
t_mlx_node		*mlx_node_create_button(char *text, int x, int y, int w, int h, void (*on_click)(t_mlx_node *self, int button, int lx, int ly));
t_mlx_node		*mlx_node_create_text(char *text, int x, int y, unsigned int color);

/* 
** Engine update and render calls */
void			mlx_node_update(t_mlx_node *root);
void			mlx_node_render(t_mlx_node *root, void *canvas_img, int parent_abs_x, int parent_abs_y);
int				mlx_node_handle_click(t_mlx_node *root, int button, int x, int y, int parent_abs_x, int parent_abs_y);

/* 
** Help & Documentation Utilities 
*/
void			mlx_ext_print_help(char *name);
void			mlx_node_print_help(t_mlx_node *node);

/* 
** OOP Helper Macros 
*/
#define node_add_child(parent, child)     (parent)->add_child((child))
#define node_remove_child(parent, child)  (parent)->remove_child((child))
#define node_destroy(node)                (node)->destroy()
#define node_print_help(node)             (node)->print_help()
#define node_update(node)                 (node)->update()
#define node_render(node, canvas, px, py) (node)->render((canvas), (px), (py))
#define node_handle_click(node, btn, x, y, px, py) (node)->handle_click((btn), (x), (y), (px), (py))

#endif
