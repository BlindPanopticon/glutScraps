#include <stdio.h> 

class movement{
public:
	float transAcceleration[3];
	float transVelocity[3];
	float rotAcceleration[3];
	float rotVelocity[3];
	movement(){
		for(int x = 0; x <3; x++){
			transAcceleration[x] = 0;
			transVelocity[x] = 0;
			rotAcceleration[x] = 0;
			rotVelocity[x] = 0;
		}
	}
};

class vertex{
private:
	float x, y, z, r, g, b;

public:
	vertex(float pos[3], float color[3]){
		x = pos[0];
		y = pos[1];
		z = pos[2];
		r = color[0];
		g = color[1];
		b = color[2];
	}

	void setPosition(float pos[3]){
		x = pos[0];
		y = pos[1];
		z = pos[2];
	}
	void setColor(float color[3]){
		r = color[0];
		g = color[1];
		b = color[2];
	}

	void printData(){
	printf("%f\t%f\t%f\t\t%f\t%f\t%f\n", x,y,z,r,g,b);
	}
};

class glutObject{
private:
	bool display;
	int objID;
	int type;
	float mass;
	movement motion;

public:
	glutObject(int id){
		display = true;
		objID = id;
		mass = 0;
	}

	void toggleDisplay(){
		display = !display;
	}
	void setType(int inType){
		type = inType;
	}
	void setMass(float inMass){
		mass = inMass;
	}
};
