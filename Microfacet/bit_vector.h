#pragma once

template <class number>
class bit_vector
{
private:
	number *data;
	int		size, stride, total_bits;
public:
	void init(number *d, int total, int strid)
	{
		data		= d;
		total_bits	= total;
		stride		= strid;
		size		= (total_bits+num_bits()-1) / num_bits();
	}

	inline int num_bits() const 
	{
		return sizeof(number)*8;
	}

	void self_multiplied_matrix_full(float *result, const float weight,
		float *temp)
	{
		to_float_array(temp, weight);

		for (int i = 0; i < size; i++)
		{
			number	mask = 1, 
					v = data[i*stride];
			int bits = min(num_bits(), total_bits-num_bits()*i);
			for (int j = 0; j < bits; j++)
			{
				if (mask & v)
					memcpy(result, temp, total_bits*sizeof(float));
				else
					memset(result, 0, total_bits*sizeof(float));

				result += total_bits;
				mask += mask;
			}
		}
	}

	void self_multiplied_matrix(float *result, const float weight, float *temp)
	{
		to_float_array(temp, weight);

		int len = 1;
		for (int i = 0; i < size; i++)
		{
			number	mask = 1, 
					v = data[i*stride];
			int bits = min(num_bits(), total_bits-num_bits()*i);
			for (int j = 0; j < bits; j++)
			{
				if (mask & v)
					memcpy(result, temp, len*sizeof(float));
				else
					memset(result, 0, len*sizeof(float));

				result += len;
				mask += mask;
				len++;
			}
		}
	}

	void to_float_array(float *result, const float weight)
	{
		float *t = result;
		for (int i = 0; i < size; i++)
		{
			number	mask = 1,
					v = data[i*stride];
			int bits = min(num_bits(), total_bits-num_bits()*i);
			for (int l = 0; l < bits; l++)
			{
				*t = (mask & v) ? weight : 0;
				t++;
				mask += mask;
			}
		}
	}
};