//https://gist.github.com/jungle-cat/2321427

#include <opencv/cv.h>
 
#include <algorithm>
#include <iostream>
#include <limits>
#ifdef __DEBUG__
#include <cstdio>
#endif


 
static inline float distance(const float* a, const float* b, int n, bool simd)
{
	int j = 0; float d = 0.f;
#if CV_SSE
	if( simd )
	{
		float CV_DECL_ALIGNED(16) buf[4];
		__m128 d0 = _mm_setzero_ps(), d1 = _mm_setzero_ps();
 
		for( ; j <= n - 8; j += 8 )
		{
			__m128 t0 = _mm_sub_ps(_mm_loadu_ps(a + j), _mm_loadu_ps(b + j));
			__m128 t1 = _mm_sub_ps(_mm_loadu_ps(a + j + 4), _mm_loadu_ps(b + j + 4));
			d0 = _mm_add_ps(d0, _mm_mul_ps(t0, t0));
			d1 = _mm_add_ps(d1, _mm_mul_ps(t1, t1));
		}
		_mm_store_ps(buf, _mm_add_ps(d0, d1));
		d = buf[0] + buf[1] + buf[2] + buf[3];
	}
	else
#endif
	{
		for( ; j <= n - 4; j += 4 )
		{
			float t0 = a[j] - b[j], t1 = a[j+1] - b[j+1], t2 = a[j+2] - b[j+2], t3 = a[j+3] - b[j+3];
			d += t0*t0 + t1*t1 + t2*t2 + t3*t3;
		}
	}
 
	for( ; j < n; j++ )
	{
		float t = a[j] - b[j];
		d += t*t;
	}
	return d;
}
 
class KmeansBoxAssignInvoker
{
	public: 
		std::vector<cv::Vec2f> box;
 
		KmeansBoxAssignInvoker(const cv::Mat* data, int dims)
			: _M_dims(dims)
			, _M_data(data)
			, box(dims)
		{
			const float flmin = std::numeric_limits<float>::min();
			const float flmax = std::numeric_limits<float>::max();
			std::for_each(box.begin(), box.end(), [flmin, flmax](cv::Vec2f& x){x[0] = flmax; x[1] = flmin;});
		}
 
		KmeansBoxAssignInvoker(const KmeansBoxAssignInvoker& x, cv::Split)
			: _M_dims(x._M_dims)
			, _M_data(x._M_data)
			, box(x.box.size())
		{
			const float flmin = std::numeric_limits<float>::min();
			const float flmax = std::numeric_limits<float>::max();
			std::for_each(box.begin(), box.end(), [flmin, flmax](cv::Vec2f& x){x[0] = flmax; x[1] = flmin;});
		}
 
		void operator() (const cv::BlockedRange& range)
		{
			for (int i = range.begin(); i != range.end(); ++i) {
				const float* sample = _M_data->ptr<float>(i);
				for (int j = 0; j < _M_dims; ++j) {
					float v = sample[j];
					box[j][0] = std::min(box[j][0], v);
					box[j][1] = std::max(box[j][1], v);
				}
			}
		}
 
		void join(const KmeansBoxAssignInvoker& y)
		{
			CV_Assert(box.size() == y.box.size());
			std::transform(box.begin(), box.end(), y.box.begin(), box.begin(), 
					[](const cv::Vec2f& x, const cv::Vec2f& y)
					{
						return cv::Vec2f(std::min(x[0], y[0]), std::max(x[1], x[2]));
					});
		}
 
	private:
		int             _M_dims;
		const cv::Mat*  _M_data;
};
 
class KmeansCenterCalInvoker
{
	public:
		cv::Mat_<float>  centers;
		std::vector<int> counters;
 
		KmeansCenterCalInvoker(const cv::Mat* data, const int* labels, int K, int dims) 
			: _M_K(K)
			, _M_dims(dims)
			, _M_data(data)
			, _M_labels(labels)
			, counters(std::vector<int>(K))
	{
		centers = cv::Mat_<float>::zeros(_M_K, _M_dims);
		std::for_each(counters.begin(), counters.end(), [](int& x){x = 0;});
	}
 
		KmeansCenterCalInvoker(const KmeansCenterCalInvoker& x, cv::Split) 
			: _M_K(x._M_K)
			, _M_dims(x._M_dims)
			, _M_data(x._M_data)
			, _M_labels(x._M_labels)
			, counters(std::vector<int>(x.counters.size()))
	{
		centers = cv::Mat_<float>::zeros(x.centers.size());
		std::for_each(counters.begin(), counters.end(), [](int& x){x = 0;});
	}
 
		void operator() (const cv::BlockedRange& range)
		{
#ifdef __DEBUG__
			int start = range.begin(), end = range.end();
			std::printf("\tBlock Range: from %d to %d\n", start, end);
#endif
			for (auto it = range.begin(); it != range.end(); ++it) {
				const float* sample = _M_data->ptr<float>(it);
				int k = _M_labels[it];
				float * center = centers[k];
 
				int j;
				for (j = 0; j <= _M_dims - 4; j += 4) {
					float t0 = center[j] + sample[j];
					float t1 = center[j+1] + sample[j+1];
 
					center[j] = t0;
					center[j+1] = t1;
 
					t0 = center[j+2] + sample[j+2];
					t1 = center[j+3] + sample[j+3];
 
					center[j+2] = t0;
					center[j+3] = t1;
				}
				for( ; j < _M_dims; j++ )
					center[j] += sample[j];
				counters[k]++;
			}
		}
 
		void join(const KmeansCenterCalInvoker& y)
		{
			centers += y.centers;
			CV_Assert(counters.size() == y.counters.size());
			std::transform(
					counters.begin()
					, counters.end()
					, y.counters.begin()
					, counters.begin()
					, [](int x, int y){return x + y;}
					);
		}
	private:
		int             _M_K;
		int             _M_dims;
		const cv::Mat*  _M_data;
		const int*      _M_labels;
};
 
class KmeansLabelAssignInvoker
{
	public:
		double compactness;
		int* labels;
 
		KmeansLabelAssignInvoker(const cv::Mat* data, const cv::Mat* centers, int* label, int K, int dims, bool simd)
			: _M_simd(simd)
			, _M_dims(dims)
			, _M_K(K)
			, _M_data(data)
			, _M_centers(centers)
			, compactness(0)
			, labels(label)
	{
	}
 
		KmeansLabelAssignInvoker(const KmeansLabelAssignInvoker& x, cv::Split)
			: _M_simd(x._M_simd)
			, _M_dims(x._M_dims)
			, _M_K(x._M_K)
			, _M_data(x._M_data)
			, _M_centers(x._M_centers)
			, compactness(0)
			, labels(x.labels)
	{
	}
 
		void operator() (const cv::BlockedRange& range)
		{
			// std::printf("BlockedRange from %d to %d\n", range.begin(), range.end());
			for (auto i = range.begin(); i != range.end(); ++i) {
				const float* sample = _M_data->ptr<float>(i);
				int k_best = 0;
				double min_dist = std::numeric_limits<double>::max();
 
				for (int k = 0; k < _M_K; k++) {
					const float* center = _M_centers->ptr<float>(k);
					double dist = distance(sample, center, _M_dims, _M_simd);
 
					if ( min_dist > dist) {
						min_dist = dist;
						k_best = k;
					}
				}
				compactness += min_dist;
				labels[i] = k_best;
			}
		}
 
		void join(const KmeansLabelAssignInvoker& y)
		{
			compactness += y.compactness;
		}
 
	private:
		bool             _M_simd;
		int              _M_dims;
		int              _M_K;
		const cv::Mat*   _M_data;
		const cv::Mat*   _M_centers;
};
 
class CentPPDistInvoker
{
	public:
		float*     dist;
		double     sum;
 
		CentPPDistInvoker(const cv::Mat* data, float* OutDist, int rngidx, int dims, bool simd)
			: _M_simd(simd)
			, _M_dims(dims)
			, _M_rngIdx(rngidx)
			, _M_data(data)
			, dist(OutDist)
			, sum(0)
		{
		}
 
		CentPPDistInvoker(const CentPPDistInvoker& x, cv::Split)
			: _M_simd(x._M_simd)
			, _M_dims(x._M_dims)
			, _M_rngIdx(x._M_rngIdx)
			, _M_data(x._M_data)
			, dist(x.dist)
			, sum(0)
		{
		}
 
		void operator() (const cv::BlockedRange& range)
		{
			const float* rngsample = _M_data->ptr<float>(_M_rngIdx);
			for (auto i = range.begin(); i != range.end(); ++i) {
				const float* sample = _M_data->ptr<float>(i);
				float d = distance(rngsample, sample, _M_dims, _M_simd);
 
				dist[i] = d;
				sum += d;
			}
		}
 
		void join(const CentPPDistInvoker& y)
		{
			sum += y.sum;
		}
 
	private:
		const bool     _M_simd;
		const int      _M_dims;
		const int      _M_rngIdx;
		const cv::Mat* _M_data;
};
 
class CentPPMinDistInvoker
{
	public: 
		CentPPMinDistInvoker()
		{
		}
 
		CentPPMinDistInvoker(const CentPPMinDistInvoker& v, cv::Split)
		{
		}
 
		void operator() (const cv::BlockedRange& range)
		{
		}
 
		void join(const CentPPMinDistInvoker& y)
		{
		}
 
	private:
		const cv::Mat*   _M_data;
};
 
static void generateRandomCenter(const std::vector<cv::Vec2f>& box, float* center, cv::RNG& rng)
{
	size_t j, dims = box.size();
	float margin = 1.f/dims;
	for( j = 0; j < dims; j++ )
		center[j] = ((float)rng*(1.f+margin*2.f)-margin)*(box[j][1] - box[j][0]) + box[j][0];
}
 
/*
   k-means center initialization using the following algorithm:
   Arthur & Vassilvitskii (2007) k-means++: The Advantages of Careful Seeding
   */
static void generateCentersPP(const cv::Mat& _data, cv::Mat& _out_centers,
		int K, cv::RNG& rng, int trials)
{
	int i, j, k, dims = _data.cols, N = _data.rows;
	const float* data = _data.ptr<float>(0);
	int step = (int)(_data.step/sizeof(data[0]));
	std::vector<int> _centers(K);
	int* centers = &_centers[0];
	std::vector<float> _dist(N*3);
	float* dist = &_dist[0], *tdist = dist + N, *tdist2 = tdist + N;
	double sum0 = 0;
	bool simd = cv::checkHardwareSupport(CV_CPU_SSE);
 
	centers[0] = (unsigned)rng % N;
 
	for( i = 0; i < N; i++ )
	{
		dist[i] = distance(data + step*i, data + step*centers[0], dims, simd);
		sum0 += dist[i];
	}
 
#if 1
	CentPPDistInvoker  cp(&_data, dist, centers[0], dims, simd);
	cv::parallel_reduce(cv::BlockedRange(0, N), cp);
	std::cerr << "sum : " << cp.sum << "\t" << sum0 << std::endl;
#endif
 
	int64 p330 = cv::getTickCount();
	for( k = 1; k < K; k++ )
	{
		double bestSum = DBL_MAX;
		int bestCenter = -1;
 
		for( j = 0; j < trials; j++ )
		{
			double p = (double)rng*sum0, s = 0;
			for( i = 0; i < N-1; i++ )
				if( (p -= dist[i]) <= 0 )
					break;
			int ci = i;
			for( i = 0; i < N; i++ )
			{
				tdist2[i] = std::min(distance(data + step*i, data + step*ci, dims, simd), dist[i]);
				s += tdist2[i];
			}
 
			if( s < bestSum )
			{
				bestSum = s;
				bestCenter = ci;
				std::swap(tdist, tdist2);
			}
		}
		centers[k] = bestCenter;
		sum0 = bestSum;
		std::swap(dist, tdist);
	}
	int64 p360 = cv::getTickCount();
	std::cerr << "Time : " << (p360 - p330) / cv::getTickFrequency() << std::endl;
 
	for( k = 0; k < K; k++ )
	{
		const float* src = data + step*centers[k];
		float* dst = _out_centers.ptr<float>(k);
		for( j = 0; j < dims; j++ )
			dst[j] = src[j];
	}
}
 
double Kmeans( const cv::Mat& data, int K, cv::Mat& best_labels,
		cv::TermCriteria criteria, int attempts,
		int flags, cv::Mat* _centers )
{
	const int SPP_TRIALS = 3;
	int N = data.rows > 1 ? data.rows : data.cols;
	int dims = (data.rows > 1 ? data.cols : 1)*data.channels();
	int type = data.depth();
	bool simd = cv::checkHardwareSupport(CV_CPU_SSE);
 
	attempts = std::max(attempts, 1);
	CV_Assert( data.dims <= 2 && type == CV_32F && K > 0 );
 
	cv::Mat _labels;
	if( flags & CV_KMEANS_USE_INITIAL_LABELS )
	{
		CV_Assert( (best_labels.cols == 1 || best_labels.rows == 1) &&
				best_labels.cols*best_labels.rows == N &&
				best_labels.type() == CV_32S &&
				best_labels.isContinuous());
		best_labels.copyTo(_labels);
	}
	else
	{
		if( !((best_labels.cols == 1 || best_labels.rows == 1) &&
					best_labels.cols*best_labels.rows == N &&
					best_labels.type() == CV_32S &&
					best_labels.isContinuous()))
			best_labels.create(N, 1, CV_32S);
		_labels.create(best_labels.size(), best_labels.type());
	}
	int* labels = _labels.ptr<int>();
 
	cv::Mat centers(K, dims, type), old_centers(K, dims, type);
	std::vector<int> counters(K);
	std::vector<cv::Vec2f> _box(dims);
	cv::Vec2f* box = &_box[0];
 
	double best_compactness = DBL_MAX, compactness = 0;
	cv::RNG& rng = cv::theRNG();
	int a, iter, i, j, k;
 
	if( criteria.type & cv::TermCriteria::EPS )
		criteria.epsilon = std::max(criteria.epsilon, 0.);
	else
		criteria.epsilon = FLT_EPSILON;
	criteria.epsilon *= criteria.epsilon;
 
	// the max iteration is bigger than 2 and no more than 100
	if( criteria.type & cv::TermCriteria::COUNT )
		criteria.maxCount = std::min(std::max(criteria.maxCount, 2), 100);
	else
		criteria.maxCount = 100;
 
	if( K == 1 )
	{
		attempts = 1;
		criteria.maxCount = 2;
	}
 
	const float* sample = data.ptr<float>(0);
	for( j = 0; j < dims; j++ )
		box[j] = cv::Vec2f(sample[j], sample[j]);
 
	for( i = 1; i < N; i++ )
	{
		sample = data.ptr<float>(i);
		for( j = 0; j < dims; j++ )
		{
			float v = sample[j];
			box[j][0] = std::min(box[j][0], v);
			box[j][1] = std::max(box[j][1], v);
		}
	}
#ifdef __TBB__
	KmeansBoxAssignInvoker boxass(&data, dims);
	cv::parallel_reduce(cv::BlockedRange(0, N), boxass);
 
	for (int regi = 0 ; regi < _box.size() && regi < boxass.box.size(); regi++)
		if (_box[regi][0] != boxass.box[regi][0] || _box[regi][1] != boxass.box[regi][1])
			std::cerr << "fuck the box!\n";
#endif
 
	for( a = 0; a < attempts; a++ )
	{
//		std::cerr << "Starting " << a << " attempt\n";
		// each attempt to perform kmeans cluster
		double max_center_shift = DBL_MAX;
		for( iter = 0; iter < criteria.maxCount && max_center_shift > criteria.epsilon; iter++ )
		{
//			std::cerr << "\tStarting " << iter << " iter: " << max_center_shift << std::endl;
			swap(centers, old_centers);
 
			// if it's first iteration, will generate initial labels.
			// while first attempt will initialize lables.
			if( iter == 0 && (a > 0 || !(flags & cv::KMEANS_USE_INITIAL_LABELS)) )
			{
				// generating intial labels use either KMEANS_PP_CENTERS or KMEANS_USE_INITIAL_LABELS.
				if( flags & cv::KMEANS_PP_CENTERS )
					generateCentersPP(data, centers, K, rng, SPP_TRIALS);
				else
				{
					for( k = 0; k < K; k++ )
						generateRandomCenter(_box, centers.ptr<float>(k), rng);
				}
			}
			else
			{ /// otherwise use the previous labels.
				// check the exist labels whether the labels is valid.
				if( iter == 0 && a == 0 && (flags & cv::KMEANS_USE_INITIAL_LABELS) )
				{
					cv::parallel_for(
							cv::BlockedRange(0, N), 
							[&K, &labels](const cv::BlockedRange& range) 
							{
								for (int i = range.begin(); i != range.end(); ++i){
									CV_Assert((unsigned)labels[i] < (unsigned)K);
								}
							}
							);
					/*
					for( i = 0; i < N; i++ )
						CV_Assert( (unsigned)labels[i] < (unsigned)K );
					*/
				}
				int64 BTicks = cv::getTickCount();
 
				// compute centers
				centers = cv::Scalar(0);
#if 0
				for( k = 0; k < K; k++ )
					counters[k] = 0;

				// calculate the mean of the cluster as a center
				// serial version of cluster
				for( i = 0; i < N; i++ )
				{
					sample = data.ptr<float>(i);
					k = labels[i];
					float* center = centers.ptr<float>(k);
					for( j = 0; j <= dims - 4; j += 4 )
					{
						float t0 = center[j] + sample[j];
						float t1 = center[j+1] + sample[j+1];

						center[j] = t0;
						center[j+1] = t1;

						t0 = center[j+2] + sample[j+2];
						t1 = center[j+3] + sample[j+3];

						center[j+2] = t0;
						center[j+3] = t1;
					}
					for( ; j < dims; j++ )
						center[j] += sample[j];
					counters[k]++;
				}
#endif
 
#ifdef __TBB__
				// parallel version of cluster
				KmeansCenterCalInvoker calcenter(&data, labels, K, dims);
				cv::parallel_reduce(cv::BlockedRange(0, N), calcenter);
 
				centers = calcenter.centers;
				counters = calcenter.counters;
#endif
				int64 ETicks = cv::getTickCount();
//				std::cerr << "LOGINFO : " << (ETicks - BTicks) / cv::getTickFrequency() << std::endl;
 
				if( iter > 0 )
					max_center_shift = 0;
 
				for( k = 0; k < K; k++ )
				{
					float* center = centers.ptr<float>(k);
					if( counters[k] != 0 )
					{
						float scale = 1.f/counters[k];
						for( j = 0; j < dims; j++ )
							center[j] *= scale;
					}
					else
						generateRandomCenter(_box, center, rng);
 
					if( iter > 0 )
					{
						double dist = 0;
						const float* old_center = old_centers.ptr<float>(k);
						for( j = 0; j < dims; j++ )
						{
							double t = center[j] - old_center[j];
							dist += t*t;
						}
						max_center_shift = std::max(max_center_shift, dist);
					}
				}
			}
			
			int64 startlabelstick = cv::getTickCount();
#if 0
            // assign labels
            compactness = 0;
            for( i = 0; i < N; i++ )
            {
                sample = data.ptr<float>(i);
                int k_best = 0;
                double min_dist = DBL_MAX;

                for( k = 0; k < K; k++ )
                {
                    const float* center = centers.ptr<float>(k);
                    double dist = distance(sample, center, dims, simd);

                    if( min_dist > dist )
                    {
                        min_dist = dist;
                        k_best = k;
                    }
                }

                compactness += min_dist;
                labels[i] = k_best;
            }
#endif
			int64 startlabelstick2 = cv::getTickCount();
 
#ifdef __TBB__
			cv::Mat __labels(_labels.size(), _labels.depth());
			KmeansLabelAssignInvoker lb(&data, &centers, __labels.ptr<int>(), K, dims, simd);
			cv::parallel_reduce(cv::BlockedRange(0, N, 10000), lb);
			// _labels = __labels;
			__labels.copyTo(_labels);
			compactness = lb.compactness;
 
#endif
			int64 endlabelstick = cv::getTickCount();
 
//			std::cerr << "\tLOGINFO: Label Assign Time " 
//				<< (endlabelstick - startlabelstick2) / cv::getTickFrequency() << "\t" 
//				<< (startlabelstick2 - startlabelstick) / cv::getTickFrequency() << std::endl;
		}
 
		if( compactness < best_compactness )
		{
			best_compactness = compactness;
			if( _centers )
				centers.copyTo(*_centers);
			_labels.copyTo(best_labels);
		}
	}
 
	return best_compactness;
}
