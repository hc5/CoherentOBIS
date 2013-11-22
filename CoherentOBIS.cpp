
#ifdef WIN32
#include <windows.h>
#define snprintf _snprintf 
#endif

#include "../../MMDevice/MMDevice.h"
#include "CoherentOBIS.h"
#include <string>
#include <math.h>
#include "../../MMDevice/ModuleInterface.h"
#include "../../MMDevice/DeviceUtils.h"
#include <sstream>



const char* DEVICE_NAME = "Coherent OBIS";

MODULE_API void InitializeModuleData(){
	AddAvailableDeviceName(DEVICE_NAME, "Coherent OBIS Laser");

}

MODULE_API MM::Device* CreateDevice(const char * name){
	if(!name) return 0;
	if(strcmp(name, DEVICE_NAME) == 0){
		return new CoherentOBIS;
	}
	return 0;
}

MODULE_API void DeleteDevice(MM::Device *pDevice){
	delete pDevice;
}

CoherentOBIS::CoherentOBIS():
	err(0),
	initialized(0),
	maxPow(0)
{
	name_ = DEVICE_NAME;
	CreateProperty(MM::g_Keyword_Name, 	DEVICE_NAME,MM::String, true);
	CreateProperty(MM::g_Keyword_Description, "CoherentOBIS OBIS Laser", MM::String, true);
	CPropertyAction * pAct = new CPropertyAction(this, &CoherentOBIS::OnPort);
	CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String,false, pAct, true);
	EnableDelay();
	UpdateStatus();
	
}


void CoherentOBIS::GenerateProperties(){
	CPropertyAction* pAct = new CPropertyAction(this, &CoherentOBIS::OnState);
	CreateProperty(MM::g_Keyword_State, "0", MM::Integer, false, pAct);
	AddAllowedValue(MM::g_Keyword_State, "0");
	AddAllowedValue(MM::g_Keyword_State, "1");

	CPropertyActionEx * powerAct = new CPropertyActionEx(this, &CoherentOBIS::OnPowerSet, 0);
	CreateProperty("Power level", "0", MM::Float, false, powerAct);
	
	// read only

	pAct = new CPropertyAction(this, &CoherentOBIS::OnCurrentLevel);
	CreateProperty("Current Level", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &CoherentOBIS::OnDiodeTemp);
	CreateProperty("Diode Temp", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &CoherentOBIS::OnBPTemp);
	CreateProperty("Base Plate Temp", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &CoherentOBIS::OnMaxPower);
	CreateProperty("Max Power Level", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &CoherentOBIS::OnDTempLow);
	CreateProperty("Diode Temp Lower Limit", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &CoherentOBIS::OnDTempHigh);
	CreateProperty("Diode Temp Upper Limit", "", MM::Float, true, pAct);
	
	pAct = new CPropertyAction(this, &CoherentOBIS::OnBPTempLow);
	CreateProperty("Base Plate Temp Lower Limit", "", MM::Float, true, pAct);

	pAct = new CPropertyAction(this, &CoherentOBIS::OnBPTempHigh);
	CreateProperty("Base Plate Temp Upper Limit", "", MM::Float, true, pAct);

}

CoherentOBIS::~CoherentOBIS(){
	Shutdown();
}

int CoherentOBIS::Initialize(){
	GenerateProperties();
	return 0;
}



bool CoherentOBIS::Busy(){
	MM::MMTime interval = GetCurrentMMTime() - changedTime_;
	MM::MMTime delay(GetDelayMs()*1000.0);
	if (interval < delay)
		return true;
	else
		return false;
}

int CoherentOBIS::Shutdown(){

	return 0;
}

void CoherentOBIS::GetName(char * name) const
{
	CDeviceUtils::CopyLimitedString(name, name_.c_str());
}

void CoherentOBIS::SetOn(long on){
	std::string cmd = "SOUR:AM:STAT ";
	cmd += on == 0?"OFF":"ON";
	Set(cmd);
}

long CoherentOBIS::GetOn(){
	string res = query("SOUR:AM:STAT?");
	if(res == "ON")
		return 1;
	else
		return 0;
}

void CoherentOBIS::SetPowerLevel(double d){
	string cmd = "SOUR:POW:LEV:IMM:AMPL ";
	std::ostringstream strs;
	strs << cmd << d;
	Set(strs.str());
}

double CoherentOBIS::GetPowerLevel(){
	return Get("SOUR:POW:LEV?");
}

double CoherentOBIS::Get(string cmd){
	string res = query(cmd);
	return ::atof(res.c_str());
}
int CoherentOBIS::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
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

int CoherentOBIS::OnState(MM::PropertyBase* pProp, MM::ActionType eAct){
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

int CoherentOBIS::OnPowerSet(MM::PropertyBase* pProp, MM::ActionType eAct, long power){
	double level;
	if(eAct == MM::BeforeGet){
		level = GetPowerLevel();
		pProp->Set(level);
	}
	else if(eAct == MM::AfterSet){
		pProp->Get(level);
		SetPowerLevel(level);
		pProp->Set(GetPowerLevel());
	}
	return 0;
}


int CoherentOBIS::OnCurrentLevel(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:POW:CURR?"));

	}
	return 0;
}
int CoherentOBIS::OnDiodeTemp(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:DIOD? C"));
	}
	return 0;
}
int CoherentOBIS::OnBPTemp(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:BAS? C"));
	}
	return 0;
}
int CoherentOBIS::OnMaxPower(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:POW:LIM:HIGH?"));
		pProp->Get(maxPow);
		SetPropertyLimits("Power Level",0,maxPow);
	}
	return 0;
}
int CoherentOBIS::OnDTempLow(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:PROT:DIOD:LOW? C"));
	}
	return 0;
}
int CoherentOBIS::OnDTempHigh(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:PROT:DIOD:HIGH? C"));
	}
	return 0;
}
int CoherentOBIS::OnBPTempLow(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:PROT:BAS:LOW? C"));
	}
	return 0;
}
int CoherentOBIS::OnBPTempHigh(MM::PropertyBase* pProp, MM::ActionType eAct){
	if(eAct == MM::BeforeGet){
		pProp->Set(Get("SOUR:TEMP:PROT:BAS:HIGH? C"));
	}
	return 0;
}
std::string CoherentOBIS::query(string q){

	std::string result;
	std::stringstream msg;
	err = (PurgeComPort(port.c_str()));
	Send(q);
	Read();
	string buf = buf_string_;
	Read();
	return buf;

}

void CoherentOBIS::Set(string cmd){
	err = (PurgeComPort(port.c_str()));
	Send(cmd);
	Read();
}

int CoherentOBIS::Read(){
	int ret = GetSerialAnswer(port.c_str(),"\r", buf_string_);
	if(ret!= DEVICE_OK)
		return ret;
	std::ostringstream msg;
	msg <<"CoherentOBIS::Read()		" << buf_string_;
	LogMessage(msg.str().c_str(), true);
	return DEVICE_OK;
}

void CoherentOBIS::Send(string cmd){
	std::ostringstream msg;
	msg << "CoherentOBIS::Send()	" << cmd;
	LogMessage(msg.str().c_str(), true);
	err = SendSerialCommand(port.c_str(),cmd.c_str(),"\r");
}



int CoherentOBIS::SetOpen(bool open){
	return 0;
}

int CoherentOBIS::GetOpen(bool& open){
	return 0;
}

int CoherentOBIS::Fire(double deltaT){
	return 0;
}
