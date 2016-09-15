#ifndef GENOME_H
#define GENOME_H

//base interface for a Genome
#include <Windows.h>

class Genome
{
public:
	Genome() {};
	virtual ~Genome() {};

	// read data from an organism
	virtual void FromOrganism(const void* pOrg) = NULL;

	// fill data into an organism (organism assumed preallocated)
	virtual void ToOrganism(void* pOrg) const = NULL;

	// mutate
	virtual void Mutate() = NULL;

	//mate
	virtual void CrossOverWith(const Genome* pSoulMate, Genome* pBaby1, Genome* pBaby2) = NULL;
};


#endif