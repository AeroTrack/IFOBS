#include <stdio.h>
#include <math.h>

const double gravity = 9.8;  // Global variable for gravity


double calculateTerminalVelocity(double bullet_coefficient, double density) {


    // Calculate terminal velocity
    double terminalVelocity = sqrt((2 * gravity * bullet_coefficient) / (density));

    return terminalVelocity;
}


// double calculateTerminalVelocity(double drag, double density, double area, double mass) {


//     // Calculate terminal velocity
//     double terminalVelocity = sqrt((2 * mass * gravity) / (drag * density * area));

//     return terminalVelocity;
// }



double calculateTime(double v0, double vt, double g, double phi, double x) {
    

    double eleAngle = phi * M_PI / 180.0;
    
    double t = -(vt * log(1 - (x * g) / (v0 * vt * cos(eleAngle)))) / g;
    return t;

}



// double calculateNormalTimeOfFlight(double phi, double x, double v0) {
//     double eleAngle= phi * M_PI / 180.0; // Convert angle to radians
//     double t = x/ (v0 * cos(eleAngle)); // Calculate time of flight
//     return t;
// }

// // Function to calculate the height given the time of flight, initial velocity, and elevation angle
// double calculateNormalHeight(double t, double v0, double phi) {
//     double eleAngle = phi * M_PI / 180.0; // Convert angle to radians
//     double y = v0 * t * sin(eleAngle) - (0.5 * 9.8 * pow(t, 2)); // Calculate height
//     return y;
// }




double calculateTargetHeight(double h0, double x, double v0, double vt, double phi, double t) {
    const double g = 9.8;  // Gravitational acceleration on Earth

    // Convert launch angle to radians
    double eleAngle = phi * M_PI / 180.0;


    // Calculate the height of the target
    double target_height = h0 + (vt / g) * (v0 * sin(eleAngle) + vt) * (1 - exp(-g * t / vt)) - vt * t;

    return target_height;
}


double calculateMaxHeight(double v0, double vt, double phi) {
    const double g = 9.8;  // Gravitational acceleration on Earth

    // Convert launch angle to radians
    double phi_rad = phi * M_PI / 180.0;

    // Calculate the time at which y is maximized (when the vertical velocity becomes zero)
    double t_max_height = (vt * sin(phi_rad)) / g;

    // Calculate the maximum height
    double max_height = (vt / g) * (v0 * sin(phi_rad) + vt) * (1 - exp(-g * t_max_height / vt)) - (vt * t_max_height);

    return max_height;
}

double ballisticsTest(double distance_m) {

    double h0 = 0.0;  // Initial launch height in meters
    double v0 = 330;  // Initial velocity in m/s
    double phi = 0.0;  // Launch angle in degrees
    double density = 1.225;
    double bullet_coefficient = 0.1;


    double bullet_coefficient_matrix = bullet_coefficient*703.1;


    double vt = calculateTerminalVelocity(bullet_coefficient_matrix, density);
    // double vt = calculateTerminalVelocity(0.08,0.37,0.01,0.01);
    printf("Terminal velocity: %.2f v/m\n", vt);
    double t = calculateTime(v0,vt,gravity,phi,distance_m);
    printf("Time: %.2f \n", t);

    double target_height = calculateTargetHeight(h0, distance_m, v0, vt, phi,t);
    

    double max_height = calculateMaxHeight(v0,vt,phi);	
    printf("Height of the target: %.2f meters\n", target_height);
    printf("Max height of the target: %.2f meters\n", max_height);
    return target_height;
}

int calculatePixelOffset(double xDistance, double yDrop)
{
	const double pixelWidth = 0.000254;
	double xEyeToOptic = .2;
	double yHeightOverBore = .05;

	double xTotal = xDistance + xEyeToOptic;
	double yTotal = yDrop + yHeightOverBore;

	double yScreen = yTotal / xTotal * xEyeToOptic;
	int pixelOffset = (int)round(yScreen / pixelWidth);

	//printf("%lf %lf %lf %d\r\n", xTotal, yTotal, yScreen, pixelOffset);
	return pixelOffset;
}
