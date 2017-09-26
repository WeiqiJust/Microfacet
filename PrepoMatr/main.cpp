#include "main.h"
#define DIM_VIS					10

//!!! Protocol - MUST READ !!!
//To process each new material,
//1. use test_sampling_rate to compute optimal sampling rate
//2. use a reduced version of the optimal sampling rate and set samples_per_texel for normal = 1, to quickly compute the eigen number!
//3. Set the reduced dim to about 12 times the eigen number and process for the final result.

void test_sampling_rate(int &dim_n, int &dim_brdf,
						const BRDF_interface *brdf,
						const vector3f &Mc,
						double SNR_n = 20, double SNR_brdf = 20)
{
	compute_sampling_rate(dim_n, dim_brdf, 
		8, 64, SNR_n,
		8, 64, SNR_brdf,
		brdf,
		Mc);
	printf_s("n: %d  BRDF: %d\n", dim_n, dim_brdf);
}

void test_matr()
{
	vector3f M;

	//Lambertian - Official
	//For SNR = 17db, n: 12  BRDF: 12 Time 22.02
	{
		Lambert_interface brdf;
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 17, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/Lambert.preconv_material", 
			16, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			84,	SVD_QUALITY, M);

	}

	////For SNR = 17db, n: 15  BRDF: 12 Time = 31.91secs
	//{
	//	Lambert_interface brdf;
	//	compute_color_matrix(M, 16, 256, 16, 256, &brdf);
	//	//int dim_n, dim_brdf;
	//	//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
	//	preconv_matr_rp_brdf("T:/ProMaterial/material/Lambert_log", 
	//		15, 64,
	//		12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
	//		84,	SVD_QUALITY, M);
	//}

	//alum-bronze - Official
	//alumina-oxide - Official
	//aluminium.binary
	//aventurnine - Official
	//black-obsidian - official
	//black-phenolic.binary - official
	//blue-acrylic.binary - official
	//blue-metallic-paint2.binary
	//brass.binary
	//cherry-235.binary - official
	//chrome.binary
	//chrome-steel.binary
	//color-changing-paint1.binary
	//color-changing-paint2.binary - official
	//color-changing-paint3.binary
	//dark-specular-fabric.binary
	//fruitwood-241.binary - official
	//gold-metallic-paint2.binary
	//gold-metallic-paint3.binary
	//gray-plastic.binary - official
	//grease-covered-steel.binary
	//green-acrylic.binary
	//green-metallic-paint2.binary
	//green-plastic.binary
	//hematite.binary
	//ipswich-pine-221.binary
	//maroon-plastic.binary
	//natural-209.binary
	//nickel.binary
	//pink-jasper.binary
	//purple-paint.binary
	//pvc.binary
	//red-metallic-paint.binary
	//red-phenolic.binary
	//red-specular-plastic.binary
	//silicon-nitrade.binary
	//specular-black-phenolic.binary
	//specular-blue-phenolic.binary
	//specular-green-phenolic.binary
	//specular-maroon-phenolic.binary
	//specular-orange-phenolic.binary
	//specular-red-phenolic.binary
	//specular-violet-phenolic.binary
	//specular-white-phenolic.binary
	//specular-yellow-phenolic.binary
	//ss440.binary
	//steel.binary
	//tungsten-carbide.binary
	//two-layer-gold.binary
	//two-layer-silver.binary
	//violet-acrylic.binary
	//white-acrylic.binary
	//white-marble.binary
	//20db, 17db, n: >63  BRDF: >63. Time = 
	
	//black-oxidized-steel.binary - official
	//20db, 17db, n: 48  BRDF: > 51. Time = 
	//delrin.binary - official
	//20db, 17db, n: 19  BRDF: > 51. Time = 
	//nylon.binary - official
	//20db, 17db, n: 44  BRDF: > 51. Time = 


	// Time = (log)150.4secs 
	/*
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\black-fabric.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/black-fabric_log", 
			15, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			200, SVD_QUALITY, M);
	}	

	//beige-fabric - Official
	//20db, 17db, n: 19, BRDF: 12. Time = (log)230.5 / 205.83
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\beige-fabric.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/beige-fabric_log", 
			19, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			200, SVD_QUALITY, M);
	}	

	//blue-metallic-paint.binary - official
	//20db, 17db, n: 63  BRDF: 34. Time = (log)22153.8 / 20835
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\blue-metallic-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		
		preconv_matr_rp_brdf("T:/ProMaterial/material/blue-metallic-paint_log", 
			63, 4,
			34, 32, DEFAULT_AREA_SAMPLES, &brdf, 
			1500, SVD_QUALITY, M);
	}	

	//black-soft-plastic.binary - official
	//20db, 17db, n: 37  BRDF: 26. Time = (log)2338.1 / 2218.07
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\black-soft-plastic.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);

		preconv_matr_rp_brdf("T:/ProMaterial/material/black-soft-plastic_log", 
			37, 8,
			26, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			400, SVD_QUALITY, M);
	}	

	//blue-fabric.binary - offical
	//20db, 17db, n: 19  BRDF: 12. Time = (log)122.2 / 113.70
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\blue-fabric.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);

		preconv_matr_rp_brdf("T:/ProMaterial/material/blue-fabric_log", 
			19, 32,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			200, SVD_QUALITY, M);
	}

	//blue-rubber - Official
	//20db, 17db, n: 19  BRDF: 19. Time = (log)659.6
	//17db, n: 12  BRDF: 19. Time = 483.65
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\blue-rubber.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);

		preconv_matr_rp_brdf("T:/ProMaterial/material/blue-rubber_log", 
			19, 32,
			19, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			240, SVD_QUALITY, M);
	}

	//colonial-maple-223.binary - Official 
	//20db, 17db, n: 30  BRDF: 34. Time = (log)10093.7 / 9538.55
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\colonial-maple-223.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);

		preconv_matr_rp_brdf("T:/ProMaterial/material/colonial-maple-223_log", 
			30, 16,
			34, 32, DEFAULT_AREA_SAMPLES, &brdf, 
			1500, SVD_QUALITY, M);
	}

	//dark-blue-paint.binary - official
	//20db, 17db, n: 26  BRDF: 22. Time = (log)2107.7 / 1995.63
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\dark-blue-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);

		preconv_matr_rp_brdf("T:/ProMaterial/material/dark-blue-paint_log", 
			26, 32,
			22, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			400, SVD_QUALITY, M);
	}

	//dark-red-paint.binary - official
	//20db, 17db, n: 15  BRDF: 12. Time = (log)146.7 / 134.79
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\dark-red-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/dark-red-paint_log", 
			15, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			200, SVD_QUALITY, M);
	}

	//gold-metallic-paint.binary - official
	//20db, 17db, n: 63  BRDF: 34. Time = (log)15891.2 / 15282.9
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\gold-metallic-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/gold-metallic-paint_log", 
			63, 2,
			34, 32, DEFAULT_AREA_SAMPLES, &brdf, 
			1200, SVD_QUALITY, M);
	}

	//gold-paint - Official
	//For SNR = 17db, n: 34  BRDF: 26. Time = 2259.66
	//20db, 17db, n: 49  BRDF: 26. Time = (log)3197.1 / 3220.62
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\gold-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);

		preconv_matr_rp_brdf("T:/ProMaterial/material/gold-paint_log", 
			49, 4,
			26, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			800, SVD_QUALITY, M);
	}	

	//green-fabric.binary
	//20db, 17db, n: 19  BRDF: 12. Time = (log)222.9 / 208.5
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\green-fabric.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/green-fabric_log", 
			19, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			200, SVD_QUALITY, M);
	}

	//green-latex.binary
	//20db, 17db, n: 19  BRDF: 12. Time = (log) 222.5 / 205.36 
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\green-latex.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/green-latex_log", 
			19, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			200, SVD_QUALITY, M);
	}

	//green-metallic-paint.binary - official
	//20db, 17db, n: 63  BRDF: 44. Time = (log) 48003 / 46874.5
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\green-metallic-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/green-metallic-paint_log", 
			63, 2,
			44, 8, DEFAULT_AREA_SAMPLES, &brdf, 
			1200, SVD_QUALITY, M);
	}

	//light-brown-fabric.binary
	//20db, 17db, n: 15  BRDF: 12. Time = (log)153.9 / 133.2
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\light-brown-fabric.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/light-brown-fabric_log", 
			15, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			150, SVD_QUALITY, M);
	}

	//light-red-paint - Official
	//20db, 17db, n: 15  BRDF: 12. Time = (log)153.9
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\light-red-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/light-red-paint_log", 
			15, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			180, SVD_QUALITY, M);
	}	

	//neoprene-rubber.binary - Official
	//20db, 17db, n: 19  BRDF: 15. Time = (log)296.4 / 249.51
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\neoprene-rubber.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/neoprene-rubber_log", 
			19, 32,
			15, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			240, SVD_QUALITY, M);
	}	

	//orange-paint.binary - Official
	//20db, 17db, n:   BRDF: . Time = (log)155.5 / 133.98
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\orange-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/orange-paint_log", 
			15, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			150, SVD_QUALITY, M);
	}	

	//pearl-paint - Official
	//For SNR = 17db, n: 26  BRDF: 19. Time = 
	//17db, 20db, n: 26  BRDF: 30. Time = 3828.53
	//20db, 17db, n: 35  BRDF: 19. Time = (log)728.1 / 681
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\pearl-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/pearl-paint_log", 
			35, 8,
			19, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			700, SVD_QUALITY, M);
	}	

	//pickled-oak-260.binary - Official
	//20db, 17db, n: 34  BRDF: 22. Time = (log)1922.1 / 1696.45
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\pickled-oak-260.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/pickled-oak-260_log", 
			34, 16, 
			22, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			300, SVD_QUALITY, M);
	}	

	//pink-fabric.binary - Official
	//20db, 17db, n: 15  BRDF: 12. Time = (log)1861.5 
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\pink-fabric.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/pink-fabric_log", 
			34, 16, 
			22, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			300, SVD_QUALITY, M);
	}		

	//pink-fabric2.binary - Official
	//20db, 17db, n: 19  BRDF: 12. Time = (log)230.5 / 205.69
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\pink-fabric2.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/pink-fabric2_log", 
			19, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			240, SVD_QUALITY, M);
	}

	//pink-felt.binary - Official
	//20db, 17db, n: 15  BRDF: 12. Time = (log)149.3 / 132.76
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\pink-felt.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/pink-felt_log", 
			15, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			150, SVD_QUALITY, M);
	}

	//pink-plastic.binary
	//20db, 17db, n: 15  BRDF: 12. Time = (log)149.3 / 135.37
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\pink-plastic.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/pink-plastic_log", 
			15, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			150, SVD_QUALITY, M);
	}

	//polyethylene.binary
	//20db, 17db, n: 19  BRDF: 12. Time = (log)230.4 / 208.39
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\polyethylene.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/polyethylene_log", 
			19, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			240, SVD_QUALITY, M);
	}

	//polyurethane-foam.binary
	//20db, 17db, n: 15  BRDF: 12. Time = (log)149.2 / 133.81
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\polyurethane-foam.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/polyurethane-foam_log", 
			15, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			150, SVD_QUALITY, M);
	}

	//pure-rubber.binary
	//20db, 17db, n: 15 BRDF: 12. Time = (log)149.2 / 134.01
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\pure-rubber.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/pure-rubber_log", 
			15, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			150, SVD_QUALITY, M);
	}

	//red-fabric.binary
	//20db, 17db, n: 19 BRDF: 12. Time = (log)230.3 / 205
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\red-fabric.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/red-fabric_log", 
			19, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			240, SVD_QUALITY, M);
	}

	//red-fabric2.binary
	//20db, 17db, n: 15 BRDF: 12. Time = (log)149.2 / 133
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\red-fabric2.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/red-fabric2_log", 
			15, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			150, SVD_QUALITY, M);
	}

	//red-plastic.binary
	//20db, 17db, n: 19 BRDF: 12. Time = (log)235.7 
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\red-plastic.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/red-plastic_log", 
			19, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			240, SVD_QUALITY, M);
	}

	//silver-metallic-paint - Official
	//20db, 17db, n: 63  BRDF: 37. Time = 25408 / 28760.7(log)
	//For SNR = 20db, n: 63  BRDF: 55.
	//For SNR = 17db, n: 55  BRDF: 37. Time = 22174.3
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\silver-metallic-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		preconv_matr_rp_brdf("T:/ProMaterial/material/silver-metallic-paint_log", 
			63, 4,
			37, 16, DEFAULT_AREA_SAMPLES, &brdf, 
			1200, SVD_QUALITY,	M);
	}

	//silver-metallic-paint2.binary - Official
	//20db, 17db, n: 63  BRDF: 30. Time = (log)11166.4 / 10172
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\silver-metallic-paint2.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/silver-metallic-paint2_log", 
			63, 4 ,
			30, 32, DEFAULT_AREA_SAMPLES, &brdf, 
			1000, SVD_QUALITY, M);
	}

	//silver-paint.binary - Official
	//20db, 17db, n:   BRDF: . Time = (log)4724.2 / 4276
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\silver-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/silver-paint_log", 
			48, 8, 
			26, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			800, SVD_QUALITY, M);
	}

	//special-walnut-224.binary
	//20db, 17db, n: 51 BRDF: 41. Time = 38016.3 / (log) 41480.2
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\special-walnut-224.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/special-walnut-224_log", 
			51, 8, 
			41, 8, DEFAULT_AREA_SAMPLES, &brdf, 
			1500, SVD_QUALITY, M);
	}

	//teflon.binary
	//20db, 17db, n: 19 BRDF: 15. Time = (log)527.4 / 477.36
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\teflon.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/teflon_log", 
			19, 64, 
			15, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			300, SVD_QUALITY, M);
	}

	//violet-rubber.binary
	//20db, 17db, n: 19 BRDF: 12. Time = (log)231.3 / 207
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\violet-rubber.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/violet-rubber_log", 
			19, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			240, SVD_QUALITY, M);
	}

	//white-diffuse-bball.binary
	//20db, 17db, n: 19 BRDF: 19. Time = (log)1265.0 / 1138
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\white-diffuse-bball.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/white-diffuse-bball_log", 
			19, 64, 
			19, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			280, SVD_QUALITY, M);
	}

	//white-fabric.binary
	//20db, 17db, n: 15 BRDF: 12. Time = (log)149.7 / 134
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\white-fabric.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/white-fabric_log", 
			15, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			150, SVD_QUALITY, M);
	}

	//white-fabric2.binary
	//20db, 17db, n:  BRDF: . Time = (log)231.2 / 207
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\white-fabric2.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/white-fabric2_log", 
			19, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			240, SVD_QUALITY, M);
	}

	//yellow-paint.binary
	//20db, 17db, n: 15 BRDF: 12. Time = (log)149.7 / 132
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\yellow-paint.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/yellow-paint_log", 
			15, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			150, SVD_QUALITY, M);
	}

	//yellow-plastic.binary
	//20db, 17db, n: 19 BRDF: 12. Time = (log)231.2 / 209
	{
		measured_BRDF brdf;
		brdf.init("D:\\Data\\MERL BRDF\\yellow-plastic.binary");
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);
		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/yellow-plastic_log", 
			19, 64, 
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			240, SVD_QUALITY, M);
	}*/
}

void sample_BlinnPhong()
{
	//exp = 2  :		19, 32, 12, 64,		rp : 128	Time =   39.70(log)
	//exp = 4  :		26, 32, 12, 64,		rp : 128	Time =   67.36(log)
	//exp = 8  :		30, 16, 15, 64,		rp : 160	Time =  111.89(log)
	//exp = 16 :		41, 8, 19, 64,		rp : 300	Time =  313.13(log) / 270.63secs
	//exp = 32 :		55, 4, 26, 64,		rp : 500	Time = 1689.82(log) / 1588.77secs
	vector3f M;

	{
		BlinnPhong_interface brdf(2.0);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		preconv_matr_rp_brdf("T:/ProMaterial/material/Phong_2", 
			19, 32,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			128, SVD_QUALITY, M);
	}

	{
		BlinnPhong_interface brdf(4.0);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		preconv_matr_rp_brdf("T:/ProMaterial/material/Phong_4", 
			26, 32,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			128, SVD_QUALITY, M);
	}

	{
		BlinnPhong_interface brdf(8.0);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		preconv_matr_rp_brdf("T:/ProMaterial/material/Phong_8", 
			30, 16,
			15, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			180, SVD_QUALITY, M);
	}

	{
		BlinnPhong_interface brdf(16.0);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/Phong_16", 
			41, 8,
			19, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			300, SVD_QUALITY, M);
	}

	{
		BlinnPhong_interface brdf(32.0);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/Phong_32", 
			55, 4,
			26, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			500, SVD_QUALITY, M);
	}
}

void sample_Ward()
{
	//Native space
	//alpha = 0.5 :		34, 16, 15, 64,		rp : 200	Time = secs
	//alpha = 0.4 :		37,  8, 19, 64,		rp : 400	Time = 287.90secs
	//alpha = 0.3 :		48,  4, 26, 64,		rp : 600	Time = 1573.51secs
	//alpha = 0.2 :		63,  4, 34, 16,		rp : 800	Time = 10488.1secs

	//Log-space
	//alpha = 0.5 :		30, 16, 15, 64,		rp : 200	Time = 135.24secs
	//alpha = 0.4 :		37,  8, 19, 64,		rp : 400	Time = 326.77secs
	//alpha = 0.3 :		48,  4, 22, 64,		rp : 600	Time = 779.387secs
	//alpha = 0.2 :		63,  4, 34, 16,		rp : 800	Time = 10354secs

	vector3f M;

	{
		Ward_interface brdf(0.5);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		int dim_n, dim_brdf;
		test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
	//	//preconv_matr_rp_brdf("d:/temp/Microfacet/material/HQ/Ward_0.50_34_15_98.preconv_material", 
		preconv_matr_rp_brdf("T:/ProMaterial/material/Ward_0.50", 
			34, 16,
			15, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			200, SVD_QUALITY, M);
	}
	
	cout << "finish ward 0.5" << endl;
	/*
	{
		Ward_interface brdf(0.4);
	//	//resample_brdf("d:/temp/Microfacet/material/Ward_0.4.brdf", 
	//	//	&brdf, 19,	64, DEFAULT_AREA_SAMPLES);
	//	//return;

		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		preconv_matr_rp_brdf("T:/ProMaterial/material/Ward_0.40", 
			37, 8,
			19,	64, DEFAULT_AREA_SAMPLES, &brdf, 
			400, SVD_QUALITY, M);
	}
	cout << "finish ward 0.4" << endl;
	{
		Ward_interface brdf(0.3);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/Ward_0.30", 
			48, 4,
			26, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			600, SVD_QUALITY, M);
	}
	cout << "finish ward 0.3" << endl;*/
	{
		Ward_interface brdf(0.2);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

	//	//resample_brdf("d:/temp/Microfacet/material/Ward_0.2.brdf", 
	//	//	&brdf, 34, 16, DEFAULT_AREA_SAMPLES);
	//	//return;
	//	//int dim_n, dim_brdf;
	//	//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		preconv_matr_rp_brdf("T:/ProMaterial/material/Ward_0.20", 
			63, 4,
			34, 16, DEFAULT_AREA_SAMPLES, &brdf, 
			800, SVD_QUALITY, M);

	//	//compute_nonzero_elements(63, 4,	34, 16, DEFAULT_AREA_SAMPLES, &brdf);
	}
}


void sample_Ward_extra()
{
	vector3f M;

	{
		Ward_interface brdf(0.6);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		//	//preconv_matr_rp_brdf("d:/temp/Microfacet/material/HQ/Ward_0.50_34_15_98.preconv_material", 
		
		preconv_matr_rp_brdf("T:/ProMaterial/material/Ward_0.60",
			30, 64,
			15, 64, DEFAULT_AREA_SAMPLES, &brdf,
			200, SVD_QUALITY, M);
	}
	cout << "finish ward 0.6" << endl;
	
	{
		Ward_interface brdf(0.7);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		//	//preconv_matr_rp_brdf("d:/temp/Microfacet/material/HQ/Ward_0.50_34_15_98.preconv_material", 

		preconv_matr_rp_brdf("T:/ProMaterial/material/Ward_0.70",
			26, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf,
			200, SVD_QUALITY, M);
	}
	cout << "finish ward 0.7" << endl;

	{
		Ward_interface brdf(0.8);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		//	//preconv_matr_rp_brdf("d:/temp/Microfacet/material/HQ/Ward_0.50_34_15_98.preconv_material", 

		preconv_matr_rp_brdf("T:/ProMaterial/material/Ward_0.80",
			22, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf,
			160, SVD_QUALITY, M);
	}
	cout << "finish ward 0.8" << endl;

	{
		Ward_interface brdf(0.9);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		//int dim_n, dim_brdf;
		//test_sampling_rate(dim_n, dim_brdf, &brdf, M, 20, 17);
		//	//preconv_matr_rp_brdf("d:/temp/Microfacet/material/HQ/Ward_0.50_34_15_98.preconv_material", 

		preconv_matr_rp_brdf("T:/ProMaterial/material/Ward_0.90",
			22, 64,
			12, 64, DEFAULT_AREA_SAMPLES, &brdf,
			160, SVD_QUALITY, M);
	}
	cout << "finish ward 0.9" << endl;

}

void sample_CT()
{
	//m = 0.5, F0 = 0.25 :	30, 16, 19, 64	rp : 200	Time = (log)  320.6 /   262.4
	//m = 0.4, F0 = 0.25 :	34, 16, 22, 64,	rp : 240	Time = (log)  729.8 /   587.5
	//m = 0.3, F0 = 0.25 :	44, 8,  30,	32	rp : 320	Time = (log) 2590.5 /  2270.7
	//m = 0.2, F0 = 0.25 :	63, 2,  48, 4	rp : 800	Time = (log)40188.1 / 93409.3
	vector3f M;

	{
		CT_interface brdf(0.5, 0.25);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		preconv_matr_rp_brdf("T:/ProMaterial/material/CT_0.5", 
			30, 16,
			19, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			200, SVD_QUALITY, M);
	}
	cout << "finish CT 0.5" << endl;
	{
		CT_interface brdf(0.4, 0.25);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		preconv_matr_rp_brdf("T:/ProMaterial/material/CT_0.4", 
			34, 16,
			22, 64, DEFAULT_AREA_SAMPLES, &brdf, 
			240, SVD_QUALITY, M);
	}
	cout << "finish CT 0.4" << endl;
	{
		CT_interface brdf(0.3, 0.25);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		preconv_matr_rp_brdf("T:/ProMaterial/material/CT_0.3", 
			44, 8,
			30, 32, DEFAULT_AREA_SAMPLES, &brdf, 
			320, SVD_QUALITY, M);
	}
	cout << "finish CT 0.3" << endl;
	{
		CT_interface brdf(0.2, 0.25);
		compute_color_matrix(M, 16, 256, 16, 256, &brdf);

		preconv_matr_rp_brdf("T:/ProMaterial/material/CT_0.2", 
			63, 2,
			48, 4, DEFAULT_AREA_SAMPLES, &brdf, 
			800, SVD_QUALITY, M);
	}
}

void compute_scalar()
{
	compute_SVD_scalar("d:/temp/Microfacet/material/Lambert_16_12_98.scalar",
		"d:/temp/Microfacet/material/Lambert_16_12_98.preconv_material", DIM_VIS);
}

void main()
{
	//parab_frame fr;
	//fr.init(12, 400, 400);
	//printf_s("%d\n", fr.get_n().size());
	//return;
	//test_matr();
	//compute_scalar();
	//sample_BlinnPhong();
	//sample_CT();
	//sample_Ward();
	sample_Ward_extra();
}

