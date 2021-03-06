#include "seam_carv_random.h"



int get_random_int_in_range(int inf, int sup){
	return inf + rand() % (1 + sup - inf);
}

int get_next_weighted(double x, double y, double z){

	double w1, w2, w3;
	if (x == -1){
		w1 = 0;
		w2 = 1 / (y + 1);
		w3 = 1 / (z + 1);
	}
	else if (y == -1){
		w1 = 1 / (x + 1);
		w2 = 0;
		w3 = 1 / (z + 1);
	}
	else if (z == -1){
		w1 = 1 / (x + 1);
		w2 = 1 / (y + 1);
		w3 = 0;
	}
	else{
		w1 = 1 / (x + 1);
		w2 = 1 / (y + 1);
		w3 = 1 / (z + 1);
	}



	double r = (double)rand() / RAND_MAX;
	double weight = w1 + w2 + w3;
	double t1 = w1 / weight;
	double t2 = (w1 + w2) / weight;
	if (r < t1){
		return -1;
	}
	else if (r < t2){
		return 0;
	}
	else{
		return 1;
	}
}

double safe_get(const Mat& E, int y, int x){
	if (y >= E.size().height || y < 0 || x >= E.size().width || x < 0){
		return -1;
	}
	else{
		return static_cast<double>(E.at<uchar>(y, x));
	}
}

int which_min_paul(double x, double y, double z){
	if (min(x, y) == x && min(x, z) == x){
		return -1;
	}
	if ((min(x, y) == y && min(y, z) == y)){
		return 0;
	}
	else{
		return 1;
	}
}

Path random_walk_x(const Mat& I){
	Path ret;
	Vector<Pixel> path(I.size().height);
	ret.path = path;

	ret.path[0].x = get_random_int_in_range(0, I.size().width - 1);
	ret.path[0].y = 0;

	ret.energy = safe_get(I, ret.path[0].y, ret.path[0].x);
	for (int y = 1; y < I.size().height; ++y){

		ret.path[y].x = ret.path[y - 1].x + get_next_weighted(safe_get(I, y, ret.path[y - 1].x - 1), safe_get(I, y, ret.path[y - 1].x), safe_get(I, y, ret.path[y - 1].x + 1));// get_random_int_in_range(-1, 1);
		//check for bounds
		if (ret.path[y].x >= I.size().width){
			ret.path[y].x = I.size().width - 1;
		}
		else if (ret.path[y].x < 0){
			ret.path[y].x = 0;
		}

		ret.path[y].y = y;

		ret.energy += safe_get(I, ret.path[y].y, ret.path[y].x);
	}

	return ret;
}

Path random_walk_y(const Mat& I){
	Path ret;
	Vector<Pixel> path(I.size().width);
	ret.path = path;

	ret.path[0].y = get_random_int_in_range(0, I.size().height - 1);
	ret.path[0].x = 0;

	ret.energy = safe_get(I, ret.path[0].y, ret.path[0].x);
	for (int x = 1; x < I.size().width; ++x){

		ret.path[x].y = ret.path[x - 1].y + get_next_weighted(safe_get(I, ret.path[x - 1].y - 1, x), safe_get(I, ret.path[x - 1].y, x), safe_get(I, ret.path[x - 1].y + 1, x));
		//check for bounds
		if (ret.path[x].y >= I.size().height){
			ret.path[x].y = I.size().height - 1;
		}
		else if (ret.path[x].y < 0){
			ret.path[x].y = 0;
		}

		ret.path[x].x = x;

		ret.energy += safe_get(I, ret.path[x].y, ret.path[x].x)*safe_get(I, ret.path[x].y, ret.path[x].x);
	}

	return ret;
}

Path min_energy_path(const Vector<Path>& V){
	Path ret = V[0];
	double min = ret.energy;
	for (int k = 1; k < V.size(); ++k){
		if (V[k].energy < min){
			ret = V[k];
			min = V[k].energy;
		}
	}
	return ret;
}

Path random_carv_x(const Mat& E, int nb_tries){

	Vector<Path> paths(nb_tries);
	for (int k = 0; k < nb_tries; k++){
		paths[k] = random_walk_x(E);
	}

	return min_energy_path(paths);
}

Path random_carv_y(const Mat& E, int nb_tries){

	Vector<Path> paths(nb_tries);
	for (int k = 0; k < nb_tries; k++){
		paths[k] = random_walk_y(E);
	}

	return min_energy_path(paths);
}

Mat show_path(const Mat& src, Path p){
	Mat ret;
	cvtColor(src, ret, COLOR_GRAY2RGB);


	for (int k = 0; k < p.path.size(); ++k){
		ret.at<Vec3b>(p.path[k].y, p.path[k].x) = Vec3b(0, 0, 255);
	}

	return ret;
}

Mat show_all_path(const Mat& src){
	srand(time(NULL));
	Mat ret;
	cvtColor(src, ret, COLOR_GRAY2RGB);
	Mat energy = get_energy(src);
	Path p;
	for (int l = 0; l < 100; ++l){
		//Vector<Path> paths(100);
		//for (int k = 0; k < 100; k++){
		//	paths[k] = random_walk_y(energy);
		//	//for (int l = 0; l < paths[k].path.size(); ++l){
		//	//	ret.at<Vec3b>(paths[k].path[l].y, paths[k].path[l].x) = Vec3b(0, 0, 255);
		//	//}
		//}
		//p = min_energy_path(paths);
		p = random_carv_x(energy, 100);
		for (int k = 0; k < p.path.size(); ++k){
			ret.at<Vec3b>(p.path[k].y, p.path[k].x) = Vec3b(0, 255, 255);
		}
	}
	return ret;
}



void carve(Mat& src, int d_rows, int d_cols, int nb_tries){
	int delta_r = src.rows - d_rows;
	int delta_c = src.cols - d_cols;
	Path seam;
	Mat I;
	cvtColor(src, I, CV_RGB2GRAY);
	Mat energy = get_energy(I);

	if (delta_r > 0){
		for (int r = 0; r < delta_r; ++r){
			seam = random_carv_y(energy, nb_tries);
			carve_y(src, seam, nb_tries);
			e_carve_y(energy, seam, nb_tries);
		}
	}
	if (delta_c > 0){

		for (int c = 0; c < delta_c; ++c){
			seam = random_carv_x(energy, nb_tries);
			carve_x(src, seam, nb_tries);
			e_carve_x(energy, seam, nb_tries);
		}
	}
	if (delta_r < 0){
		for (int r = 0; r > delta_r; --r){
			seam = random_carv_y(energy, nb_tries);
			add_y(src, seam, nb_tries);
			e_add_y(energy, seam, nb_tries);
        }
	}
	if (delta_c < 0){

		for (int c = 0; c > delta_c; --c){
			seam = random_carv_x(energy, nb_tries);
			add_x(src, seam, nb_tries);
			e_add_x(energy, seam, nb_tries);
		}
	}

}



void resize_seam_carv_random(Mat& src, Size wanted, int nb_tries){
    int n_rows = wanted.height;
    int n_cols = wanted.width;

	carve(src, n_rows, n_cols, nb_tries);
}