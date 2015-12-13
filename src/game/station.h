#ifndef SPACE_STATION_H_
#define SPACE_STATION_H_

#include <camera.h>
#include <sprite.h>
#include <mission.h>

typedef struct
{
	space_entity e;
	whitgl_float angle_speed;
	whitgl_bool defensive;
	mission_index mission_id;
} space_station;

space_station space_station_zero(whitgl_int num_points, whitgl_fvec pos, mission_index mission_id);
space_station space_station_update(space_station s);
void space_station_draw(space_station s, space_camera camera);

#endif // SPACE_STATION_H_
