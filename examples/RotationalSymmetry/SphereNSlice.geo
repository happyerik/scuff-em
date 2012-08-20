//
// this is a modified version of SphereN.geo that outputs 
// only a single 2*Pi/N slice of a sphere surface
//
// homer reid 8/2012
//

//************************************************************
//* freely specifiable input parameters      
//************************************************************

R = 1.0;   // radius
 
l = R/5;   // meshing fineness

N = 5;     // degree of rotational symmetry

//************************************************************
//* derived parameters ***************************************
//************************************************************
Theta = 2*Pi / N;

//************************************************************
//************************************************************
//************************************************************
Point(1) = {  0,  0, 0,  l };
Point(2) = {  0,  0, R,  l };
Point(3) = {  R,  0, 0,  l };
Point(4) = {  0,  0, -R,  l };

Circle(1) = { 2, 1, 3 };
Circle(2) = { 3, 1, 4 };

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
ReturnLateralEntities=0;
out[] = Extrude { {0,0,1}, {0,0,0}, Theta} { Line{1, 2}; };
S1=out[1];
S2=out[4];
sl[0]=S1; // sl is a running list of surface indices
sl[1]=S2;
