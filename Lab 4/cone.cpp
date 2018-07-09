#include "cone.h"
#include "general.h"
Cone::Cone()
{
}

Cone::~Cone()
{

}

double Cone::lineTest(double origin[3], double direction[3], double maxDistance)
{
	double a = pow(direction[0], 2) + pow(direction[1], 2) - pow(direction[2], 2);
	double b = 2 * (origin[0] * direction[0] + origin[1] * direction[1] - origin[2] * direction[2]);
	double c = pow(origin[0], 2) + pow(origin[1], 2) - pow(origin[2], 2);

	double discriminant = pow(b, 2) - (4 * a * c);
	if (discriminant >= 0)
	{
		double s = sqrt(discriminant);
		double sol1 = (-b - s) / (2 * a);
		if (sol1 > 0 && sol1 <= maxDistance && (origin[2] + sol1 * direction[2]) > 0) return sol1;
		double sol2 = (-b + s) / (2 * a);
		if (sol2 > 0 && sol2 <= maxDistance && (origin[2] + sol2 * direction[2]) > 0) return sol2;
	}
	return maxDistance;

	/*double alpha = pow(b, 2) - (4 * a * c);
	if (alpha < 0)
		return maxDistance;

	alpha = sqrt(alpha);

	double alpha1 = (-b - alpha) / (2 * a);
	double alpha2 = (-b + alpha) / (2 * a);
	if (alpha1 > 0 && alpha1 <= maxDistance)
	{
		if ((origin[2] + alpha1 * direction[2]) <= 0)
			return maxDistance;
		else
			return alpha1;
		
	}
	else if (alpha2 > 0 && alpha2 <= maxDistance)
	{
		if ((origin[2] + alpha2 * direction[2]) <= 0)
			return maxDistance;
		else
			return alpha2;

	}

	return maxDistance;*/
}

void Cone::getNormal(double point[3], double normal[3])
{
	double gradient[3] = { 2 * point[0], 2 * point[1], -2 * point[2] };
	assign(gradient, normal);
}

bool Cone::isInside(double point[3])
{
	//return pow(point[0], 2) + pow(point[1], 2) - pow(point[2], 2) <= 0 && point[2] <= 0;
	if (point[2] <= 0)
		return false;
	return point[0] * point[0] + point[1] * point[1] - point[2] * point[2] <= 0;
}