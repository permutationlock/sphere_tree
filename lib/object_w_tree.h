//Object.h
//Author: Aven Bross
//
//Object classes and helper functions for sphere tree collision detection

#ifndef OBJECT_H
#define OBJECT_H

#include <math.h>
#include "treesphere.h"
#include "octree.h"

const int SPHERE_TREE_DEPTH = 2;

//Gets the magnitude of the given vector
double getMagnitude(double x, double y, double z){
	return sqrt(x*x + y*y + z*z);
}

class Object {
	public:
		Object(double x, double y, double z): _x(x), _y(y), _z(z), _xSpeed(0.), _ySpeed(0.), _zSpeed(0.), _xAccel(0.), _yAccel(0.), _zAccel(0.) {}
		
		//Draw function (is overloaded)
		virtual void draw(){}

		//Checks sphere trees for collisions
		bool checkCollision(const Object & other){
			return checkColRecursive(_spaceTree, other._spaceTree);
		}

		//Recursively goes through all node sphere 
		bool checkColRecursive(const OctreeNode & t1, const OctreeNode & t2){
			TreeSphere s1 = t1.getValue();
			TreeSphere s2 = t2.getValue();

			//Find the distance between the two spheres
			double xDist = s1._x-s2._x;
			double yDist = s1._y-s2._y;
			double zDist = s1._z-s2._z;
			double dist = getMagnitude(xDist, yDist, zDist);

			//If the spheres are colliding => if both are leaf nodes -> true, else -> keep looking
			if(dist<=s1._r+s2._r && t1._isThere){
				if(t1._children==NULL && t2._children==NULL) { return true; }
				for(int i=0; i<8; i++){
					if(t1._children==NULL){
						if(checkColRecursive(t1, *t2._children[i])) { return true; }
					}
					else if(t2._children==NULL){
						if(checkColRecursive(*t1._children[i], t2)) { return true; }
					}
					else{
						for(int j=0; j<8; j++){
							if(checkColRecursive(*t1._children[i], *t2._children[j])) { return true; }
						}
					}
				}
			}

			//Return false if no colliding leaf nodes
			return false;
		}

		//Returns the velocity of the object
		double getVel(){
			return getMagnitude(_xSpeed, _ySpeed, _zSpeed);
		}

		//Updates the sphere map upon object movement
		void updateSphereNode(OctreeNode & t, double xChange, double yChange, double zChange){
			//Update coordinates
			t.getValue()._x+=xChange;
			t.getValue()._y+=yChange;
			t.getValue()._z+=zChange;
			if(t._children==NULL) return;	//Stop if leaf node
			for(int i=0; i<8; i++){
				updateSphereNode(*t._children[i], xChange, yChange, zChange);
			}
		}

		//Returns distance between 2 objects
		double getDist(Object & other){
			double xDist = _x-other._x;
			double yDist = _y-other._y;
			double zDist = _z-other._z;
			return getMagnitude(xDist, yDist, zDist);
		}

		//Bounce function so objects have somethign to do if they collide
		void bounce(Object & other){
			double newvx, newvy, newvz;
			//Calculations for this object
			newvx=other._xSpeed;
			newvy=other._zSpeed;
			newvy=other._zSpeed;

			//Calculations for block other object
			other._xSpeed=_xSpeed;
			other._ySpeed=_ySpeed;
			other._ySpeed=_zSpeed;

			_xSpeed=newvx;
			_ySpeed=newvy;
			_zSpeed=newvz;
		}

		//Draw a wireframe of the sphere tree
		void drawSphereTree(){
			drawRecurse(_spaceTree);
		}

		//Goes through sphere tree calling draw if a sphere represents a collidable surface
		void drawRecurse(const OctreeNode & t){
			if(t._children!=NULL)
				for(int i=0; i<8; i++){
					drawRecurse(*t._children[i]);
				}
			else if(t._isThere) t.getValue().draw();
			return;
		}

		//Set the velocity of an object
		void setVel(double x, double y, double z){
			_xSpeed = x;
			_ySpeed = y;
			_zSpeed = z;
		}

		//Update position and velocity based on an elapsed time
		void update(double elapsedTime){
			_xSpeed += _xAccel * elapsedTime;
			_ySpeed += _yAccel * elapsedTime;
			_zSpeed += _zAccel * elapsedTime;

			_x += _xSpeed * elapsedTime;
			_y += _ySpeed * elapsedTime;
			_z += _zSpeed * elapsedTime;
		
			//If the object moved, update sphere map
			if(_xSpeed!=0. || _ySpeed!=0. || _zSpeed!=0.)
				updateSphereNode(_spaceTree, _xSpeed * elapsedTime, _ySpeed * elapsedTime, _zSpeed * elapsedTime);
		}

		//Helper bool to avoid counting same collision twice
		bool _colliding  = false;

	protected:
		double _x, _y, _z;	//Object's position
		double _xSpeed, _ySpeed, _zSpeed;	//Object's velocity
		double _xAccel, _yAccel, _zAccel;
		OctreeNode _spaceTree;
};

//Sphere object
class Sphere : public Object {
	public:
		//Constructor - Generates sphere tree (tree is depth 1, just a sphere)
		Sphere(double x, double y, double z, double r): Object(x,y,z), _r(r)
		{
			_spaceTree = OctreeNode(0, TreeSphere(x, y, z, r), true);
		}
		
		//Draw a solid sphere with appropriate size and location
		virtual void draw(){
			glTranslated(_x, _y, _z);
			glutSolidSphere(_r, 16, 12);
		}

	private:
		double _r;
};

//Cube object
class Cube : public Object {
	public:
		//Constructor - Generates sphere tree of depth SPHERE_TREE_DEPTH for the cube
		Cube(double x, double y, double z, double halfSide, int treeDepth): Object(x,y,z), _halfSide(halfSide)
		{
			if(treeDepth>=1){			
				_spaceTree = OctreeNode(1, TreeSphere(x,y,z,getMagnitude(halfSide, halfSide, halfSide)), true);
				buildTree(treeDepth-1, _spaceTree, halfSide, getMagnitude(halfSide, halfSide, halfSide));
			}
			else
				_spaceTree = OctreeNode(0, TreeSphere(x,y,z,getMagnitude(halfSide, halfSide, halfSide)), true);
		}

		//Draw solid cube with appropriate size and location
		virtual void draw(){
			glTranslated(_x, _y, _z);
			glutSolidCube(_halfSide*2);
		}

	private:
		//Recursively builds a sphere tree for the cube (levelsAfter => tree levels remaining after this one)
		void buildTree(int levelsAfter, OctreeNode & prevTree, double halfSide, double r){
			//Node type is leaf if last level of tree
			int nodeType = 1;
			if(levelsAfter==0) nodeType = 0;

			//Find values needed for creating child nodes
			double nHalfSide = halfSide/2.;
			double newr = r/2.;
			double x = prevTree.getValue()._x, y = prevTree.getValue()._y, z = prevTree.getValue()._z;

			//Create each child node
			prevTree._children[0] = new OctreeNode(nodeType, TreeSphere(x-nHalfSide, y-nHalfSide, z-nHalfSide, newr), true);
			prevTree._children[1] = new OctreeNode(nodeType, TreeSphere(x+nHalfSide, y-nHalfSide, z-nHalfSide, newr), true);
			prevTree._children[2] = new OctreeNode(nodeType, TreeSphere(x-nHalfSide, y+nHalfSide, z-nHalfSide, newr), true);
			prevTree._children[3] = new OctreeNode(nodeType, TreeSphere(x+nHalfSide, y+nHalfSide, z-nHalfSide, newr), true);
			prevTree._children[4] = new OctreeNode(nodeType, TreeSphere(x-nHalfSide, y-nHalfSide, z+nHalfSide, newr), true);
			prevTree._children[5] = new OctreeNode(nodeType, TreeSphere(x+nHalfSide, y-nHalfSide, z+nHalfSide, newr), true);
			prevTree._children[6] = new OctreeNode(nodeType, TreeSphere(x-nHalfSide, y+nHalfSide, z+nHalfSide, newr), true);
			prevTree._children[7] = new OctreeNode(nodeType, TreeSphere(x+nHalfSide, y+nHalfSide, z+nHalfSide, newr), true);

			//If not the last level, create child nodes for each of prevTree's child nodes
			if(levelsAfter>0)
				for(int i=0; i<8; i++){
					buildTree(levelsAfter-1, *prevTree._children[i], nHalfSide, newr);
				}
			return;
		}
		
		double _halfSide;	//Half the length of cube side
};


//Cube object
/*class Cylander : public Object {
	public:
		//Constructor - Generates sphere tree of depth SPHERE_TREE_DEPTH for the cube
		Cube(double x, double y, double z, double r, int treeDepth): Object(x,y,z), _r(r)
		{
			if(treeDepth>1){			
				_spaceTree = OctreeNode(1, TreeSphere(x,y,z,getMagnitude(length/2., r, 0.)), true);
				buildTree(treeDepth-1, _spaceTree, halfSide, getMagnitude(halfSide, halfSide, halfSide));
			}
			else
				_spaceTree = OctreeNode(0, TreeSphere(x,y,z,getMagnitude(length/2., r, 0.)), true);
		}

		//Draw solid cube with appropriate size and location
		virtual void draw(){
			glTranslated(_x, _y, _z);
			glutSolidCube(_halfSide*2);
		}
	private:
		//Recursively builds a sphere tree for the cube (levelsAfter => tree levels remaining after this one)
		void buildTree(int levelsAfter, OctreeNode & prevTree, double halfSide, double r){
			//Node type is leaf if last level of tree
			int nodeType = 1;
			if(levelsAfter==0) nodeType = 0;

			//Find values needed for creating child nodes
			double nHalfSide = halfSide/2.;
			double newr = r/2.;
			double x = prevTree.getValue()._x, y = prevTree.getValue()._y, z = prevTree.getValue()._z;

			//Create each child node
			prevTree._children[0] = new OctreeNode(nodeType, TreeSphere(x-nHalfSide, y-nHalfSide, z-nHalfSide, newr), true);
			prevTree._children[1] = new OctreeNode(nodeType, TreeSphere(x+nHalfSide, y-nHalfSide, z-nHalfSide, newr), true);
			prevTree._children[2] = new OctreeNode(nodeType, TreeSphere(x-nHalfSide, y+nHalfSide, z-nHalfSide, newr), true);
			prevTree._children[3] = new OctreeNode(nodeType, TreeSphere(x+nHalfSide, y+nHalfSide, z-nHalfSide, newr), true);
			prevTree._children[4] = new OctreeNode(nodeType, TreeSphere(x-nHalfSide, y-nHalfSide, z+nHalfSide, newr), true);
			prevTree._children[5] = new OctreeNode(nodeType, TreeSphere(x+nHalfSide, y-nHalfSide, z+nHalfSide, newr), true);
			prevTree._children[6] = new OctreeNode(nodeType, TreeSphere(x-nHalfSide, y+nHalfSide, z+nHalfSide, newr), true);
			prevTree._children[7] = new OctreeNode(nodeType, TreeSphere(x+nHalfSide, y+nHalfSide, z+nHalfSide, newr), true);

			//If not the last level, create child nodes for each of prevTree's child nodes
			if(levelsAfter>0)
				for(int i=0; i<8; i++){
					buildTree(levelsAfter-1, *prevTree._children[i], nHalfSide, newr);
				}
			return;
		}
		
		double _length;
		double _r;
};*/


#endif
