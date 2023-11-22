/*---------------------------------------------------------------------------- /
/	IFOBS - ballistics.c													   /
/ ---------------------------------------------------------------------------- /
/	Mint Luc
/	Bowie Gian
/	Hong Shi
/	Created: 2023-06-30
/	Modified: 2023-11-16
/
/	This file contains the functions that calculates ballistic trajectory.
/
/	coordinate system
/
/	z yaw	^
/			|
/			|
/			|_______> y cant
/			/
/		   /
/		  /  x elevation
/		 v
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
const double v_muzzle = 390;  // Initial velocity in m/s
const double elev_bias_rad = 0.012; // angle between muzzle and red dot
// v_muzzle and elev_bias_rad will need to be tweaked during testing.

/*--------------------------------------------------------------*/
/*  Static Function Implemetations								*/
/*--------------------------------------------------------------*/

// initial velocity components
struct Vector {
	double x, y, z;
};

static struct Vector rotate_vector(double cant_rad, double elev_rad, double r)
{
	struct Vector result;

	// positive x is left
	// positive y is forward (to target)
	// positive z is up (to sky)

	result.x = r * sin(cant_rad) * sin(elev_rad);
	result.y = r * cos(elev_rad);
	result.z = r * cos(cant_rad) * sin(elev_rad);

	return result;

}

static double calculateTime(double v0, double target_dist) {
	// description: calculate the time of flight (eq 184)
	// input :
	//			v0 - inital velocity
	//			target_dist - distance to the target measured by range finder
	//
	// output:
	//			t  - expected time of flight

	double t = target_dist/(v0); // Calculate time of flight
	return t;
}



static double calculateTargetDisplacement(double v0, double g, double t)
{
	// description: Calculate the height of the target (e
	// input: 
	//			v0 - inital velocity,
	//			t - time of bullet flight
	//			g - gravity
	// output:
	//			target height - expected bullet drop / expected bullet displacement 
	//


	double target_height = (v0 * t) - (0.5 * g * pow(t, 2));

	return target_height;
}


//  25m   0mm
//  50m  50mm
// 100m 250mm
static struct Vector calculateDrop(double distance_m, double elev_rad, double cant_rad)
{

	struct Vector v0 = rotate_vector(cant_rad, elev_rad + elev_bias_rad, v_muzzle);
	struct Vector aimHeight = rotate_vector(cant_rad, elev_rad, distance_m);
	struct Vector offset; // calculation results is placed in offset vector



	double t = calculateTime(v0.y, aimHeight.y); // when range finder is unlocked distance is range finder reading projected onto y-axis
	//double t = calculateTime(v0.y, distance_m); // when range finder is locked distance is just range finder reading

	offset.y = 0; // theres only a LR bullet displacement and Up Down bullet displacement

	offset.z = calculateTargetDisplacement(v0.z, gravity, t) - aimHeight.z;

	offset.x = calculateTargetDisplacement(v0.x, 0, t) - aimHeight.x;
	

	//printf("Drop: %.3f meters, right: %.3f meters\n", offset.z, offset.x);

	return offset;
}

static struct Vector proj2screen(double cant_rad, double elevation_rad, double x, double z)
{
	// description: takes the caculated bullet drop and LR displacement and project it onto the screen plane
	// input:
	//			cant_rad
	//			elevation_rad
	//			z vertical bullet drop , drop is negative
	//			x LR displacement, right is positive 
	struct Vector result;



	// drop is lengthed based on screen angle (elevation)
	z = z / cos(elevation_rad);

	// rotate target displacement base on screen
	result.x = x * cos(cant_rad) + z * sin(cant_rad);
	result.z = -x * sin(cant_rad) + z * cos(cant_rad);
	result.y = 0; // y is not used in the screen vector
	return result;

}
/*--------------------------------------------------------------*/
/*  Function Implemetations										*/
/*--------------------------------------------------------------*/

void Ballistics_calculatePixelOffset(double distance_m, double elev_deg, double cant_deg, int *xOffset, int *zOffset)
{
	const double pixelWidth = 0.000254;
	const double EyeToOptic = .05; 
	const double HeightOverBore = .06;

	double elev_rad = elev_deg * M_PI / 180.0;  // Launch angle in degrees
	double cant_rad = cant_deg * M_PI / 180.0;

	struct Vector offset = calculateDrop(distance_m, elev_rad, cant_rad);

	double yTotal = distance_m + EyeToOptic;
	offset.z = offset.z - HeightOverBore;

	// convert drop distance to offset distance at eye to optic length
	offset.z = offset.z / yTotal * EyeToOptic;
	offset.x = offset.x / yTotal * EyeToOptic ;
	
	// project target drop (and LR displacement) onto the screen plane
	struct Vector screen = proj2screen(cant_rad, elev_rad, offset.x,offset.z);

	// convert m to pixels	

	*zOffset = (int)round(screen.z / pixelWidth);

	*xOffset = (int)round(screen.x / pixelWidth);

	return;
}



// // for testing 
// int main() {
// 	double distance_m = 100;
// 	double elev_deg = 0; // up is positive
// 	double cant_deg = 0; // right is positive
// 	int xOffset = 0;
// 	int zOffset = 0;

// 	// test parameters: distance = 100m, 
// 	// key: d = ~0.5m drop due to gravity or 1px, os = off screen 
// 	// test cases: cant    elev   | drop	    right     screen drop 		sreen right
// 	//				0		0	  |	-d          0            -d                  0
// 	//				0       45    | -d          0            -d					 0
// 	// 			    45		0     | -d          0          -0.707d 		     -0.707d
// 	//				90      0     | -d          0             0			        -d
// 	//              90     45     | -d         100           -os                -os
// 	//              45     45     | -d          0            -d                  os


// 	// Ballistics_calculatePixelOffset(distance_m, elev_deg, cant_deg, &xOffset, &zOffset);
// 	// printf("cant Deg: %0.1f right: %d px, up: %d px \n", cant_deg, xOffset, zOffset);


// 	/*
// 	for (int i = 0; i<90; i+=1){
// 		cant_deg = (double)i;
// 		Ballistics_calculatePixelOffset(distance_m, elev_deg, cant_deg, &xOffset, &zOffset);
// 		//printf("cant Deg: %f Drop: %d px, right: %d px \n", cant_deg, zOffset, xOffset);
// 		printf("%.1f, %d, %d\n", cant_deg, xOffset, zOffset);

// 	} 
// 	*/


//     return 0; 

// }  