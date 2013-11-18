#ifndef _COHERENT
#define _COHERENT
#include "../../MMDevice/MMDevice.h"
#include "../../MMDevice/DeviceBase.h"
#include "../../MMDevice/DeviceUtils.h"
#include <string>
#include <vector>
using namespace std;

class Coherent : public CShutterBase<Coherent>{
public:
	
	Coherent();
	~Coherent();

	int Initialize();
	int Shutdown();
	void GetName(char *name)const;
	bool Busy();


	void SetOn(long on);
	long GetOn();
	void SetPowerLevel(double d);
	double GetPowerLevel();
	std::string name_;
	
	int OnAddress(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPowerSet(MM::PropertyBase* pProp, MM::ActionType eAct, long power);
	int OnConnectionType(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnReceivedData(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnState(MM::PropertyBase* pProp, MM::ActionType eAct);
	std::string query(std::string query);
	int SetOpen(bool open = true);
	int GetOpen(bool &open);
	int Fire(double delta);
private:
	bool initialized;
	int err;
	std::string port;
	std::string buf_string_;
	unsigned char buf_[200];
	void Send(string cmd);
	MM::MMTime changedTime_;
	void Set(string cmd);

	void GenerateProperties();

	int Read();
};


#endif

