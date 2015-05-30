//treesphere.h
//Author: Aven Bross
//
//Simple sphere class that holds node info for sphere trees

class TreeSphere
{
	public:
		TreeSphere(): _r(0.), _x(0.), _y(0.), _z(0.) {}
		TreeSphere(const TreeSphere & other): _r(other._r), _x(other._x), _y(other._y), _z(other._z) {}
		TreeSphere(double x, double y, double z, double r): _r(r), _x(x), _y(y), _z(z) {}
		double _r, _x, _y, _z;

		//Draw a wire frame of the sphere
		void draw() const{
			glPushMatrix();
			glTranslated(_x,_y,_z);
			glutWireSphere(_r, 16, 12);
			glPopMatrix();
		}
};
