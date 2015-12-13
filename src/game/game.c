#include "game.h"

#include <whitgl/logging.h>
#include <whitgl/sound.h>
#include <resource.h>

space_game space_game_zero(whitgl_ivec screen_size)
{
	whitgl_int i;
	space_game g;
	space_camera camera = {{0.0,0.0}, 8, whitgl_ivec_to_fvec(screen_size), {0,0}};
	g.camera = camera;
	g.player = space_player_zero;
	for(i=0; i<NUM_PIRATES; i++)
	{
		g.pirates[i] = space_pirate_zero;
		g.pirates[i].e.pos.x += i*4;
	}
	whitgl_fvec diso_pos = {30,10};
	g.stations[0] = space_station_zero(5, diso_pos);
	whitgl_fvec centurai_pos = {-10,-40};
	g.stations[1] = space_station_zero(7, centurai_pos);
	whitgl_fvec alpha_pos = {-50,75};
	g.stations[2] = space_station_zero(11, alpha_pos);
	for(i=0; i<NUM_ASTEROIDS; i++)
		g.asteroids[i] = space_asteroid_zero();
	g.starfield = space_starfield_zero();
	g.debris = space_debris_zero();
	return g;
}
space_game space_game_update(space_game g, whitgl_ivec screen_size, whitgl_fvec camera_offset)
{
	whitgl_int i;
	g.player = space_player_update(g.player);
	for(i=0; i<NUM_PIRATES; i++)
		g.pirates[i] = space_pirate_update(g.pirates[i], g.player.e.pos);
	for(i=0; i<NUM_STATIONS; i++)
		g.stations[i] = space_station_update(g.stations[i]);
	for(i=0; i<NUM_ASTEROIDS; i++)
		g.asteroids[i] = space_asteroid_update(g.asteroids[i], g.player.e.pos);
	g.starfield = space_starfield_update(g.starfield, g.camera.speed, g.camera);
	g.debris = space_debris_update(g.debris);

	g.player.docked = -1;
	for(i=0; i<NUM_STATIONS; i++)
	{
		whitgl_float diff = whitgl_fvec_magnitude(whitgl_fvec_sub(g.player.e.pos, g.stations[i].e.pos));
		if(diff < 1 && g.player.engine_thrust[0]+g.player.engine_thrust[1] < 1.8)
			g.player.docked = i;
	}
	if(g.player.docked != -1)
	{
		g.player.e.pos = whitgl_fvec_interpolate(g.player.e.pos, g.stations[g.player.docked].e.pos, 0.1);
		g.player.e.angle = whitgl_angle_lerp(g.player.e.angle, g.stations[g.player.docked].e.angle, 0.1);
	}

	space_camera_focus focus;
	g.hud = space_hud_markers_zero;
	focus.num_foci = 0;
	whitgl_fvec focus_pos = whitgl_fvec_add(g.player.e.pos, whitgl_fvec_scale_val(g.player.speed, 25));
	if(g.player.e.active)
	{
		focus.foci[focus.num_foci].a = whitgl_fvec_sub(focus_pos, whitgl_fvec_val(3));
		focus.foci[focus.num_foci].b = whitgl_fvec_add(focus_pos, whitgl_fvec_val(3));
		focus.num_foci++;
		focus.foci[focus.num_foci].a = whitgl_fvec_sub(focus_pos, whitgl_fvec_val(3));
		focus.foci[focus.num_foci].b = whitgl_fvec_add(focus_pos, whitgl_fvec_val(3));
		focus.num_foci++;
	}
	focus_pos = whitgl_fvec_add(focus_pos, whitgl_fvec_scale_val(g.player.speed, 30));
	for(i=0; i<NUM_PIRATES; i++)
	{
		whitgl_float diff = whitgl_fvec_magnitude(whitgl_fvec_sub(focus_pos, g.pirates[i].e.pos));
		whitgl_float mult = g.pirates[i].e.seen ? 1.25 : 1;
		g.pirates[i].e.seen = false;
		if(diff < 20*mult && g.player.e.active)
		{
			g.pirates[i].e.seen = true;
			focus.foci[focus.num_foci].a = whitgl_fvec_sub(g.pirates[i].e.pos, whitgl_fvec_val(3));
			focus.foci[focus.num_foci].b = whitgl_fvec_add(g.pirates[i].e.pos, whitgl_fvec_val(3));
			focus.num_foci++;
		}
	}
	for(i=0; i<NUM_STATIONS; i++)
	{
		whitgl_float diff = whitgl_fvec_magnitude(whitgl_fvec_sub(focus_pos, g.stations[i].e.pos));
		whitgl_float mult = g.stations[i].e.seen ? 1.25 : 1;
		g.stations[i].e.seen = false;
		if(diff < 20*mult && g.player.e.active)
		{
			g.stations[i].e.seen = true;
			focus.foci[focus.num_foci].a = whitgl_fvec_sub(g.stations[i].e.pos, whitgl_fvec_val(2));
			focus.foci[focus.num_foci].b = whitgl_fvec_add(g.stations[i].e.pos, whitgl_fvec_val(2));
			focus.num_foci++;
		} else
		{
			g.hud.marker[g.hud.num++] = g.stations[i].e;
		}
	}
	for(i=0; i<NUM_ASTEROIDS; i++)
	{
		whitgl_float diff = whitgl_fvec_magnitude(whitgl_fvec_sub(focus_pos, g.asteroids[i].e.pos));
		whitgl_float mult = g.asteroids[i].e.seen ? 1.25 : 1;
		g.asteroids[i].e.seen = false;
		if(diff < 12*mult && g.player.e.active)
		{
			g.asteroids[i].e.seen = true;
			focus.foci[focus.num_foci].a = whitgl_fvec_sub(g.asteroids[i].e.pos, whitgl_fvec_val(1));
			focus.foci[focus.num_foci].b = whitgl_fvec_add(g.asteroids[i].e.pos, whitgl_fvec_val(1));
			focus.num_foci++;
		}
	}
	for(i=0; i<MAX_PIECES; i++)
	{
		if(g.player.e.active || !g.debris.pieces[i].e.active)
			continue;
		focus.foci[focus.num_foci].a = whitgl_fvec_sub(g.debris.pieces[i].e.pos, whitgl_fvec_val(1));
		focus.foci[focus.num_foci].b = whitgl_fvec_add(g.debris.pieces[i].e.pos, whitgl_fvec_val(1));
		focus.num_foci++;
	}
	g.camera = space_camera_update(g.camera, focus, screen_size, camera_offset);

	whitgl_bool colliding = false;
	for(i=0; i<NUM_STATIONS; i++)
		colliding |= space_entity_colliding(g.player.e, g.stations[i].e);
	for(i=0; i<NUM_ASTEROIDS; i++)
		colliding |= space_entity_colliding(g.player.e, g.asteroids[i].e);
	if(colliding && g.player.e.active)
	{
		g.debris = space_debris_create(g.debris, g.player.e, g.player.speed);
		g.player.e.active = false;
		whitgl_sound_play(SOUND_EXPLODE, 1);
	}
	whitgl_int j;
	for(j=0; j<NUM_PIRATES; j++)
	{
		colliding = false;
		for(i=0; i<NUM_STATIONS; i++)
			colliding |= space_entity_colliding(g.pirates[j].e, g.stations[i].e);
		for(i=0; i<NUM_ASTEROIDS; i++)
			colliding |= space_entity_colliding(g.pirates[j].e, g.asteroids[i].e);
		if(colliding && g.pirates[j].e.active)
		{
			g.debris = space_debris_create(g.debris, g.pirates[j].e, g.player.speed);
			g.pirates[j].e.active = false;
			whitgl_sound_play(SOUND_EXPLODE, 1.5);
		}
	}
	for(j=0; j<NUM_PIRATES; j++)
	{
		if(!g.pirates[j].e.active)
			continue;
		for(i=0; i<NUM_PIRATES; i++)
		{
			if(i==j)
				continue;
			if(!g.pirates[i].e.active)
				continue;
			if(space_entity_colliding(g.pirates[j].e, g.pirates[i].e))
			{
				g.debris = space_debris_create(g.debris, g.pirates[j].e, g.player.speed);
				g.pirates[j].e.active = false;
				g.debris = space_debris_create(g.debris, g.pirates[i].e, g.player.speed);
				g.pirates[i].e.active = false;
				whitgl_sound_play(SOUND_EXPLODE, 1.5);
			}
		}
	}
	for(i=0; i<NUM_PIRATES; i++)
	{
		if(!g.player.e.active)
			continue;
		if(!g.pirates[i].e.active)
			continue;
		if(space_entity_colliding(g.player.e, g.pirates[i].e))
		{
			g.debris = space_debris_create(g.debris, g.player.e, g.player.speed);
			g.player.e.active = false;
			g.debris = space_debris_create(g.debris, g.pirates[i].e, g.player.speed);
			g.pirates[i].e.active = false;
			whitgl_sound_play(SOUND_EXPLODE, 1);
			whitgl_sound_play(SOUND_EXPLODE, 1.5);
		}
	}
	return g;
}

void space_game_draw(space_game g)
{
	whitgl_int i;
	space_debris_draw(g.debris, g.camera);
	space_player_draw(g.player, g.camera);
	for(i=0; i<NUM_PIRATES; i++)
		space_pirate_draw(g.pirates[i], g.camera);
	for(i=0; i<NUM_STATIONS; i++)
		space_station_draw(g.stations[i], g.camera);
	for(i=0; i<NUM_ASTEROIDS; i++)
		space_asteroid_draw(g.asteroids[i], g.camera);
	space_starfield_draw(g.starfield, g.camera);
	space_hud_draw(g.player.e, g.hud, g.camera);

}
