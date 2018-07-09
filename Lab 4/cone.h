#pragma once
#include "object.h"
#include <algorithm>
class Cone :
	public Object
{
public:
	Cone();
	~Cone();
	double lineTest(double origin[3], double direction[3], double maxDistance);
	void getNormal(double point[3], double normal[3]);
	bool isInside(double point[3]);
};

