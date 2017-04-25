#include "Object.h"

Object::Object()
{
	//set values for default constructor
	setType("Object");
	setColor(Scalar(0, 0, 0));
}

Object::Object(string name) {

	setType(name);

	if (name == "blue") 
	{
		//BGR pre modru
		setColor(Scalar(255, 0, 0));
	}

	if (name == "green") 
	{
		//BGR pre zelenu
		setColor(Scalar(0, 255, 0));	
	}
	if (name == "yellow") 
	{
		//BGR pre zltu
		setColor(Scalar(0, 255, 255));
	}
	if (name == "red") 
	{
		//BGR pre cervenu
		setColor(Scalar(0, 0, 255));
	}
}

Object::~Object(void)
{
}

int Object::getXPos() 
{
	return Object::xPos;
}

void Object::setXPos(int x) 
{
	Object::xPos = x;
}

int Object::getYPos() 
{
	return Object::yPos;
}

void Object::setYPos(int y) 
{
	Object::yPos = y;
}

Scalar Object::setHSVmin(Scalar min) 
{
	 return Object::HSVmin = min;
}

Scalar Object::setHSVmax(Scalar max) 
{
	return Object::HSVmax = max;
}


