#include "microfacet_factory.h"
#include "r_shader_basic.h"
#include "r_shader_vispoint.h"
#include "r_shader_area.h"
#include "r_shader_vismap.h"
#include "instance_property.h"
#include "r_states.h"

//#define LOAD_ALL_MATR
microfacet_factory::microfacet_factory(D3D_dev_handle *pdev)
: pdev(pdev)
{
	//codex::utils::timer t;

	//Lambertian
	add_basis_matr(BASIS_MATR_LAMBERTIAN, MATR_PATH"Lambert.txt");

	//Cook-Torrance
	//add_basis_matr(BASIS_MATR_CT_0_20_0_25, MATR_PATH "CT_0.20.txt");
	//add_basis_matr(BASIS_MATR_CT_0_30_0_25, MATR_PATH "CT_0.30.txt");
	//add_basis_matr(BASIS_MATR_CT_0_40_0_25, MATR_PATH "CT_0.40.txt");
	//add_basis_matr(BASIS_MATR_CT_0_50_0_25, MATR_PATH "CT_0.50.txt");
	//Ward
	add_basis_matr(BASIS_MATR_WARD_0_20, MATR_PATH "Ward_0.20.txt");

	add_basis_matr(BASIS_MATR_WARD_0_30, MATR_PATH"Ward_0.30.txt");

	//add_basis_matr(BASIS_MATR_WARD_0_40, MATR_PATH "Ward_0.40.txt");
	//add_basis_matr(BASIS_MATR_WARD_0_50, MATR_PATH "Ward_0.50.txt");
	////BlinnPhong
	//add_basis_matr(BASIS_MATR_PHONG_2, MATR_PATH "Phong_2.txt");
	//add_basis_matr(BASIS_MATR_PHONG_4, MATR_PATH "Phong_4.txt");
	//add_basis_matr(BASIS_MATR_PHONG_8, MATR_PATH "Phong_8.txt");
	//add_basis_matr(BASIS_MATR_PHONG_16, MATR_PATH "Phong_16.txt");
	//add_basis_matr(BASIS_MATR_PHONG_32, MATR_PATH "Phong_32.txt");

	/*
	//gold-metallic-paint
	add_basis_matr(BASIS_MATR_GOLD_METALLIC, MATR_PATH "gold-metallic-paint.txt");
	//pearl-paint
	add_basis_matr(BASIS_MATR_PEARL_PAINT, MATR_PATH "pearl-paint.txt");
	//dark-red-paint
	//add_basis_matr(BASIS_MATR_DARK_RED_PAINT, MATR_PATH "dark-red-paint.txt");
	//silver-metallic-paint	
	add_basis_matr(BASIS_MATR_SILVER_METALLIC, MATR_PATH "silver-metallic-paint.txt");
	//silver-metallic-paint2	
	add_basis_matr(BASIS_MATR_SILVER_METALLIC2, MATR_PATH "silver-metallic-paint2.txt");
	//silver-paint
	add_basis_matr(BASIS_MATR_SILVER_PAINT, MATR_PATH "silver-paint.txt");
	//blue-metallic-paint
	add_basis_matr(BASIS_MATR_BLUE_METALLIC, MATR_PATH "blue-metallic-paint.txt");
	//special-walnut-224_51_41_98
	add_basis_matr(BASIS_MATR_WALNUT_224, MATR_PATH "special-walnut-224.txt");
	//white-diffuse-bball
	add_basis_matr(BASIS_MATR_WHITE_DIFFUSE, MATR_PATH "white-diffuse-bball.txt");
	//white-fabric2
	add_basis_matr(BASIS_MATR_WHITE_FABRIC2, MATR_PATH "white-fabric2.txt");
	//pickled-oak-260_34_22_98
	add_basis_matr(BASIS_MATR_OAK_260, MATR_PATH "pickled-oak-260.txt");
	//colonial-maple-223_30_34_98
	add_basis_matr(BASIS_MATR_MAPLE_223, MATR_PATH "colonial-maple-223.txt");
	*/
	//t.update();
	//printf_s("\n total loading time = %g secs\n", t.elapsed_time());
	printf_s("\nFinish loading material. \n");
	//material;
	lib_matr_id.add("matr_lambertian_0",	MATR_LAMBERTIAN_0);
	lib_matr_id.add("matr_sphere_0",		MATR_SPHERE_0);
	lib_matr_id.add("matr_rod_0",			MATR_ROD_0);
	lib_matr_id.add("matr_plane_0",			MATR_PLANE_0);

	lib_matr_id.add("matr_distr_0",			MATR_DISTR_0);
	lib_matr_id.add("matr_distr_1",			MATR_DISTR_1);
	lib_matr_id.add("matr_distr_2",			MATR_DISTR_2);
	lib_matr_id.add("matr_distr_3",			MATR_DISTR_3);

	lib_matr_id.add("matr_binder_0",		MATR_BINDER_0);
	lib_matr_id.add("matr_binder_1",		MATR_BINDER_1);
	lib_matr_id.add("matr_binder_2",		MATR_BINDER_2);
	lib_matr_id.add("matr_binder_3",		MATR_BINDER_3);

	//shaders
	{
		FILE *fp;

		fp = fopen(SHADER_PATH "shadow_config.txt", "wt");
		fprintf_s(fp, "#define SHADOW_MAP_SIZE %d\n", SHADOW_MAP_SIZE);
		fclose(fp);

		fp = fopen(SHADER_PATH "batch_area_sampler_config.txt", "wt");
		fprintf_s(fp, "#define MAX_TARGETS %d\n", NUM_AREA_DIR_PER_BATCH);
		fclose(fp);
	}

	ID3DBlob		*pVSBlobBasic, *pVSBlobTangent, *pVSBlobVisPoint, *pVSBlobVisSampler, *pVSBlobTest;
	r_shader		*sh;

	sh = new r_shader_basic;
	create_shader_from_file(sh, SHADER_PATH "diffuse.fx",
		"VS", "vs_5_0", "", "", "PS", "ps_5_0", pdev, &pVSBlobBasic);
	lib_shader.add("diffuse", sh);

	sh = new r_shader_basic;
	create_shader_from_file(sh, SHADER_PATH "diffuse.fx", 
		"VS", "vs_5_0", "", "", "PS_fence", "ps_5_0", pdev, &pVSBlobBasic);
	lib_shader.add("vis_fence", sh);

	sh = new r_shader_basic;
	create_shader_from_file(sh, SHADER_PATH "diffuse.fx", 
		"VS", "vs_5_0", "", "", "PS_constant", "ps_5_0", pdev, &pVSBlobBasic);
	lib_shader.add("vis_disc", sh);

	sh = new r_shader_basic;
	create_shader_from_file(sh, SHADER_PATH "shadowmap.fx", 
		"VS", "vs_5_0", "", "", "PS", "ps_5_0", pdev, NULL);
	lib_shader.add("shadow", sh);

	sh = new r_shader_basic;
	create_shader_from_file(sh, SHADER_PATH "initial_render.fx", 
		"VSContext", "vs_5_0", "", "", "PSContext", "ps_5_0", pdev, &pVSBlobTangent);
	lib_shader.add("init_context", sh);

	/******test code********/
	sh = new r_shader_basic;
	create_shader_from_file_seperate(sh, SHADER_PATH "VertexShader.hlsl", SHADER_PATH "PixelShader.hlsl", "",
		"main", "vs_5_0", "", "", "main", "ps_5_0", pdev, &pVSBlobTest);
	lib_shader.add("test_shader", sh);
	/******test code********/

	sh = new r_shader_basic;
	create_shader_from_file(sh, SHADER_PATH "initial_render.fx", 
		"VSContext", "vs_5_0", "", "", "PSContextBackground", "ps_5_0", pdev, NULL);
	lib_shader.add("init_back", sh);

	sh = new r_shader_vis;
	create_shader_from_file(sh, SHADER_PATH "initial_render.fx", 
		"VSShadow", "vs_5_0", "", "", "PSShadow", "ps_5_0", pdev, NULL);
	lib_shader.add("init_shadow", sh);

	sh = new r_shader_vis;
	create_shader_from_file(sh, SHADER_PATH "ground_truth.fx", 
		"VS", "vs_5_0", "", "", "PS", "ps_5_0", pdev, NULL);
	lib_shader.add("ground_truth", sh);

	//{
	//	FILE *fp;
	//	FOPEN(fp, "E:/Code/Research/Microfacet/MicrofacetEditor/vis_point_config.txt", "wt");
	//	fprintf_s(fp, "#define VIS_SHADOW_MAP_SIZE %d\n", VIS_POINT_SHADOW_DIM);
	//	fprintf_s(fp, "#define NUM_VIS_SAMPLES %d\n", NUM_PREVIS_DIR);
	//	fclose(fp);
	//}
	//sh = new r_shader_vis_point(NUM_PREVIS_DIR);
	//create_shader_from_file(sh, "E:/Code/Research/Microfacet/MicrofacetEditor/vis_point.fx", 
	//	"VS", "vs_5_0", "", "", "PS", "ps_5_0", pdev, &pVSBlobVisPoint);
	//lib_shader.add("vis_point", sh);

	{
		FILE *fp;
		fp = fopen(SHADER_PATH "vis_point_config.txt", "wt");
		fprintf_s(fp, "#define VIS_SHADOW_MAP_SIZE %d\n", VIS_POINT_SHADOW_DIM);
		fprintf_s(fp, "#define NUM_VIS_SAMPLES %d\n", NUM_VIS_PER_BATCH);
		fclose(fp);
	}

	parab_frame	fr_vis;
	fr_vis.init(DIM_VIS, 256, DEFAULT_AREA_SAMPLES);
	sh = new r_shader_vis_point(fr_vis.get_n().size());
	//sh = new r_shader_vis_point(NUM_VIS_PER_BATCH);
	create_shader_from_file(sh, SHADER_PATH "vis_point.fx", 
		"VS", "vs_5_0", "", "", "PSMask", "ps_5_0", pdev, &pVSBlobVisPoint);
	lib_shader.add("vis_point_mask", sh);

	sh = new r_shader_area;
	create_shader_from_file(sh, SHADER_PATH "batch_area_sampler.fx", 
		"VS", "vs_5_0", "GS", "gs_5_0", "PS", "ps_5_0", pdev, &pVSBlobVisSampler);
	lib_shader.add("area", sh);

	//DEBUG
	{
		sh = new r_shader_vismap;
		create_shader_from_file(sh, SHADER_PATH "vis_sampler.fx", 
			"VS", "vs_5_0", "GS", "gs_5_0", "PS", "ps_5_0", pdev, NULL);
		lib_shader.add("vis_sampler", sh);

		sh = new r_shader_basic;
		create_shader_from_file(sh, SHADER_PATH "area_sampler.fx", 
			"VS", "vs_5_0", "", "", "PS", "ps_5_0", pdev, NULL);
		lib_shader.add("area_dbg", sh);
	}

	//input layer
	r_input_layer *p_layer;
	tri_mesh_d3dx11 mesh(pdev->get_device());
	
	mesh.set_vertex_attr(VERT_ATTR_POSITION|VERT_ATTR_NORMAL);
	p_layer = new r_input_layer(pdev);
	p_layer->init(&mesh, pVSBlobBasic->GetBufferPointer(), pVSBlobBasic->GetBufferSize());
	lib_layer.add("basic", p_layer);

	mesh.set_vertex_attr(VERT_ATTR_POSITION|VERT_ATTR_NORMAL|VERT_ATTR_TANGENT|VERT_ATTR_UV1);
	p_layer = new r_input_layer(pdev);
	p_layer->init(&mesh, pVSBlobTangent->GetBufferPointer(), pVSBlobTangent->GetBufferSize());
	lib_layer.add("init_context", p_layer);

	mesh.set_vertex_attr(VERT_ATTR_POSITION|VERT_ATTR_NORMAL|VERT_ATTR_TANGENT|VERT_ATTR_UV1);
	p_layer = new r_input_layer(pdev);
	p_layer->init(&mesh, pVSBlobBasic->GetBufferPointer(), pVSBlobBasic->GetBufferSize());
	lib_layer.add("init_shadow", p_layer);

	mesh.set_vertex_attr(VERT_ATTR_POSITION|VERT_ATTR_UV1);
	p_layer = new r_input_layer(pdev);
	p_layer->init(&mesh, pVSBlobVisPoint->GetBufferPointer(), pVSBlobVisPoint->GetBufferSize());
	lib_layer.add("vis_point", p_layer);

	mesh.set_vertex_attr(VERT_ATTR_POSITION|VERT_ATTR_NORMAL);
	p_layer = new r_input_layer(pdev);
	p_layer->init(&mesh, pVSBlobVisSampler->GetBufferPointer(), pVSBlobVisSampler->GetBufferSize());
	lib_layer.add("pos", p_layer);

	mesh.set_vertex_attr(VERT_ATTR_POSITION);
	p_layer = new r_input_layer(pdev);
	p_layer->init(&mesh, pVSBlobTest->GetBufferPointer(), pVSBlobTest->GetBufferSize());
	lib_layer.add("test_shader", p_layer);
	

	SAFE_RELEASE(pVSBlobBasic);
	SAFE_RELEASE(pVSBlobTangent);
	SAFE_RELEASE(pVSBlobVisPoint);
	SAFE_RELEASE(pVSBlobVisSampler);

	//geometries
	r_geometry *p_geom;
	//triangle_face face;

	//---------  sphere  ---------
	p_geom = new r_geometry(pdev);
	p_geom->load(DATA_PATH"sphere_low_res.obj");
	p_geom->get_mesh()->uvs.clear();
	p_geom->get_mesh()->tangents.clear();
	p_geom->get_mesh()->set_attribute(VERT_ATTR_POSITION | VERT_ATTR_NORMAL);
	//"D:/temp/Microfacet/axis.obj"
	lib_geom.add("sphere_200", shared_ptr<r_geometry>(p_geom));

	
	//---------  rod  ---------
	p_geom = new r_geometry(pdev);
	{
		Matrix4 mat;
		Translate(Vector3(0.0, 0.0, 0.5));
		p_geom->load(DATA_PATH"rod.obj", &mat);
		p_geom->get_mesh()->uvs.clear();
		p_geom->get_mesh()->tangents.clear();
		p_geom->get_mesh()->set_attribute(VERT_ATTR_POSITION | VERT_ATTR_NORMAL);
		//p_geom->get_mesh()->compute_face_normal();
		//std::vector<float> face_scalar;
		//face_scalar.resize(p_geom->get_mesh()->num_faces(), 1.0f);
		//for (int i = 0; i < face_scalar.size(); i++)
		//	if (p_geom->get_mesh()->face_normal(i).z < -0.8f)
		//	{
		//		face_scalar[i] = 3.0f;
		//	}
		//p_geom->set_face_scalar(face_scalar);
	}
	lib_geom.add("rod", shared_ptr<r_geometry>(p_geom));
	/*
	//---------  fence  ---------
	p_geom = new r_geometry(pdev);
	p_geom->load(GEOM_PATH "vis_fence.obj");
	lib_geom.add("fence", shared_ptr<r_geometry>(p_geom));
	*/
	//---------  vis_disc  ---------
	mesh.clear();
	mesh.set_vertex_attr(VERT_ATTR_POSITION|VERT_ATTR_NORMAL);
	//for (int i = 0; i <= 32; i++)
	//	mesh.add_vertex();
	mesh.vertices.push_back(Vector3(0, 0, 0));
	mesh.normals.push_back(Vector3(0, 0, 1));

	for (int i = 1; i <= 32; i++)
	{
		float theta = 2*PI*(i-1)/32;
		mesh.vertices.push_back(Vector3(cos(theta), sin(theta), 0));
		mesh.normals.push_back(Vector3(0, 0, 1));

		//face.i0 = 0;
		//face.i2 = ((i+1) % 32) + 1; 
		//face.i1 = i;
		mesh.faces.push_back(triangle_face(0, i, ((i + 1) % 32) + 1));
	}

	p_geom = new r_geometry(pdev);
	p_geom->load(&mesh, true);
	lib_geom.add("vis_disc", shared_ptr<r_geometry>(p_geom));

	//---------  vis_ring  ---------
	mesh.clear();
	mesh.set_vertex_attr(VERT_ATTR_POSITION|VERT_ATTR_NORMAL);
	//for (int i = 0; i < 32*2; i++)
		//mesh.add_vertex();
	mesh.vertices.resize(64);
	mesh.normals.resize(64);
	for (int i = 0; i < 32; i++)
	{
		float theta = 2*PI*(i-1)/32;
		mesh.vertices[i*2] = Vector3(cos(theta), sin(theta), 0);
		mesh.vertices[i * 2 + 1] = Vector3(cos(theta)*0.75, sin(theta)*0.75, 0);

		mesh.normals[i*2] = mesh.normals[i*2+1] = Vector3(0, 0, 1);

		int i0 = i*2,
			i1 = i*2+1,
			i2 = (i*2+2) % 64,
			i3 = (i*2+3) % 64;

		//face.i0 = i0;
		//face.i2 = i1;
		//face.i1 = i2; 
		mesh.faces.push_back(triangle_face(i0, i2, i1));
		//face.i0 = i2;
		//face.i2 = i1;
		//face.i1 = i3; 
		mesh.faces.push_back(triangle_face(i2, i3, i1));
	}

	p_geom = new r_geometry(pdev);
	p_geom->load(&mesh, true);
	lib_geom.add("vis_ring", shared_ptr<r_geometry>(p_geom));

	//---------  scr_quad  ---------
	mesh.clear();
	mesh.set_vertex_attr(VERT_ATTR_POSITION|VERT_ATTR_UV1);
	//for (int i = 0; i < 4; i++)
		//mesh.add_vertex();
	
	mesh.vertices.push_back(Vector3(-1, 1, 0));
	mesh.vertices.push_back(Vector3(1, 1, 0));
	mesh.vertices.push_back(Vector3(-1, -1, 0));
	mesh.vertices.push_back(Vector3(1, -1, 0));
	mesh.uvs.push_back(Vector2(0, 0));
	mesh.uvs.push_back(Vector2(1, 0));
	mesh.uvs.push_back(Vector2(0, 1));
	mesh.uvs.push_back(Vector2(1, 1));
	//face.i0 = 2; face.i1 = 1; face.i2 = 0;
	mesh.faces.push_back(triangle_face(2,1,0));
	//face.i0 = 2; face.i1 = 3; face.i2 = 1;
	mesh.faces.push_back(triangle_face(2,3,1));

	/*
	mesh.vertices.push_back(Vector3(-1, -1, 0));
	mesh.vertices.push_back(Vector3(-1, 1, 0));
	mesh.vertices.push_back(Vector3(1, -1, 0));
	mesh.vertices.push_back(Vector3(1, 1, 0));
	mesh.uvs.push_back(Vector2(0, 0));
	mesh.uvs.push_back(Vector2(0, 1));
	mesh.uvs.push_back(Vector2(1, 0));
	mesh.uvs.push_back(Vector2(1, 1));
	//face.i0 = 2; face.i1 = 1; face.i2 = 0;
	mesh.faces.push_back(triangle_face(0, 2, 1));
	//face.i0 = 2; face.i1 = 3; face.i2 = 1;
	mesh.faces.push_back(triangle_face(1, 2, 3));*/

	p_geom = new r_geometry(pdev);
	p_geom->load(&mesh, true);
	lib_geom.add("scr_quad", shared_ptr<r_geometry>(p_geom));

	//geometry hierarchy
	std::vector<point_sample_param> scales;
	point_sample_param pp;
	r_geom_hierarchy *p_gh;

	p_gh = new r_geom_hierarchy();
	p_gh->set_geom(get_geom("sphere_200"));
	scales.clear();
	pp.s = 0.1f; pp.num = 8;
	scales.push_back(pp);
	pp.s = 0.15f; pp.num = 4;
	scales.push_back(pp);
	pp.s = 0.2f; pp.num = 4;
	scales.push_back(pp);
	pp.s = 0.25f; pp.num = 4;
	scales.push_back(pp);
	pp.s = 0.3f; pp.num = 4;
	scales.push_back(pp);
	pp.s = 1.0f; pp.num = 1;
	scales.push_back(pp);
	p_gh->set_scales(scales);
	lib_gh.add("sphere_200", shared_ptr<r_geom_hierarchy>(p_gh));

	p_gh = new r_geom_hierarchy();
	p_gh->set_geom(get_geom("rod"));
	scales.clear();
	pp.s = 0.2f; pp.num = 8;
	scales.push_back(pp);
	pp.s = 0.3f; pp.num = 8;
	scales.push_back(pp);
	pp.s = 0.35f; pp.num = 12;
	scales.push_back(pp);
	pp.s = 0.4f; pp.num = 8;
	scales.push_back(pp);
	pp.s = 0.5f; pp.num = 8;
	scales.push_back(pp);
	pp.s = 0.6f; pp.num = 8;
	scales.push_back(pp);
	pp.s = 0.7f; pp.num = 8;
	scales.push_back(pp);
	pp.s = 0.8f; pp.num = 8;
	scales.push_back(pp);
	pp.s = 0.9f; pp.num = 8;
	scales.push_back(pp);
	pp.s = 1.0f; pp.num = 8;
	scales.push_back(pp);
	p_gh->set_scales(scales);
	lib_gh.add("rod", shared_ptr<r_geom_hierarchy>(p_gh));

	//blend state
	r_blendstate *p_blend;

	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.RenderTarget[0].BlendEnable	= TRUE;
	desc.RenderTarget[0].SrcBlend		= D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlend		= D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOp		= D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha	= D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha	= D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOpAlpha	= D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask
										= D3D11_COLOR_WRITE_ENABLE_ALL;
	p_blend = new r_blendstate(pdev, desc);
	lib_blend.add("add", p_blend);

	ZeroMemory(&desc, sizeof(desc));
	desc.RenderTarget[0].BlendEnable	= TRUE;
	desc.RenderTarget[0].SrcBlend		= D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend		= D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOp		= D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha	= D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha	= D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOpAlpha	= D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask
										= D3D11_COLOR_WRITE_ENABLE_ALL;
	p_blend = new r_blendstate(pdev, desc);
	lib_blend.add("alpha", p_blend);

	//ds state
	r_dsstate *p_ds;

	D3D11_DEPTH_STENCIL_DESC dsdesc;
	ZeroMemory(&dsdesc, sizeof(dsdesc));
	dsdesc.DepthEnable = TRUE;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsdesc.StencilEnable = FALSE;
	p_ds = new r_dsstate(pdev, dsdesc);
	lib_ds.add("lesseq", p_ds);

	dsdesc.DepthEnable = FALSE;
	dsdesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	p_ds = new r_dsstate(pdev, dsdesc);
	lib_ds.add("no", p_ds);

	//sampler state
	r_samplerstate *p_sampler;

	//for shadow
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	p_sampler = new r_samplerstate(pdev, sampDesc);
	lib_sampler.add("shadow", p_sampler);

	//for vis_point
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = 0;
	p_sampler = new r_samplerstate(pdev, sampDesc);
	lib_sampler.add("point", p_sampler);
}

void microfacet_factory::add_basis_matr(const int id, const char *filename)
{
	float	attr[NUM_MATR_ATTR];
	char	matr_name[256], matr_attr[256], matr_filename[512],
			brdf_name[256], brdf_param[256];

	FILE *fp;
	fp = fopen(filename, "rt");
	fgets(matr_name, sizeof(matr_name)-1, fp);
	matr_name[strlen(matr_name)-1] = 0;
	fgets(matr_attr, sizeof(matr_attr)-1, fp);
	fgets(matr_filename, sizeof(matr_filename)-1, fp);
	matr_filename[strlen(matr_filename)-1] = 0;
	fgets(brdf_name, sizeof(brdf_name)-1, fp);
	brdf_name[strlen(brdf_name)-1] = 0;
	if (fgets(brdf_param, sizeof(brdf_param)-1, fp))
	{
		if (strlen(brdf_param) > 0)
			brdf_param[strlen(brdf_param)-1] = 0;
	} else {
		brdf_param[0] = 0;
	}
	fclose(fp);

	//printf_s("config loaded. ");

	preconv_matr *b_matr;
	b_matr = new preconv_matr;
	b_matr->pBRDF = BRDF_factory::produce(brdf_name, brdf_param);
	//printf_s("BRDF loaded. ");

	b_matr->p_sampler = new BRDF_sampler;
	b_matr->p_sampler->init(b_matr->pBRDF);
	b_matr->load(matr_filename);

	//printf_s("material loaded. ");

	sscanf(matr_attr, "%f", &attr[MATR_ATTR_SPEC]);
	memcpy(b_matr->attr.attr, attr, sizeof(b_matr->attr.attr));
	b_matr->init_R(NORMAL_DISTR_REDUCED);
	lib_basis_matr.add(id, b_matr);
	reg_basis_matr_name(id, matr_name);
}

//void microfacet_factory::add_basis_matr(const int id, const char *name, const char *filename, 
//										BRDF_interface *p_brdf, float *attr)
//{
//	preconv_matr *b_matr;
//	b_matr = new preconv_matr;
//	b_matr->pBRDF = p_brdf;
//	b_matr->load(filename);
//	memcpy(b_matr->attr.attr, attr, sizeof(b_matr->attr.attr));
//	b_matr->init_R(NORMAL_DISTR_REDUCED);
//	lib_basis_matr.add(id, b_matr);
//	reg_basis_matr_name(id, name);
//}

r_input_layer* microfacet_factory::get_layer(const string &name)
{
	return lib_layer.get(name);
}

r_shader* microfacet_factory::get_shader(const string &name)
{
	return lib_shader.get(name);
}

r_samplerstate* microfacet_factory::get_sampler(const string &name)
{
	return lib_sampler.get(name);
}

r_blendstate* microfacet_factory::get_blend(const string &name)
{
	return lib_blend.get(name);
}

r_dsstate* microfacet_factory::get_ds(const string &name)
{
	return lib_ds.get(name);
}

shared_ptr<r_geometry> microfacet_factory::get_geom(const string &name)
{
	return lib_geom.get(name);
}

shared_ptr<r_geom_hierarchy> microfacet_factory::get_geom_hierarchy(const string &name)
{
	return lib_gh.get(name);
}

bool microfacet_factory::assign_geom(const string &name, shared_ptr<r_geometry> &p)
{
	return lib_geom.assign(name, p);
}

bool microfacet_factory::assign_geom_hierarchy(const string &name, shared_ptr<r_geom_hierarchy> &p)
{
	return lib_gh.assign(name, p);
}

preconv_matr* microfacet_factory::get_basis_matr(const int &id)
{
	return lib_basis_matr.get(id);
}

string microfacet_factory::get_basis_matr_name(const int &id)
{
	return lib_basis_matr_name.get(id);
}

const std::vector<int>& microfacet_factory::get_all_basis_matr_ids()
{
	return basis_matr_id;
}

matr* microfacet_factory::get_matr(const int &id)
{
	return lib_matr.get(id);
}

int microfacet_factory::get_matr_id(const string &name)
{
	return lib_matr_id.get(name);
}

void microfacet_factory::add_matr(const int &id, matr* p)
{
	lib_matr.add(id, p);
}

void microfacet_factory::reg_basis_matr_name(const int &id, const string &name)
{
	lib_matr_id.add(name, id);
	lib_basis_matr_name.add(id, name);
	basis_matr_id.push_back(id);
}

microfacet_factory::~microfacet_factory()
{
}

D3D_dev_handle* microfacet_factory::get_handle() const
{
	return pdev;
}

//void microfacet_factory::set_light(const char *shader_name, const r_light_dir *lt, const Matrix4 *mat)
//{
//	((r_shader_basic*)get_shader(shader_name))->setup_cb_lights(lt, mat);
//}

void microfacet_factory::produce(r_instance* &result, instance_property &p, int &mat_id)
{
	result = new r_instance;
	//cout << "int factory produce id = " << hex<< p.id << endl;
	switch (p.id)
	{
	case M_TEST:
		{
			result->init(p.p_geom, p.scale, get_shader("test_shader"), false);
			mat_id = MATR_DISTR_0;
		}
		break;
	case M_ID_TEST_PARTICLE:
		{
			result->init(get_geom_hierarchy("sphere_200"), p.scale, get_shader("diffuse"), false);
			mat_id = MATR_DISTR_0;
		}
		break;
	case M_ID_TEST_ROD:
		{
			result->init(get_geom_hierarchy("rod"), p.scale, get_shader("diffuse"), false);
			mat_id = MATR_DISTR_0;
		}
		break;
	case M_ID_BINDER_PLANE:
		{
			result->init(get_geom_hierarchy("bd_plane"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_0;
		}
		break;
	case M_ID_BINDER_GROOVE_PLANE:
		{
			result->init(get_geom_hierarchy("bd_groove_p"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_0;
		}
		break;
	case M_ID_BINDER_GROOVE_HILL:
		{
			result->init(get_geom_hierarchy("bd_groove_h"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_1;
		}
		break;
	case M_ID_BINDER_BRICK_BOTTOM:
		{
			result->init(get_geom_hierarchy("bd_brick_b"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_0;
		}
		break;
	case M_ID_BINDER_BRICK_SLOPE:
		{
			result->init(get_geom_hierarchy("bd_brick_s"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_1;
		}
		break;
	case M_ID_BINDER_BRICK_TOP:
		{
			result->init(get_geom_hierarchy("bd_brick_t"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_2;
		}
		break;
	case M_ID_BINDER_REGPOLY:
		{
			result->init(get_geom_hierarchy("bd_regpoly"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_1;
		}
		break;
	case M_ID_BINDER_DISPMAP:
		{
			result->init(get_geom_hierarchy("bd_dispmap"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_0;
		}
		break;
	case M_ID_BINDER_WOVEN_Y:
		{
			result->init(get_geom_hierarchy("bd_woven_y"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_1;
		}
		break;
	case M_ID_BINDER_WOVEN_X:
		{
			result->init(get_geom_hierarchy("bd_woven_x"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_0;
		}
		break;
	case M_ID_BINDER_WOVEN_THREAD_Y:
		{
			result->init(get_geom_hierarchy("bd_thread_y"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_1;
		}
		break;
	case M_ID_BINDER_WOVEN_THREAD_X:
		{
			result->init(get_geom_hierarchy("bd_thread_x"), p.scale, get_shader("diffuse"), true);
			mat_id = MATR_BINDER_0;
		}
		break;
	case M_ID_BASE_OBJ:
		{
			result->init(p.p_geom, p.scale, get_shader("init_context"), true);
			mat_id = MATR_LAMBERTIAN_0;
		}
		break;
	case M_ID_BACKGROUND_OBJ:
		{
			result->init(p.p_geom, p.scale, get_shader("init_back"), true);
			mat_id = MATR_LAMBERTIAN_0;
		}
		break;
	case M_ID_OBJ_VIS:
		{
			result->init(p.p_geom, p.scale, get_shader("diffuse"), true);
			mat_id = MATR_LAMBERTIAN_0;
		}
		break;
	default:
		//throw_error(ERROR_POS, EXP_GENERIC, "Unknown id");
		printf("Error::microfacet_factory: Unknown id\n");
		break;
	}

	result->get_shader_input()->matWorld = p.mat;
}

//factory singleton
microfacet_factory* mff_singleton::factory_instance = NULL;
parab_frame mff_singleton::fr_sample;

microfacet_factory* mff_singleton::get()
{
	return factory_instance;
}

void mff_singleton::set(microfacet_factory* p)
{
	factory_instance = p;
}

void mff_singleton::init()
{
	FILE *fp;
	fopen_s(&fp, FRAME_PATH "sample.frame", "rb");
	if (fp == NULL)
	{
		fr_sample.init(256, 32, 32);
		fr_sample.normalize_n();
		fr_sample.compute_spherical_area(256*1048576);
		fopen_s(&fp, FRAME_PATH "sample.frame", "wb");
		fr_sample.save(fp);
		fclose(fp);
	} else {
		fr_sample.load(fp);
		fclose(fp);
	}
}

const parab_frame& mff_singleton::fr()
{
	return fr_sample;
}