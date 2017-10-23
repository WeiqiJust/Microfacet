#include "worker.h"


#define PREFIX	"T:/Microfacet/output/animation/"

#include "Magick++.h"
using namespace Magick;


void worker_microfacet::work_task_animation(task_microfacet *t_org)
{
	task_animation	&t = t_org->ta;
	task_microfacet tm;
	char			filename[MAX_PATH];

	Vector3 eye, lookat, v_eye_lookat;
	eye			= *t.eye;
	lookat		= *t.lookat;
	v_eye_lookat= eye - lookat;

	for (int frame = 0; frame < t.angle_num; frame++)
	{
		printf_s("\n******* frame %d / %d *******\n", frame+1, t.angle_num);

		float angle = (t.angle_start+(t.angle_finish-t.angle_start)*(float)frame/t.angle_num)/360.0*2*PI;
		Vector3 current_eye;
		Matrix4 mat_rot, matView;
		mat_rot = RotateY(angle);
		current_eye = mat_rot*v_eye_lookat + lookat;
		matrix_lookat(matView, current_eye, lookat, Vector3(0, 1, 0));

		Vector3 v_up, v_right, v_lookat;
		Vector3 v_temp;
		v_temp = current_eye - lookat;
		v_temp.normalize();
		v_lookat.x = (float)v_temp.x;
		v_lookat.y = (float)v_temp.y;
		v_lookat.z = (float)v_temp.z;
		v_up = Vector3(0, 1, 0);
		v_up -= (v_up*v_lookat)*v_lookat;
		v_up.normalize();
		v_right = v_up ^ v_lookat;
		v_right.normalize();

		//Buffer
		tm.type		= TASK_TYPE_BUFFER;
		tm.id		= 0;
		tm.width	= t_org->width;;
		tm.height	= t_org->height;
		tm.matProj = t_org->matProj;
		tm.matView = matView;
		tm.tb.p_background		= t.p_background;
		tm.tb.p_main			= t.p_main;
		tm.tb.p_background_vis	= t.p_background_vis;
		tm.tb.p_main_vis		= t.p_main_vis;
		tm.tb.p_skybox			= t.p_skybox;
		tm.tb.p_shadow			= t.p_shadow;
		tm.tb.p_target			= t.p_target;
		tm.tb.p_target_uv		= t.p_target_uv;
		tm.tb.p_target_normal	= t.p_target_normal;
		tm.tb.p_target_tangent	= t.p_target_tangent;
		tm.tb.p_target_vis		= t.p_target_vis;
		tm.tb.p_screen			= t.p_screen;
		tm.tb.p_uv				= t.p_uv;
		tm.tb.p_normal			= t.p_normal;
		tm.tb.p_tangent			= t.p_tangent;
		tm.tb.vis_pixel_size	= t.vis_pixel_size;
		tm.tb.p_vis_buffer		= t.p_vis_buffer;
		tm.tb.result			= t.result;
		work_task_buffer(&tm);
		tm.tb.p_screen->get_rt(0)->get_result((BYTE*)tm.tb.result, 4);

		//Our method
		if (t.sub_type & TASK_SUBTYPE_ANIM_OURS)
		{
			tm.type		= TASK_TYPE_RENDER;
			tm.matProj	= t_org->matProj;
			tm.matView	= matView;
			tm.r.blocks = t.blocks;
			tm.r.details= t.details;
			tm.r.result	= t.result;
			tm.r.vis_pixel_size
						= t.vis_pixel_size;
			tm.r.fr_Avis= t.fr_Avis;
			tm.r.fr_vis	= t.fr_vis;

			tm.r.p_shadow		= t.p_shadow;
			tm.r.p_normal		= t.p_normal;
			tm.r.p_tangent		= t.p_tangent;
			tm.r.p_uv			= t.p_uv;
			tm.r.p_vis_buffer	= t.p_vis_buffer;
			tm.r.p_wo			= t.p_wo;
			tm.r.dbg_pixel_x	= -1;
			tm.r.dbg_pixel_y	= -1;

			tm.r.v_lookat	= &v_lookat;
			tm.r.v_up		= &v_up;
			tm.r.v_right	= &v_right;

			qrender_block b;
			b.x0 = b.y0 = 0;
			b.x1 = t_org->width;
			b.y1 = t_org->height;
			work_task_render_block(&tm, &b);
			
			sprintf_s(filename, "%sours_%04d.jpg", PREFIX, frame);
			
			save_image(filename, t.result, t_org->width, t_org->height);

		}

		//Ground-truth
		if (t.sub_type & TASK_SUBTYPE_ANIM_TRUTH)
		{
			tm.type		= TASK_TYPE_GROUND_TRUTH;
			tm.matProj	= t_org->matProj;
			tm.matView	= matView;
			tm.trt.blocks	= t.blocks;
			tm.trt.details	= t.details;
			tm.trt.result	= t.result;
			tm.trt.vis_pixel_size
							= t.vis_pixel_size;

			tm.trt.p_shadow		= t.p_shadow;
			tm.trt.p_normal		= t.p_normal;
			tm.trt.p_tangent	= t.p_tangent;
			tm.trt.p_uv			= t.p_uv;
			tm.trt.p_vis_buffer	= t.p_vis_buffer;
			tm.trt.p_wo			= t.p_wo;
			tm.trt.dbg_pixel_x	= -1;
			tm.trt.dbg_pixel_y	= -1;

			tm.trt.v_lookat	= &v_lookat;
			tm.trt.v_up		= &v_up;
			work_task_ground_truth(&tm);

			sprintf_s(filename, "%struth_%04d.jpg", PREFIX, frame);
			save_image(filename, t.result, t_org->width, t_org->height);
		}
	}
}
