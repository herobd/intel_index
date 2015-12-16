#include "corpus.h"
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
 ////////Copied from Rosettacode.org .. http://rosettacode.org/wiki/K-means%2B%2B_clustering
typedef struct { vector<double> v; int group; } point_t, *point;
double randf(double m)
{
	return m * rand() / (RAND_MAX - 1.);
}
inline double dist2(point a, point b, int S)
{
	double sum = 0;
    for (int i=0; i<S; i++)
        sum+=pow(a->v[i] - b->v[i],2);
	return sum;
}
 
inline int
nearest(point pt, point cent, int n_cluster, double *d2, int S)
{
	int i, min_i;
	point c;
	double d, min_d;
 
#	define for_n for (c = cent, i = 0; i < n_cluster; i++, c++)
	for_n {
		min_d = HUGE_VAL;
		min_i = pt->group;
		for_n {
			if (min_d > (d = dist2(c, pt, S))) {
				min_d = d; min_i = i;
			}
		}
	}
	if (d2) *d2 = min_d;
	return min_i;
}
 
void kpp(point pts, int len, point cent, int n_cent, int S)
{
#	define for_len for (j = 0, p = pts; j < len; j++, p++)
	int i, j;
	int n_cluster;
	double sum, *d = (double*)malloc(sizeof(double) * len);
 
	point p, c;
	cent[0] = pts[ rand() % len ];
	for (n_cluster = 1; n_cluster < n_cent; n_cluster++) {
		sum = 0;
		for_len {
			nearest(p, cent, n_cluster, d + j, S);
			sum += d[j];
		}
		sum = randf(sum);
		for_len {
			if ((sum -= d[j]) > 0) continue;
            cent[n_cluster].v.assign(pts[j].v.begin(),pts[j].v.end());
			break;
		}
	}
	for_len p->group = nearest(p, cent, n_cluster, 0,S);
	free(d);
}
 
point lloyd(point pts, int len, int n_cluster, int S)
{
	int i, j, min_i;
	int changed;
 
	point cent = new point_t[n_cluster];
    point p, c;
 
	/* assign init grouping randomly */
	//for_len p->group = j % n_cluster;
 
	/* or call k++ init */
	kpp(pts, len, cent, n_cluster, S);
 
	do {
		/* group element for centroids are used as counters */
        for_n { c->group = 0; c->v.assign(S,0); }
		for_len {
			c = cent + p->group;
			c->group++;
            for (int j=0; j<S; j++)
                c->v[j]+=p->v[j];
			//c->x += p->x; c->y += p->y;
		}
		for_n {
            for (int j=0; j<S; j++)
                c->v[j]/=c->group;
            //c->x /= c->group; c->y /= c->group;
        }
 
		changed = 0;
		/* find closest centroid of each point */
		for_len {
			min_i = nearest(p, cent, n_cluster, 0, S);
			if (min_i != p->group) {
				changed++;
				p->group = min_i;
			}
		}
	} while (changed > (len >> 10)); /* stop when 99.9% of points are good */
 
	for_n { c->group = i; }
 
	return cent;
}
////////////////////////////////////////////////////////

Corpus::Corpus()
{
    for (int c=0; c<codebook.size(); c++)
    {
        Vec3b color(rand()%256,rand()%256,rand()%256);
        colorTable.push_back(color);
    }
}

Corpus::Corpus(string savedPath)
{
    
    for (int c=0; c<codebook.size(); c++)
    {
        Vec3b color(rand()%256,rand()%256,rand()%256);
        colorTable.push_back(color);
    }
    
    ifstream file;
    file.open (savedPath,ios::in);
    string line;
    getline (file,line);
    int numOfPages = stoi(line);
    for (int n=0; n < numOfPages; n++)
    {
        getline (file,line);
        int width = stoi(line);
        getline (file,line);
        int height = stoi(line);
        
        CodedImage* ci = new CodedImage(width,height);
        
        for (int j=0; j<height; j++)
        {
            for (int i=0; i<width; i++)
            {
                getline (file,line);
                int val = stoi(line);
                ci->set(i,j,val);
            }
        }
        codedPages.push_back(ci);
        
    }
    file.close();
}

Corpus::~Corpus()
{
    for (FeaturizedImage* fi : pages)
        delete fi;
    for (CodedImage* ci : codedPages)
        delete ci;
}

void Corpus::addImage(Mat img)
{
    SIFTizedImage* si = new SIFTizedImage(img);
    pages.push_back(si);
}

void Corpus::addPrevImage(Mat img, int i)
{
    codedPages[i]->setImg(img);
}

void Corpus::generateCodebook(int codebook_size)
{
    int totalFeatures=0;
    for (int i=0; i < pages.size(); i++)
    {
        totalFeatures += pages[i]->height() * pages[i]->width();
    }
    
    int sampleCount = totalFeatures*CODEBOOK_SAMPLE_AMOUNT;
    cout << "sampling " << sampleCount << endl;
    
    //vector< vector<double> > sampledFVectors;
//    point sampledFVectors = (point_t*)malloc(sizeof(point_t) * sampleCount);
    point sampledFVectors = new point_t[sampleCount];
    for (int i=0; i <sampleCount; i++)
    {
        int page = rand() % pages.size();
        int row = rand() % pages[page]->height();
        int col = rand() % pages[page]->width();
        
//        sampledFVectors.push_back(pages[page]->get(col,row));
        sampledFVectors[i].v=pages[page]->get(col,row);
        
//        cout << "sample:[ ";
//        for (double f : sampledFVectors[i].v)
//            cout << f << ", "<<flush;
//        cout << " ]" <<  endl;
    }
    
//    Mat sampledFVMat(sampledFVectors.size(), sampledFVectors[0].size(), CV_32F);
//    for (size_t i = 0; i < sampledFVectors.size(); i++) {
//        for (size_t j = 0; j < sampledFVectors[i].size(); j++) {
//            sampledFVMat.at<float>(i, j) = (float)sampledFVectors[i][j];
//        }
//    }
    
//    Mat exemplars(/*4096*/20, sampledFVectors[0].size(), CV_32F);
//    TermCriteria crit(0,10,.99);
//    kmeans(sampledFVMat,/*4096*/20,Mat(),crit,3,KMEANS_RANDOM_CENTERS,exemplars);
    point centers= lloyd(sampledFVectors,sampleCount,codebook_size,pages[0]->featureLength());
    
    
    for (int i=0; i <codebook_size; i++)
    {
        codebook.push_back(centers[i].v);
//        cout << "center " << i << ": " << centers[i].v[0] << endl;
    }
    delete[] centers;
    delete[] sampledFVectors;
    
    int test=0;
    for (FeaturizedImage* fi : pages)
    {
        CodedImage *ci = new CodedImage(*fi,codebook);
        codedPages.push_back(ci);
    }
}

//void Corpus::codeImages()
//{
    
//}

void Corpus::saveQuantizedImage(string filePath, int pageNum)
{
    
    Mat output = codedPages[pageNum]->ref().clone();
    cvtColor( output, output, CV_GRAY2RGB );
    codedPages[pageNum]->performOverIndexes([&](int i, int j, int fi, int fj){
            output.at<Vec3b>(j,i)=colorTable[codedPages[pageNum]->get(fi,fj)];
            output.at<Vec3b>(j+1,i)=colorTable[codedPages[pageNum]->get(fi,fj)];
            output.at<Vec3b>(j,i+1)=colorTable[codedPages[pageNum]->get(fi,fj)];
            output.at<Vec3b>(j+1,i+1)=colorTable[codedPages[pageNum]->get(fi,fj)];
        });

//    for (int j =SIFT_EFFECTIVE_WINDOW_SIZE/2; j < output.size[0]-SIFT_EFFECTIVE_WINDOW_SIZE/2; j+=WINDOW_SIZE)
//    {
//        for (int i =SIFT_EFFECTIVE_WINDOW_SIZE/2; i < output.size[1]-SIFT_EFFECTIVE_WINDOW_SIZE/2; i+=WINDOW_SIZE)
//        {
//            output.at<Vec3b>(j,i)=colorTable[codedPages[pageNum]->get(i,j)];
//            output.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+j*WINDOW_SIZE+1,SIFT_EFFECTIVE_WINDOW_SIZE/2+i*WINDOW_SIZE )=colorTable[codedPages[pageNum]->get(i,j)];
//            output.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+j*WINDOW_SIZE ,SIFT_EFFECTIVE_WINDOW_SIZE/2+i*WINDOW_SIZE+1)=colorTable[codedPages[pageNum]->get(i,j)];
//            output.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+j*WINDOW_SIZE+1,SIFT_EFFECTIVE_WINDOW_SIZE/2+i*WINDOW_SIZE+1)=colorTable[codedPages[pageNum]->get(i,j)];
//        }
//    }
    imwrite( filePath, output );
}

void Corpus::saveCodedCorpus(string filePath)
{
    if (codebook.size()==0)
    {
        cout << "save error, corpus not encoded" <<endl;
        return;
    }
    
    ofstream file;
    file.open (filePath, ios::out);
    
    file << codedPages.size() << endl;
    for (CodedImage* ci : codedPages)
    {
        file << ci->width() << endl;
        file << ci->height() << endl;
        
        for (int j=0; j<ci->height(); j++)
        {
            for (int i=0; i<ci->width(); i++)
            {
                file << ci->get(i,j) << endl;
            }
        }
    }
    file.close();
}

void Corpus::saveCodeBook(string filePath)
{
    if (codebook.size()==0)
    {
        cout << "save error, corpus not encoded" <<endl;
        return;
    }
    
    codebook.save(filePath);
}

void Corpus::readCodeBook(string filePath)
{
    
    codebook.readIn(filePath);
}

void Corpus::makeHeatMaps(Mat &query)
{
    HMMQuery q(query,&codebook,colorTable);
//    return;
    for (CodedImage* ci : codedPages)
    {
        Mat heatMap;
        cvtColor(ci->ref(),heatMap,CV_GRAY2RGB);
        for (int col=0; col<=ci->width()-q.width(); col++)
            for (int row=0; row<=ci->height()-q.height(); row++)
            {
                vector< vector<int> > candidate(q.width());
                for (int term=0; term<q.width(); term++)
                {
                    for (int f=0; f<q.height(); f++)
                    {
                        candidate[term].push_back(ci->get(col+term,row+f));
                    }
                }
                double prob = q.eval(candidate);
//                cout << "at ("<<col<<","<<row<<") = "<<prob<<endl;
                Vec3b color(255*(1-prob),500*(.5-fabs(prob-.5)),255*prob);
                
                
                
                heatMap.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+row*WINDOW_SIZE,SIFT_EFFECTIVE_WINDOW_SIZE/2+col*WINDOW_SIZE)=color;
                heatMap.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+row*WINDOW_SIZE+1,SIFT_EFFECTIVE_WINDOW_SIZE/2+col*WINDOW_SIZE)=color;
                heatMap.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+row*WINDOW_SIZE,SIFT_EFFECTIVE_WINDOW_SIZE/2+col*WINDOW_SIZE+1)=color;
                heatMap.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+row*WINDOW_SIZE+1,SIFT_EFFECTIVE_WINDOW_SIZE/2+col*WINDOW_SIZE+1)=color;
            }
        imshow("prob map", heatMap);
        waitKey();
    }
}

void Corpus::makeHeatMap(Mat &query, int page)
{
    HMMQuery q(query,&codebook,colorTable);
//    return;
    CodedImage* ci = codedPages[page];
    {
        Mat heatMap;
        cvtColor(ci->ref(),heatMap,CV_GRAY2RGB);
        for (int col=0; col<=ci->width()-q.width(); col++)
            for (int row=0; row<=ci->height()-q.height(); row++)
            {
                vector< vector<int> > candidate(q.width());
                for (int term=0; term<q.width(); term++)
                {
                    for (int f=0; f<q.height(); f++)
                    {
                        candidate[term].push_back(ci->get(col+term,row+f));
                    }
                }
                double prob = q.eval(candidate);
//                cout << "at ("<<col<<","<<row<<") = "<<prob<<endl;
                Vec3b color(255*(1-prob),500*(.5-fabs(prob-.5)),255*prob);
                heatMap.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+row*WINDOW_SIZE,SIFT_EFFECTIVE_WINDOW_SIZE/2+col*WINDOW_SIZE)=color;
                heatMap.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+row*WINDOW_SIZE+1,SIFT_EFFECTIVE_WINDOW_SIZE/2+col*WINDOW_SIZE)=color;
                heatMap.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+row*WINDOW_SIZE,SIFT_EFFECTIVE_WINDOW_SIZE/2+col*WINDOW_SIZE+1)=color;
                heatMap.at<Vec3b>(SIFT_EFFECTIVE_WINDOW_SIZE/2+row*WINDOW_SIZE+1,SIFT_EFFECTIVE_WINDOW_SIZE/2+col*WINDOW_SIZE+1)=color;
            }
        imshow("prob map", heatMap);
        waitKey();
    }
}
