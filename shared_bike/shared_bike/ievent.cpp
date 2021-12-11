#include "ievent.h"

iEvent::iEvent(u32 eid, u32 sn)
{
	
	set_eid(eid);
	printf("!set_eid=%d\n\n");
}

iEvent::~iEvent()
{
}

u32 iEvent::generateSeqNo()
{
	static u32 sn = 0;
	return sn++;
}
