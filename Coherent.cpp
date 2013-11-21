
#ifdef WIN32
#include <windows.h>
#define snprintf _snprintf 
#endif

#include "../../MMDevice/MMDevice.h"
#include "Coherent.h"
#include <string>
#include <math.h>
#include "../../MMDevice/ModuleInterface.h"
#include "../../MMDevice/DeviceUtils.h"
#include <sstream>



const char* DEVICE_NAME = "OBIS Coherent";

MODULE_API void InitializeModuleData(){
	AddAvailableDeviceName(DEVICE_NAME, "OBIS Coherent Laser");

}

MODULE_API MM::Device* CreateDevice(const char * name){
	if(!name) return 0;
	if(strcmp(name, DEVICE_NAME) == 0){
		return new Coherent;
	}
	return 0;
}

MODULE_API void DeleteDevice(MM::Device *pDevice){
	delete pDevice;
}

Coherent::Coherent():
	err(0),
	initialized(0)
{
	name_ = DEVICE_NAME;
	CreateProperty(MM::g_Keyword_Name, 	DEVICE_NAME,MM::String, true);
	CreateProperty(MM::g_Keyword_Description, "Coherent OBIS Laser", MM::String, true);
	CPropertyAction * pAct = new CPropertyAction(this, &Coherent::OnPort);
	CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String,false, pAct, true);
	EnableDelay();
	UpdateStatus();
	
}


void Coherent::GenerateProperties(){
	CPropertyAction* pAct = new CPropertyAction(this, &Coherent::OnState);
	CreateProperty(MM::g_Keyword_State, "0", MM::Integer, false, pAct);
	AddAllowedValue(MM::g_Keyword_State, "0");
	AddAllowedValue(MM::g_Keyword_State, "1");

	CPropertyActionEx * powerAct = new CPropertyActionEx(this, &Coherent::OnPowerSet, 0);
	CreateProperty("Power level", "0", MM::Float, false, powerAct);

	// read only

	pAct = new CPropertyAction(this, &Coherent::OnCurrentLevel);
	CreateProperty("Current Level", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &Coherent::OnDiodeTemp);
	CreateProperty("Diode Temp", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &Coherent::OnBPTemp);
	CreateProperty("Base Plate Temp", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &Coherent::OnMaxPower);
	CreateProperty("Max Power Level", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &Coherent::OnDTempLow);
	CreateProperty("Diode Temp Lower Limit", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &Coherent::OnDTempHigh);
	CreateProperty("Diode Temp Upper Limit", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &Coherent::OnBPTempLow);
	CreateProperty("Base Plate Temp Lower Limit", "", MM::Float, true, pAct);

	pAct = new CPropertyAction(this, &Coherent::OnBPTempHigh);
	CreateProperty("Base Plate Temp Upper Limit", "", MM::Float, true, pAct);

}

Coherent::~Coherent(){
	Shutdown();
}

int Coherent::Initialize(){
	GenerateProperties();
	return 0;
}



bool Coherent::Busy(){
	MM::MMTime interval = GetCurrentMMTime() - changedTime_;
	MM::MMTime delay(GetDelayMs()*1000.0);
	if (interval < delay)
		return true;
	else
		return false;
}

int Coherent::Shutdown(){

	return 0;
}

void Coherent::GetName(char * name) const
{
	CDeviceUtils::CopyLimitedString(name, name_.c_str());
}

void Coherent::SetOn(long on){
	std::string cmd = "SOUR:AM:STAT ";
	cmd += on == 0?"OFF":"ON";
	Set(cmd);
}

long Coherent::GetOn(){
	string res = query("SOUR:AM:STAT?");
	if(res == "ON")
		return 1;
	else
		return 0;
}

void Coherent::SetPowerLevel(double d){
	string cmd = "SOUR:POW:LEV:IMM:AMPL ";
	std::ostringstream strs;
	strs << cmd << d;
	Set(strs.str());
}

double Coherent::GetPowerLevel(){
	return Get("SOUR:POW:LEV?");
}

double Coherent::Get(string cmd){
	string res = query(cmd);
	return ::atof(res.c_str());
}
int Coherent::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::BeforeGet)
	{
		pProp->Set(port.c_str());
	}
	else if (eAct == MM::AfterSet)
	{
		if (initialized)
		{
			// revert
			pProp->Set(port.c_str());
			return -1;
		}

		pProp->Get(port);
	}

	return 0;
}

int Coherent::OnState(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		long on = GetOn();
		pProp->Set(on);
	}
	else if(eAct == MM::AfterSet){
		long reqOn;
		pProp->Get(reqOn);
		SetOn(reqOn);
	}
	return 0;
}

int Coherent::OnPowerSet(MM::PropertyBase* pProp, MM::ActionType eAct, long power){
	double level;
	if(eAct == MM::BeforeGet){
		level = GetPowerLevel();
		pProp->Set(level);
	}
	else if(eAct == MM::AfterSet){
		pProp->Get(level);
		SetPowerLevel(level);
	}
	return 0;
}


int Coherent::OnCurrentLevel(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:POW:CURR?"));

	}
	return 0;
}
int Coherent::OnDiodeTemp(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:DIOD? C"));
	}
	return 0;
}
int Coherent::OnBPTemp(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:BAS? C"));
	}
	return 0;
}
int Coherent::OnMaxPower(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:POW:LIM:HIGH?"));
	}
	return 0;
}
int Coherent::OnDTempLow(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:PROT:DIOD:LOW? C"));
	}
	return 0;
}
int Coherent::OnDTempHigh(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:PROT:DIOD:HIGH? C"));
	}
	return 0;
}
int Coherent::OnBPTempLow(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:PROT:BAS:LOW? C"));
	}
	return 0;
}
int Coherent::OnBPTempHigh(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:PROT:BAS:HIGH? C"));
	}
	return 0;
}
std::string Coherent::query(string q){

	std::string result;
	std::stringstream msg;
	err = (PurgeComPort(port.c_str()));
	Send(q);
	Read();
	string buf = buf_string_;
	Read();
	return buf;

}

void Coherent::Set(string cmd){
	err = (PurgeComPort(port.c_str()));
	Send(cmd);
	Read();
}

int Coherent::Read(){
	int ret = GetSerialAnswer(port.c_str(),"\r", buf_string_);
	if(ret!= DEVICE_OK)
		return ret;
	std::ostringstream msg;
	msg <<"Coherent::Read()		" << buf_string_;
	LogMessage(msg.str().c_str(), true);
	return DEVICE_OK;
}

void Coherent::Send(string cmd){
	std::ostringstream msg;
	msg << "Coherent::Send()	" << cmd;
	LogMessage(msg.str().c_str(), true);
	err = SendSerialCommand(port.c_str(),cmd.c_str(),"\r\n");
}



int Coherent::SetOpen(bool open){
	return 0;
}

int Coherent::GetOpen(bool& open){
	return 0;
}

int Coherent::Fire(double deltaT){
	return 0;
}
