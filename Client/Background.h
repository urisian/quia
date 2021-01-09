#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "Object.h"
class CBackground : public CObject
{
public:
	explicit					CBackground			(std::string objectKey);
							   ~CBackground			(void);
		
					void		Initialize			(void);
					void		Update				(void);
					void		LateUpdate			(void);
					void		Release				(void);

};

#endif