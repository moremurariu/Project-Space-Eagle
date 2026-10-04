#!/bin/bash
# a recorded run to video: render.sh <demo name> (from /home/user/runs/demos); the video lands in
# /home/user/runs/render/home/user/videos/<name>.mp4. Needs Xvfb on :99 (Xvfb :99 -screen 0 1280x720x24 &), the client
# built with -DVIDEORECORDER=ON, the map at render/home/user/maps/tasmap.map and claude.png in render/home/user/skins
N=$1
cd /home/user/runs/render/home
cp /home/user/runs/demos/$N.demo user/demos/
DDNET_DEMO_RENDER_WAIT_MS=8000 DDNET_READ_BACK_BUFFER=1 DISPLAY=:99 LP_NUM_THREADS=1 taskset -c 0,1 nice /home/user/ddnet/build-client/DDNet "gfx_backend OpenGL; gfx_gl_major 3; gfx_gl_minor 3; gfx_fullscreen 0; gfx_screen_width 1280; gfx_screen_height 720; cl_default_zoom 12; cl_video_sound_enable 0; snd_enable 0; player_skin claude; demo_render demos/$N.demo $N 4 1 127" > render_$N.log 2>&1
ls -la user/videos/$N.mp4
