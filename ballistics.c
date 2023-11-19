/*---------------------------------------------------------------------------- /
/	IFOBS - ballistics.c													   /
/ ---------------------------------------------------------------------------- /
/	Mint Luc
/	Bowie Gian
/	Created: 2023-06-30
/	Modified: 2023-11-19
/
/	This file contains the functions that calculates ballistic trajectory.
/ ----------------------------------------------------------------------------*/

/*--------------------------------------------------------------*/
/* Include Files												*/
/*--------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

/*--------------------------------------------------------------*/
/* Global Variables				 								*/
/*--------------------------------------------------------------*/

const double gravity = 9.8;

/*--------------------------------------------------------------*/
/*  Static Function Implemetations								*/
/*--------------------------------------------------------------*/

static double calculateTerminalVelocity(double ballistic_coefficient_metric, double density) {
	double terminalVelocity = sqrt((2 * gravity * ballistic_coefficient_metric) / (density));
	return terminalVelocity;
}

// static double calculateTerminalVelocity(double drag, double density, double area, double mass)
// {
// 	// Calculate terminal velocity
// 	double terminalVelocity = sqrt((2 * mass * gravity) / (drag * density * area));
// 	return terminalVelocity;
// }

static double calculateTime(double v0, double vt, double elev_rad, double x) {
	double t = -(vt * log(1 - (x * gravity) / (v0 * vt * cos(elev_rad)))) / gravity;
	return t;
}

// static double calculateNormalTimeOfFlight(double phi, double x, double v0)
// {
// 	double eleAngle= phi * M_PI / 180.0; // Convert angle to radians
// 	double t = x/ (v0 * cos(eleAngle)); // Calculate time of flight
// 	return t;
// }

// Function to calculate the height given the time of flight, initial velocity, and elevation angle
// static double calculateNormalHeight(double t, double v0, double phi)
// {
// 	double eleAngle = phi * M_PI / 180.0; // Convert angle to radians
// 	double y = v0 * t * sin(eleAngle) - (0.5 * 9.8 * pow(t, 2)); // Calculate height
// 	return y;
// }

static double calculateTargetHeight(double h0, double x, double v0, double vt, double elev_red, double t)
{
	// Calculate the height of the target
	double target_height = h0 + (vt / gravity) * (v0 * sin(elev_red) + vt) * (1 - exp(-gravity * t / vt)) - vt * t;

	return target_height;
}

static double calculateMaxHeight(double v0, double vt, double elev_rad)
{
	// Calculate the time at which y is maximized (when the vertical velocity becomes zero)
	double t_max_height = (vt * sin(elev_rad)) / gravity;

	// Calculate the maximum height
	double max_height = (vt / gravity) * (v0 * sin(elev_rad) + vt) * (1 - exp(-gravity * t_max_height / vt)) - (vt * t_max_height);

	return max_height;
}

//  25m   0mm
//  50m  50mm
// 100m 250mm
static double calculateDrop(double distance_m, double elev_deg)
{
	double h0 = 0.0;  // Initial launch height in meters
	double v0 = 430;  // Initial velocity in m/s
	double elev_rad = elev_deg * M_PI / 180.0;  // Launch angle in degrees
	double density = 1.225;
	double ballistic_coefficient = 0.143;
	double ballistic_coefficient_metric = ballistic_coefficient * 703.1;

	double xDistance_m = distance_m * cos(elev_rad);
	double yAimHeight = distance_m * sin(elev_rad);

	double vt = calculateTerminalVelocity(ballistic_coefficient_metric, density);
	// double vt = calculateTerminalVelocity(0.08,0.37,0.01,0.01);
	// printf("Terminal velocity: %.2f v/m\n", vt);
	double t = calculateTime(v0, vt, elev_rad, xDistance_m);
	// printf("Time: %.2f \n", t);

	double target_height = calculateTargetHeight(h0, xDistance_m, v0, vt, elev_rad, t);
	double max_height = calculateMaxHeight(v0, vt, elev_rad);	
	double yDrop = target_height - yAimHeight;

	printf("Drop: %.3f meters\n", yDrop);
	// printf("Max height of the target: %.2f meters\n", max_height);

	return yDrop;
}

// Converts polar (r, theta) to (x, y) output to the pointers
static void polarTocartesian(double r, double theta_deg, double *x, double *y)
{
	double theta_rad = theta_deg * M_PI / 180.0;

	*x = r * cos(theta_rad);
	*y = r * sin(theta_rad);
}

/*--------------------------------------------------------------*/
/*  Function Implemetations										*/
/*--------------------------------------------------------------*/

void Ballistics_calculatePixelOffset(double distance_m, double elev_deg,
		double cant_deg, int *xOffset, int *yOffset)
{
	const double pixelWidth = 0.000254;
	const double xEyeToOptic = .05;
	const double yHeightOverBore = .06;

	double drop_m = calculateDrop(distance_m, elev_deg);
	
	double yOffset_m = 0;
	double zOffset_m = 0;
	polarTocartesian(drop_m, (cant_deg + 90.0), &zOffset_m, &yOffset_m);

	double xTotal = distance_m + xEyeToOptic;
	double yTotal = yOffset_m - yHeightOverBore;
	double zTotal = zOffset_m;

	double yScreen = yTotal / xTotal * xEyeToOptic;
	*yOffset = (int)round(yScreen / pixelWidth);

	double zScreen = zTotal / xTotal * xEyeToOptic;
	*xOffset = (int)round(zScreen / pixelWidth);

	//printf("%lf %lf %lf %d\r\n", xTotal, yTotal, yScreen, yPixelOffset);
	
	double yAngle = atan2(yOffset_m, distance_m);
	double zAngle = atan2(zOffset_m, distance_m);

	printf("X Offset: %7.3lf Y Offset: %7.3lf\r\n", zAngle, yAngle);
}
