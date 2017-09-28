/**
 @file 		Printable.h
 @author	kmurphy
 @practical
 @brief		Utility class to simplify logging of entiies in a physical simuation.
 */

#ifndef PRINTABLE_H
#define PRINTABLE_H

#include <iostream>
#include <sstream>

namespace YAMPE {
    
typedef std::string String;

class Printable {
    
public:
	Printable(String label) : m_label(label) { };
    
	Printable& setLabel(String label);
	String label() const;
    
	virtual const String toString() const = 0;
	friend std::ostream& operator <<(std::ostream& outputStream, const Printable& p);
    
private:
	String m_label;
};
    
}	// namespace YAMPE

#endif