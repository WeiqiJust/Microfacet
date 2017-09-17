#include "microfacet.h"
#include "worker.h"
#include "kd_tri_intersect.h"

#define GROUND_TRUTH_SCENE_R	1.73

void init_raytracing(kd_tri_intersect* &pisa,
					 kd_tri_intersect* &pisa_all,
					 tri_mesh* &pobj, 
					 tri_mesh* &pobj_all,
					 task_render_truth &t,
					 Vector3 &center,
					 const int block_idx)
{
	int block_x, block_y;
	t.details->compute_back_idx(block_x, block_y, block_idx);
	center = Vector3(block_x+0.5f, block_y+0.5f, 0);

	pobj		= new tri_mesh();
	pobj_all	= new tri_mesh();

	microfacet_block &block = (*t.blocks)[block_idx];
	vector<Triangle*> triangle_self, triangle_all;
	for (int k = 0; k < block.neighbors_and_self.size(); k++)
	{
		microfacet_block &b = *block.neighbors_and_self[k];
		for (int i = 0; i < b.per_mat_groups.insts.size(); i++) // iter all the materials
			for (int j = 0; j < b.per_mat_groups.insts[i].size(); j++) // iter each r_instance group for a material
			{
				tri_mesh_d3dx11* p = b.per_mat_groups.insts[i][j]->get_geom()->get_mesh();

				//tri_mesh q = *p;
				const Matrix4 &mat = b.per_mat_groups.insts[i][j]->get_shader_input()->matWorld; //get the transformation for geometry
				Matrix4 mat_normal = mat;
				mat_normal.m[0][3] = mat_normal.m[1][3] = mat_normal.m[2][3] = 0;

				//printf_s("----\n");
				//printf_s("%g %g %g %g\n", mat._11, mat._12, mat._13, mat._14);
				//printf_s("%g %g %g %g\n", mat._21, mat._22, mat._23, mat._24);
				//printf_s("%g %g %g %g\n", mat._31, mat._32, mat._33, mat._34);
				//printf_s("%g %g %g %g\n", mat._41, mat._42, mat._43, mat._44);
				//printf_s("(%g %g %g) ", q.position(0).x, q.position(0).y, q.position(0).z);

				for (int v = 0; v < p->get_vertex_number(); v++)
				{
					p->vertices[v] = mat*p->vertices[v];
					p->normals[v] = mat_normal*p->normals[v];
					p->normals[v].normalize();
				}

				for (int f = 0; f < p->get_face_number(); f++)
				{
					Triangle* tri = new Triangle();
					tri->vertex[0] = p->vertices[p->faces[f].v1];
					tri->vertex[1] = p->vertices[p->faces[f].v2];
					tri->vertex[2] = p->vertices[p->faces[f].v3];
					tri->face_index = f;
					tri->material_index = i;
					if (k == 0)
						triangle_self.push_back(tri);
					triangle_all.push_back(tri);
				}
				//printf_s("(%g %g %g)\n", q.position(0).x, q.position(0).y, q.position(0).z);
				///if (k == 0)
				///	pobj->add_sub_base_mesh(i, q); // the block itself
				///pobj_all->add_sub_base_mesh(i, q); // blocks including itself and neighboors
			}
	}

	///pobj->compute_face_normal();
	///pobj_all->compute_face_normal();

	///pisa = new kd_tri_intersect(*pobj, 0.001, 6, intersection_accelerator::search_front);
	///pisa_all = new kd_tri_intersect(*pobj_all, 0.001, 6, intersection_accelerator::search_front);
	pisa = new kd_tri_intersect();
	pisa = pisa->build(triangle_self, 6);
	pisa_all = new kd_tri_intersect();
	pisa_all = pisa_all->build(triangle_all, 6);
}

void raytrace(Vector3 &result,
			  const intersection_result &ir,
			  const microfacet_block &block,
			  const kd_tri_intersect* pisa_all,
			  const tri_mesh* pobj_all,
			  const std::vector<r_light_dir> &lights,
			  const Vector3 &wo,
			  const int num_indirect_rays,
			  const int num_level,
			  const int max_level,
			  random &rng,
			  bool b_debug
			  //const Number *rnd
			  )
{
	//direct illumination
	///int pid = block.per_mat_groups.get_id(ir.local_geom.group);
	int pid = block.per_mat_groups.get_id(ir.triangle->material_index);
	matr* p_matr = mff_singleton::get()->get_matr(pid);
	preconv_matr *basis_matr = mff_singleton::get()->get_basis_matr(p_matr->basis_id);

	result = Vector3(0, 0, 0);

	Vector3 tangentd, binormald;
	///codex::graphics::utils::build_frame(tangentd, binormald, ir.d_geom.normal);
	///Vector3 n(ir.d_geom.normal.x, ir.d_geom.normal.y, ir.d_geom.normal.z),
	///		tangent(tangentd.x, tangentd.y, tangentd.z),
	///		binormal(binormald.x, binormald.y, binormald.z);
	ir.triangle->calculate_normal();
	build_frame(tangentd, binormald, ir.triangle->normal);
	Vector3 n(ir.triangle->normal.x, ir.triangle->normal.y, ir.triangle->normal.z),
			tangent(tangentd.x, tangentd.y, tangentd.z),
			binormal(binormald.x, binormald.y, binormald.z);
	intersection_result sir;

	if (num_level > 0)
	for (int i = 0; i < lights.size(); i++)
	{
		const Vector3 &wi = lights[i].dir;

		
		///ry.origin = ir.d_geom.p;
		Vector3 origin = ir.hit_point;
		Vector3 direction;
		direction.x = wi.x;
		direction.y = wi.y;
		direction.z = wi.z;
		Ray ry(origin, direction);
		if (!pisa_all->hit(pisa_all, ry, sir))
		{
			Vector3 refl;
			basis_matr->pBRDF->sample(refl, wi, wo, n);
			if (b_debug)
				printf_s("n(%g %g %g)\nbrdf(%g %g %g\nalbedo(%g %g %g)\nn*wo(%g)\nlight(%g %g %g)\n", 
					n.x, n.y, n.z,
					refl.x, refl.y, refl.z, p_matr->albedo.x, p_matr->albedo.y, p_matr->albedo.z, n*wo,
					lights[i].c.x, lights[i].c.y, lights[i].c.z);
			//if (b_debug)
			//	printf_s("[%d-%d] brdf(%g %g %g) n*wo(%g)", num_level, i, refl.x, refl.y, refl.z, n*wo);
			refl.x *= p_matr->albedo.x;
			refl.y *= p_matr->albedo.y;
			refl.z *= p_matr->albedo.z;
			refl *= basis_matr->inv_avg/(n*wo);
			Vector3 c;
			c.x = refl.x*lights[i].c.x;
			c.y = refl.y*lights[i].c.y;
			c.z = refl.z*lights[i].c.z;
			//if (b_debug)
			//	printf_s(" light(%g %g %g)\n", lights[i].c.x, lights[i].c.y, lights[i].c.z);
			if (b_debug)
				printf_s("final(%g %g %g)\n", c.x, c.y, c.z);
			result += c;
		}
	}

	//add up indirect illumination
	if (num_level < max_level-1)
	{
		//codex::math::prob::uniform_hemi_direction_3d<Number> pr;

		int num_path = (num_level == 0) ? num_indirect_rays : 1;
		for (int i = 0; i < num_path; i++)
		{
			//Vector3 samp;
			//Number	pdf;
			//pr.sample(samp, pdf, Vector3(rng.rand_real(), rng.rand_real(), 0.0));

			Vector3 samp;
			float pdf;
			basis_matr->p_sampler->sample_dir(samp, pdf, Vector3(wo*tangent, wo*binormal, wo*n), rng);

			if (pdf > 1e-7)
			{
				
				Vector3 origin = ir.hit_point;
				//codex::math::prob::transform_hemi_direction_3d(hemi_ray.direction, ir.d_geom.normal, samp);
				Vector3 direction = samp.x*tangentd + samp.y*binormald + samp.z*ir.triangle->normal;
				Ray hemi_ray(origin, direction);
				if (pisa_all->hit(pisa_all, hemi_ray, sir))
				{
					Vector3 c, indirect_wi(hemi_ray.direction.x, hemi_ray.direction.y, hemi_ray.direction.z);
					///sir.local_geom.group = pobj_all->face2group(sir.local_geom.index);
					sir.triangle->calculate_normal();
					if (b_debug)
						printf_s("[%d] p(%g %g %g) n(%g %g %g)\n", num_level, 
						sir.hit_point.x, sir.hit_point.y, sir.hit_point.z,
						sir.triangle->normal.x, sir.triangle->normal.y, sir.triangle->normal.z);
					raytrace(c, sir, block, pisa_all, pobj_all, lights, -indirect_wi, 
						num_indirect_rays, num_level+1, max_level, rng, b_debug);
					if (b_debug)
						printf_s("[%d] c(%g %g %g) : ", num_level, 
							c.x, c.y, c.z);

					Vector3 refl;
					basis_matr->pBRDF->sample(refl, indirect_wi, wo, n);
					refl.x *= p_matr->albedo.x;
					refl.y *= p_matr->albedo.y;
					refl.z *= p_matr->albedo.z;
					refl *= basis_matr->inv_avg/(n*wo);
					c.x *= refl.x;
					c.y *= refl.y;
					c.z *= refl.z;
					if (b_debug)
						printf_s("brdf(%g %g %g) : pdf(%g)\n", refl.x, refl.y, refl.z, pdf);
					result += c / (pdf*num_path);
				}
			}
		}
	}
}

void raytrace_pixel(Vector3 &result,
					const int block_idx,
					const int dim,
					const int num_raytrace_level,
					const int num_rays,
					task_render_truth &t,
					const Vector3 &center,
					const Vector3 &tangent,
					const Vector3 &binormal,
					const Vector3 &normal,
					const Vector3 &global_wo,
					const std::vector<r_light_dir> &global_lights,
					const std::vector<bool> &vis,
					const kd_tri_intersect* pisa,
					const kd_tri_intersect* pisa_all,
					const tri_mesh* pobj_all,
					random &rng,
					//const Number *rnd,
					bool b_debug)
{	
	int block_x, block_y;
	t.details->compute_back_idx(block_x, block_y, block_idx);
	microfacet_block &block = (*t.blocks)[block_idx];

	Vector3 wo(global_wo*tangent, global_wo*binormal, global_wo*normal);
	if (b_debug)
	{
		///codex::math::utils::random_number_generator_state state;
		///rng.save_state(state);
		//printf_s("rng = %g\n", rng.rand_real());
		///rng.load_state(state);
		printf_s("wo (%g %g %g)\n", wo.x, wo.y, wo.z);
	}

	std::vector<r_light_dir> lights;
	for (int i = 0; i < global_lights.size(); i++)
		if (vis[i])
		{
			r_light_dir lt;
			Vector3 dir = global_lights[i].dir;
			lt.dir	= Vector3(dir*tangent, dir*binormal, dir*normal);
			lt.c	= global_lights[i].c;
			lights.push_back(lt);
		}	

	Vector3 v_right, v_up;
	v_right = (-wo) ^ Vector3(0, 1, 0);
	v_right.normalize();
	v_up = v_right ^ (-wo);
	v_up.normalize();

	std::vector<float> img_dbg;
	img_dbg.resize(dim*dim*3, 0.5f);

	int num = 0;
	for (int y = 0; y < dim; y++)
		for (int x = 0; x < dim; x++)
		{
			
			Vector3 origin	= Vector3(center.x, center.y, center.z) 
							+ GROUND_TRUTH_SCENE_R*((x + rng.get_random_float())*2.0f/dim-1.0f)/2.0f*v_right 
							- GROUND_TRUTH_SCENE_R*((y + rng.get_random_float())*2.0f / dim - 1.0f) / 2.0f * v_up
							+ wo*3;
			Vector3 direction = -wo;
			Ray ry(origin, direction);
			intersection_result ir, ir_all;

			if (pisa->hit(pisa, ry, ir) &&
				pisa_all->hit(pisa_all, ry, ir_all) &&
				abs(ir.distance - ir_all.distance) < 1e-6)
			{
				///ir.local_geom.group = pobj_all->face2group(ir_all.local_geom.index);
				
				if (b_debug && (x==18 && y==15))
					printf_s("[%d %d]\n", x, y);

				Vector3 r;
				raytrace(r, ir, block, pisa_all, pobj_all, lights, Vector3(wo.x, wo.y, wo.z), num_rays, 0, 
					num_raytrace_level,	rng, (x==56 && y==51) ? b_debug : false
					//rnd
					);
				result += r;
				num++;

				if (b_debug)
				{
					img_dbg[(x+y*dim)*3+0] = r.x;
					img_dbg[(x+y*dim)*3+1] = r.y;
					img_dbg[(x+y*dim)*3+2] = r.z;
				}
				//img_dbg[(x+y*dim)*3+0] = (ir.d_geom.normal.x+1)/2;
				//img_dbg[(x+y*dim)*3+1] = (ir.d_geom.normal.y+1)/2;
				//img_dbg[(x+y*dim)*3+2] = (ir.d_geom.normal.z+1)/2;
			}
		}

	result /= num;
	
	if (b_debug)
	{
		save_image_color("d:/temp/Microfacet/debug_pixel_rt.png", img_dbg, dim, dim);
		printf_s("result(lighted) : %g %g %g\n", result.x, result.y, result.z);
	}
}

void worker_microfacet::work_task_debug_raytrace_pixel(task_microfacet *t_org)
{
	task_render_truth &t = t_org->trt;
	Vector3 v_right = (*t.v_up) ^ (*t.v_lookat);
	v_right.normalize();

	std::vector<r_light_dir> lights;
	lights = t.p_shadow->lights;

	int x = t.dbg_pixel_x;
	int y = t.dbg_pixel_y;
	int	idx = x+y*t_org->width;

	if (t.p_uv[idx*2] >= 0)
	{
		random rng;

		//{
		//	char str[256];
		//	sprintf_s(str, "D:/temp/Microfacet/temp/%d.rng", idx);
		//	codex::math::utils::random_number_generator_state rng_state;
		//	FILE *fp;
		//	FOPEN(fp, str, "rb");
		//	fread(&rng_state, sizeof(rng_state), 1, fp);
		//	fclose(fp);

		//	rng.load_state(rng_state);
		//}

		Vector3 normal, tangent, binormal, global_wo;

		normal.x	= snorm2float(t.p_normal[idx*4]);
		normal.y	= snorm2float(t.p_normal[idx*4+1]);
		normal.z	= snorm2float(t.p_normal[idx*4+2]);
		tangent.x	= snorm2float(t.p_tangent[idx*4]);
		tangent.y	= snorm2float(t.p_tangent[idx*4+1]);
		tangent.z	= snorm2float(t.p_tangent[idx*4+2]);

		binormal	= normal ^ tangent;
		global_wo	=	(*t.p_wo)[idx].x*v_right +
						(*t.p_wo)[idx].y*(*t.v_up) +
						(*t.p_wo)[idx].z*(*t.v_lookat);
		//DEBUG
		//printf_s("gwo %g %g %g\n", global_wo.x, global_wo.y, global_wo.z);
		int idx_block = t.details->compute_idx(0, 0);

		Vector3 center;
		kd_tri_intersect *pisa, *pisa_all;
		tri_mesh *pobj, *pobj_all;
		init_raytracing(pisa, pisa_all, pobj, pobj_all, t, center, idx_block);

		std::vector<bool> vis_wi;
		for (int batch = 0; batch < t.vis_pixel_size; batch++)
		{
			int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
			for (int l = 0; l < min(t.p_shadow->lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
				if (mask & (1<<l))
				{
					vis_wi.push_back(true);
				} else {
					vis_wi.push_back(false);
				}
		}

		Vector3 c;
		raytrace_pixel(c, idx_block, //DIM_GROUND_TRUTH, 
			t.dim_raytrace, t.num_raytrace_level, t.num_rays,
			t, center, tangent, binormal, normal, 
			global_wo, lights, vis_wi, pisa, pisa_all, pobj_all, rng, true);

		BYTE r, g, b;
		r = min(max(c.x, 0), 1)*255;
		g = min(max(c.y, 0), 1)*255;
		b = min(max(c.z, 0), 1)*255;
		UINT pixel = (r << 16) + (g << 8) + (b);
		t.result[idx] = pixel;
	}
}

void worker_microfacet::work_task_raytrace_block(task_microfacet *t_org, qrender_block *pb)
{
	task_render_truth &t = t_org->trt;
	int dim = t.dim_raytrace;
	Vector3 v_right = (*t.v_up) ^ (*t.v_lookat);
	v_right.normalize();

	std::vector<r_light_dir> lights;
	lights = t.p_shadow->lights;

	bool b_nonempty = false;
	for (int y = pb->y0; y < pb->y1; y++)
		for (int x = pb->x0; x < pb->x1; x++)
		{
			int	idx = x+y*t_org->width;
			if (t.p_uv[idx*2] >= 0)
			{
				b_nonempty = true;
				break;
			}
		}

	if (!b_nonempty) return;

	int idx_block = t.details->compute_idx(0, 0);
	Vector3 center;
	kd_tri_intersect *pisa, *pisa_all;
	tri_mesh *pobj, *pobj_all;
	init_raytracing(pisa, pisa_all, pobj, pobj_all, t, center, idx_block);

	//codex::math::utils::Halton_Zaremba_seq<Number> seq;
	//std::vector<Number> rnd;
	//rnd.resize(dim*dim*t.num_raytrace_level*t.num_rays*2);
	//for (int i = 0; i < rnd.size()/2; i++)
	//{	
	//	vector2 samp;
	//	seq.get_sample(samp);
	//	rnd[i*2+0] = samp.x;
	//	rnd[i*2+1] = samp.y;
	//}
	random rng;

	for (int y = pb->y0; y < pb->y1; y++)
		for (int x = pb->x0; x < pb->x1; x++)
		{
			int	idx = x+y*t_org->width;
			if (t.p_uv[idx*2] >= 0)
			{
				{
					//char str[256];
					//sprintf_s(str, "D:/temp/Microfacet/temp/%d.rng", idx);
					//codex::math::utils::random_number_generator_state rng_state;
					//rng.save_state(rng_state);

					//FILE *fp;
					//FOPEN(fp, str, "wb");
					//fwrite(&rng_state, sizeof(rng_state), 1, fp);
					//fclose(fp);
				}

				//DEBUG
				//random_number_generator<Number> rng;
				Vector3 normal, tangent, binormal, global_wo;

				normal.x	= snorm2float(t.p_normal[idx*4]);
				normal.y	= snorm2float(t.p_normal[idx*4+1]);
				normal.z	= snorm2float(t.p_normal[idx*4+2]);
				tangent.x	= snorm2float(t.p_tangent[idx*4]);
				tangent.y	= snorm2float(t.p_tangent[idx*4+1]);
				tangent.z	= snorm2float(t.p_tangent[idx*4+2]);

				binormal	= normal ^ tangent;
				global_wo	=	(*t.p_wo)[idx].x*v_right +
					(*t.p_wo)[idx].y*(*t.v_up) +
					(*t.p_wo)[idx].z*(*t.v_lookat);
				//DEBUG
				//printf_s("gwo %g %g %g\n", global_wo.x, global_wo.y, global_wo.z);

				std::vector<bool> vis_wi;
				for (int batch = 0; batch < t.vis_pixel_size; batch++)
				{
					int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
					for (int l = 0; l < min(t.p_shadow->lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
						if (mask & (1<<l))
						{
							vis_wi.push_back(true);
						} else {
							vis_wi.push_back(false);
						}
				}

				Vector3 c;
				raytrace_pixel(c, idx_block, dim, t.num_raytrace_level, t.num_rays,
					t, center, tangent, binormal, normal, 
					global_wo, lights, vis_wi, pisa, pisa_all, pobj_all, 
					rng, 
					//(x == 190 && y == 216) ? true :
					false);

				//if (c.x > 1 || c.y > 1 || c.z > 1)
				//	printf_s("%d %d = (%g %g %g)\n", x, y, c.x, c.y, c.z);

				val[idx] = c;
				//BYTE r, g, b;
				//r = min(max(c.x, 0), 1)*255;
				//g = min(max(c.y, 0), 1)*255;
				//b = min(max(c.z, 0), 1)*255;
				//UINT pixel = (r << 16) + (g << 8) + (b);
				//t.result[idx] = pixel;
			}
		}

	SAFE_DELETE(pisa);
	SAFE_DELETE(pisa_all);
	SAFE_DELETE(pobj);
	SAFE_DELETE(pobj_all);
}

extern float gen_pixel(Vector3 &result,
					   task_render_truth &t,
					   const Vector3 &tangent,
					   const Vector3 &binormal,
					   const Vector3 &normal,
					   const Vector3 &global_wo,
					   const int block_idx,
					   r_shadowmap *p_shadow,
					   render_output *p_screen,
					   render_target *p_target_id,
					   float *p_id,
					   render_target *p_target_normal,
					   short *p_normal,
					   render_target *p_target_vis,
					   float *p_vis,
					   const std::vector<r_light_dir> &global_lights,
					   const std::vector<bool> vis_wi,
					   bool b_debug);

void worker_microfacet::work_task_raytrace_direct(task_microfacet *t_org)
{
	task_render_truth &t = t_org->trt;
	Vector3 v_right = (*t.v_up) ^ (*t.v_lookat);
	v_right.normalize();

	short *p_normal;
	float *p_vis, *p_id;
	p_normal	= new short[DIM_GROUND_TRUTH*DIM_GROUND_TRUTH*4];
	p_vis		= new float[DIM_GROUND_TRUTH*DIM_GROUND_TRUTH];
	p_id		= new float[DIM_GROUND_TRUTH*DIM_GROUND_TRUTH];

	render_target	*p_target_normal,
					*p_target_vis,
					*p_target_id;
	p_target_normal = new render_target(mff_singleton::get()->get_handle());
	p_target_normal->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1, 1, DXGI_FORMAT_R16G16B16A16_SNORM, R_TARGET_CPU_READ);
	p_target_vis = new render_target(mff_singleton::get()->get_handle());
	p_target_vis->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1, 1, DXGI_FORMAT_R32_FLOAT, R_TARGET_CPU_READ);
	p_target_id = new render_target(mff_singleton::get()->get_handle());
	p_target_id->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1, 1, DXGI_FORMAT_R32_FLOAT, R_TARGET_CPU_READ);

	render_output *p_screen;
	p_screen = new render_output(mff_singleton::get()->get_handle());
	p_screen->init(DIM_GROUND_TRUTH, DIM_GROUND_TRUTH, 1);

	r_shadowmap shadowmap;
	shadowmap.init(mff_singleton::get()->get_handle(), mff_singleton::get()->get_sampler("shadow"), 
		1.0f, SHADOW_MAP_SIZE, DXGI_FORMAT_R32_FLOAT, GROUND_TRUTH_SCENE_R);

	std::vector<r_light_dir> lights;
	lights = t.p_shadow->lights;
	for (int y = 0; y < t_org->height; y++)
		for (int x = 0; x < t_org->width; x++)
		{
			int		idx = x+y*t_org->width;
			if (t.p_uv[idx*2] >= 0)
			{
				printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
				printf_s("(%d/%d) ", x+y*t_org->width+1, t_org->width*t_org->height);

				Vector3 normal, tangent, binormal, global_wo;

				normal.x	= snorm2float(t.p_normal[idx*4]);
				normal.y	= snorm2float(t.p_normal[idx*4+1]);
				normal.z	= snorm2float(t.p_normal[idx*4+2]);
				tangent.x	= snorm2float(t.p_tangent[idx*4]);
				tangent.y	= snorm2float(t.p_tangent[idx*4+1]);
				tangent.z	= snorm2float(t.p_tangent[idx*4+2]);

				binormal	= normal ^ tangent;
				global_wo	=	(*t.p_wo)[idx].x*v_right +
					(*t.p_wo)[idx].y*(*t.v_up) +
					(*t.p_wo)[idx].z*(*t.v_lookat);

				//FIX ME: huge bug here!
				int idx_block = t.details->compute_idx(0, 0);

				std::vector<bool> vis_wi;

				for (int batch = 0; batch < t.vis_pixel_size; batch++)
				{
					int mask = (int)t.p_vis_buffer[t_org->width*t_org->height*batch+idx];
					for (int l = 0; l < min(t.p_shadow->lights.size()-batch*NUM_BITS_VIS, NUM_BITS_VIS); l++)
						if (mask & (1<<l))
						{
							vis_wi.push_back(true);
						} else {
							vis_wi.push_back(false);
						}
				}

				Vector3 c;
				float area = gen_pixel(c, t, tangent, binormal, normal, global_wo, idx_block, 
					&shadowmap, p_screen, p_target_id, p_id, p_target_normal, p_normal, p_target_vis, p_vis, lights, vis_wi,
					false);
				c += val[idx];
				//c = val[idx];

				BYTE r, g, b;
				r = min(max(c.x, 0), 1)*255;
				g = min(max(c.y, 0), 1)*255;
				b = min(max(c.z, 0), 1)*255;
				UINT pixel = (r << 16) + (g << 8) + (b);
				t.result[idx] = pixel;
			}
		}
	printf_s("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	printf_s("done\n");

	SAFE_DELETE_ARR(p_normal);
	SAFE_DELETE_ARR(p_vis);
	SAFE_DELETE_ARR(p_id);
	SAFE_DELETE(p_target_normal);
	SAFE_DELETE(p_target_vis);
	SAFE_DELETE(p_target_id);
	SAFE_DELETE(p_screen);
}
