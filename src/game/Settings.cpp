#include "game/Settings.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

void GameSettings::resetControls() {
    lookSensitivity=1.0f;
    stickDeadzone=0.14f;
    invertCameraX=false;
    invertCameraY=false;
    vibration=true;
    aimAssist=AimAssistLevel::Normal;
    firstPersonEnabled=true;
    firstPersonOnFoot=true;
    firstPersonInVehicle=true;
    firstPersonFov=75.0f;
    vehicleIndicators=true;
    vehicleLights=true;
    speedUnit=SpeedUnit::Kmh;
}

bool GameSettings::save(const std::string& path) const {
    std::ofstream f(path.c_str(), std::ios::trunc);
    if(!f) return false;
    f << "lookSensitivity=" << lookSensitivity << "\n";
    f << "stickDeadzone=" << stickDeadzone << "\n";
    f << "invertCameraX=" << (invertCameraX?1:0) << "\n";
    f << "invertCameraY=" << (invertCameraY?1:0) << "\n";
    f << "vibration=" << (vibration?1:0) << "\n";
    f << "aimAssist=" << static_cast<int>(aimAssist) << "\n";
    f << "firstPersonEnabled=" << (firstPersonEnabled?1:0) << "\n";
    f << "firstPersonOnFoot=" << (firstPersonOnFoot?1:0) << "\n";
    f << "firstPersonInVehicle=" << (firstPersonInVehicle?1:0) << "\n";
    f << "firstPersonFov=" << firstPersonFov << "\n";
    f << "vehicleIndicators=" << (vehicleIndicators?1:0) << "\n";
    f << "vehicleLights=" << (vehicleLights?1:0) << "\n";
    f << "speedUnit=" << static_cast<int>(speedUnit) << "\n";
    return true;
}

bool GameSettings::load(const std::string& path) {
    std::ifstream f(path.c_str());
    if(!f) return false;
    std::string line;
    while(std::getline(f,line)) {
        const auto p=line.find('=');
        if(p==std::string::npos) continue;
        const std::string k=line.substr(0,p), v=line.substr(p+1);
        if(k=="lookSensitivity") lookSensitivity=std::strtof(v.c_str(),nullptr);
        else if(k=="stickDeadzone") stickDeadzone=std::strtof(v.c_str(),nullptr);
        else if(k=="invertCameraX") invertCameraX=std::atoi(v.c_str())!=0;
        else if(k=="invertCameraY") invertCameraY=std::atoi(v.c_str())!=0;
        else if(k=="vibration") vibration=std::atoi(v.c_str())!=0;
        else if(k=="firstPersonEnabled") firstPersonEnabled=std::atoi(v.c_str())!=0;
        else if(k=="firstPersonOnFoot") firstPersonOnFoot=std::atoi(v.c_str())!=0;
        else if(k=="firstPersonInVehicle") firstPersonInVehicle=std::atoi(v.c_str())!=0;
        else if(k=="firstPersonFov") firstPersonFov=std::strtof(v.c_str(),nullptr);
        else if(k=="vehicleIndicators") vehicleIndicators=std::atoi(v.c_str())!=0;
        else if(k=="vehicleLights") vehicleLights=std::atoi(v.c_str())!=0;
        else if(k=="speedUnit") speedUnit=std::atoi(v.c_str())==1?SpeedUnit::Mph:SpeedUnit::Kmh;
        else if(k=="aimAssist") {
            int n=std::atoi(v.c_str()); if(n<0)n=0; if(n>3)n=3;
            aimAssist=static_cast<AimAssistLevel>(n);
        }
    }
    if(stickDeadzone<0.0f) stickDeadzone=0.0f;
    if(stickDeadzone>0.45f) stickDeadzone=0.45f;
    if(lookSensitivity<0.25f) lookSensitivity=0.25f;
    if(lookSensitivity>3.0f) lookSensitivity=3.0f;
    if(firstPersonFov<55.0f) firstPersonFov=55.0f;
    if(firstPersonFov>95.0f) firstPersonFov=95.0f;
    return true;
}
