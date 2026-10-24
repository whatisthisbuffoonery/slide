/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vec3_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achew <achew@student.42singapore.sg>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 00:00:00 by achew             #+#    #+#             */
/*   Updated: 2026/10/21 00:00:00 by achew            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "vec3.h"

t_vec3	vec_cross(t_vec3 a, t_vec3 b)
{
	t_vec3	ret;

	ret = (t_vec3)
	{
		(a.y * b.z) - (a.z * b.y),
		(a.z * b.x) - (a.x * b.z),
		(a.x * b.y) - (a.y * b.x)
	};
	return (ret);
}

//returns unsquared magnitude
//for squared magnitude, just call vec_dot(a, a)
//vec_dot(a, b) returns cos(angle between the two) in unitless range -1, 1
//^^ only if a and b were normalised
double	vec_magnitude(t_vec3 a)
{
	return (sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z)));
}

t_vec3	vec_norm(t_vec3 a)
{
	return (vec_scale(a, 1.0 / vec_magnitude(a)));
}
