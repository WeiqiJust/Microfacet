#pragma once

#include "microfacet.h"

#include <map>

class instance_property;

template<class Type, class KeyType, bool b_release = true>
class generic_map
{
private:
	map<KeyType, Type> lib;

	void release()
	{
	#if (b_release)
		{
			map<KeyType, Type>::iterator iter = lib.begin();
			while (iter != lib.end())
			{
				SAFE_DELETE(iter->second);
				iter++;
			}
		}
	#endif
	}

public:
	~generic_map()
	{
		if (b_release)
			release();
	}

	Type get(const KeyType &name) const
	{
		map<KeyType, Type>::const_iterator iter;

		iter = lib.find(name);

		if (iter != lib.end())
			return iter->second;
		else
		{ 
			//cout << "get return null" << endl;
			return NULL;
		}
	}

	void add(const KeyType &name, const Type p)
	{
		lib.insert(std::make_pair(name, p));
	}

	bool assign(const KeyType &name, const Type p)
	{
		map<KeyType, Type>::iterator iter;
		iter = lib.find(name);

		if (iter != lib.end())
		{
			iter->second = p;
			return true;
		} else {
			lib.insert(std::make_pair(name, p));
			return false;
		}
	}
};

class microfacet_factory
{
private:
	D3D_dev_handle		*pdev;
		
	generic_map<r_shader*, string>		lib_shader;
	generic_map<r_blendstate*, string>	lib_blend;
	generic_map<r_dsstate*, string>		lib_ds;
	generic_map<r_samplerstate*, string>lib_sampler;
	generic_map<r_input_layer*, string>	lib_layer;
	generic_map<preconv_matr*, int>		lib_basis_matr;
	generic_map<matr*, int>				lib_matr;
		
	std::vector<int>					basis_matr_id;
	generic_map<string, int, false>		lib_basis_matr_name;
	//For both basis matr and matr
	generic_map<int, string, false>		lib_matr_id;
		
	generic_map<shared_ptr<r_geometry>, string, false>		
										lib_geom;
	generic_map<shared_ptr<r_geom_hierarchy>, string, false>
										lib_gh;

public:
	microfacet_factory(D3D_dev_handle *pdev);
	~microfacet_factory();
	void produce(r_instance* &result, instance_property &p, int &mat_id);

	D3D_dev_handle* get_handle() const;

	shared_ptr<r_geometry> get_geom(const string &name);
	shared_ptr<r_geom_hierarchy> get_geom_hierarchy(const string &name);
	bool assign_geom(const string &name, shared_ptr<r_geometry> &p);
	bool assign_geom_hierarchy(const string &name, shared_ptr<r_geom_hierarchy> &p);

	r_input_layer*		get_layer(const string &name);
	r_shader*			get_shader(const string &name);
	r_samplerstate*		get_sampler(const string &name);
	r_blendstate*		get_blend(const string &name);
	r_dsstate*			get_ds(const string &name);

	preconv_matr*		get_basis_matr(const int &id);
	string				get_basis_matr_name(const int &id);
	const std::vector<int>& get_all_basis_matr_ids();
	int					get_matr_id(const string &name);

	void				reg_basis_matr_name(const int &id, const string &name);
	//void				add_basis_matr(const int id, const char *name, const char *filename, 
	//						BRDF_interface *p_brdf, float *attr);
	void				add_basis_matr(const int id, const char *filename);

	matr*				get_matr(const int &id);
	void				add_matr(const int &id, matr* p);
};

class mff_singleton
{
public:
	static microfacet_factory* get();
	static void set(microfacet_factory* p);
	static void init();

	static const parab_frame& fr();

protected:
	mff_singleton();
private:
	static parab_frame fr_sample;
	static microfacet_factory*	factory_instance;
};