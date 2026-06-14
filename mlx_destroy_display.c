/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_destroy_display.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mg <mg@student.42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/03 18:56:35 by mg                #+#    #+#             */
/*   Updated: 2020/10/04 01:55:35 by mg               ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "mlx_int.h"

int	mlx_destroy_display(t_xvar *xvar)
{
	t_win_list	*w;
	t_win_list	*next;

	w = xvar->win_list;
	while (w)
	{
		next = w->next;
		XFreeGC(xvar->display, w->gc);
		free(w);
		w = next;
	}
	xvar->win_list = NULL;
	if (xvar->private_cmap)
		XFreeColormap(xvar->display, xvar->cmap);
	XCloseDisplay(xvar->display);
	return (0);
}
