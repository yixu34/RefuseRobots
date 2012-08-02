#include "main.hpp"

struct PacketType
{
	PacketType() :bytesSent(0), bytesReceived(0), packetsSent(0), packetsReceived(0) {}
	void sent(int size) { bytesSent += size; packetsSent++; }
	void received(int size) { bytesReceived += size; packetsReceived++; }
	
	int bytesSent, bytesReceived, packetsSent, packetsReceived;
};
static std::map<int,PacketType> packetLog;
static PacketType total;

static bool logTrafficEnabled = true;
static double lastUpdateTime=0;
static PacketType secondlyTotal;

void logTrafficIncoming(Packet *packet)
{
	int type = *((int*)packet->getData()+1);
	if(logTrafficEnabled) {
		total.received(packet->getSize());
		packetLog[type].received(packet->getSize());
		secondlyTotal.received(packet->getSize());
	}
}

void logTrafficOutgoing(Packet *packet)
{
	int type = *((int*)packet->getData()+1);
	if(logTrafficEnabled) {
		total.sent(packet->getSize());
		packetLog[type].sent(packet->getSize());
		secondlyTotal.sent(packet->getSize());
	}
}

static PacketType lastSecond;
void logTrafficStats(int &bytesSent, int &bytesRecv, int &packSent, int &packRecv)
{
	bytesSent = lastSecond.bytesSent;
	bytesRecv = lastSecond.bytesReceived;
	packSent  = lastSecond.packetsSent;
	packRecv  = lastSecond.packetsReceived;
	
	if(getTime() >= lastUpdateTime+1.0) {
		lastUpdateTime = getTime();
		lastSecond = secondlyTotal;
		secondlyTotal = PacketType();
	}
}



void logTrafficClear()
{
	total = PacketType();
	packetLog.clear();
}

void logTrafficSave(std::string filename)
{
	FILE *fout = fopen(filename.c_str(), "w");
	if(!fout) return;
	
	fprintf(fout, "Total sent: %i bytes in %i packets (%.1f avg size)\n"
	              "Total received: %i bytes in %i packets (%.1f avg size)\n",
		total.bytesSent, total.packetsSent, (float)total.bytesSent/(float)total.packetsSent,
		total.bytesReceived, total.packetsReceived, (float)total.bytesReceived/(float)total.packetsReceived);
	
	fprintf(fout, "     Type  Quantity  Size   Bytes\n");
	for(std::map<int,PacketType>::iterator ii=packetLog.begin(); ii!=packetLog.end(); ii++)
	{
		PacketType packet = ii->second;
		if(packet.bytesSent) {
			fprintf(fout, "Send %5i %9i %4.1f %i\n",
				ii->first, packet.packetsSent, (float)packet.bytesSent/(float)packet.packetsSent, packet.bytesSent);
		}
		if(packet.bytesReceived) {
			fprintf(fout, "Recv %5i %9i %4.1f %i\n",
				ii->first, packet.packetsReceived, (float)packet.bytesReceived/(float)packet.packetsReceived, packet.bytesReceived);
		}
	}
	
	fclose(fout);
}

