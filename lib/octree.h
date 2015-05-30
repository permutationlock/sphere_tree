//Octree.h
//Author: Aven Bross
//
//Simple octree class for making sphere trees

#ifndef OCTNODE
#define OCTNODE

class OctreeNode {
public:
  TreeSphere _value;
  OctreeNode ** _children;
  bool _isThere = false;

  OctreeNode(){
     _children = NULL;
  }

  OctreeNode(int nodeType, TreeSphere value, bool isThere): _value(value), _isThere(isThere) {
    if (nodeType == 0)
      _children = NULL;
    else {
      _children = new OctreeNode*[8]; 
      for (int i=0; i<8; i++)
	_children[i] = NULL;
    }
  }

  TreeSphere & getValue(){
	return _value;
  } 

  const TreeSphere & getValue() const{
	return _value;
  } 

  ~OctreeNode() {
    if (_children)
      for (int i=0; i<8; i++)
	delete _children[i];
  }
};
#endif
