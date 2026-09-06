#include "PhoneSystem.h"
#include <algorithm>
PhoneSystem::PhoneSystem(){
 items_={{"sofa_modern","Modern Sofa",ShopCategory::Furniture,1200},{"tv_55","55in TV",ShopCategory::Electronics,1800},{"pc_gaming","Gaming PC",ShopCategory::Gaming,2400},{"bed_lux","Luxury Bed",ShopCategory::Bedroom,3200},{"lamp_neon","Neon Lamp",ShopCategory::Lighting,450},{"grill_pro","Garden Grill",ShopCategory::GardenPool,900},{"safe_home","Home Safe",ShopCategory::Security,2600},{"cinema_home","Home Cinema",ShopCategory::Luxury,12500},{"garage_lift","Garage Lift",ShopCategory::Garage,6500},{"office_desk","Executive Desk",ShopCategory::Office,2100}};
 listings_.push_back({"car_sentinel","Sentinel Street",InternetCategory::Car,24000});
 listings_.push_back({"moto_city","City 600",InternetCategory::Motorcycle,9500});
 listings_.push_back({"boat_marlin","Marlin S8",InternetCategory::Boat,68000});
 listings_.push_back({"plane_skylet","Skylet S20",InternetCategory::Aircraft,210000});
 listings_.push_back({"house_suburb","Sight City House",InternetCategory::House,125000});
 listings_.push_back({"apt_downtown","Downtown Apartment",InternetCategory::Apartment,85000});
 listings_.push_back({"garage_west","West Garage",InternetCategory::Garage,45000});
 listings_.push_back({"business_taxi","Taxi Company",InternetCategory::Business,175000});
 listings_.push_back({"furniture_pack","Modern Furniture Pack",InternetCategory::Furniture,8500});
 contacts_={{"mechanic","Mechanic",true,true,true},{"taxi","Sight Taxi",true,true,true},{"ambulance","Emergency Medical",true,true,true},{"story_fix","Story Contact",true,false,true},{"crew","Online Crew",false,true,true}};
 heists_={{"harbor_score","Harbor Score",GameMode::Story,75000,2,2,true,"heist",true},{"city_bank","Sight City Bank Job",GameMode::Online,120000,3,3,true,"heist",true},{"quick_store","Store Take",GameMode::Story,12000,0,0,false,"",false},{"crew_cargo","Crew Cargo",GameMode::Online,45000,1,1,false,"",false}};
}
void PhoneSystem::toggle(){state_.open=!state_.open;if(!state_.open)state_.app=PhoneApp::Home;} void PhoneSystem::openApp(PhoneApp a){state_.open=true;state_.app=a;state_.selection=0;} void PhoneSystem::close(){state_.open=false;state_.app=PhoneApp::Home;}
std::vector<ShopItem> PhoneSystem::itemsIn(ShopCategory c)const{std::vector<ShopItem>o;for(const auto&i:items_)if(i.category==c)o.push_back(i);return o;}
std::vector<InternetListing> PhoneSystem::listingsIn(InternetCategory c)const{std::vector<InternetListing>o;for(const auto&i:listings_)if(i.category==c)o.push_back(i);return o;}
std::vector<ContactEntry> PhoneSystem::contactsFor(GameMode m)const{std::vector<ContactEntry>o;for(const auto&c:contacts_)if(c.unlocked&&((m==GameMode::Story&&c.story)||(m==GameMode::Online&&c.online)))o.push_back(c);return o;}
std::vector<HeistEntry> PhoneSystem::heistsFor(GameMode m)const{std::vector<HeistEntry>o;for(const auto&h:heists_)if(h.mode==m)o.push_back(h);return o;}
bool PhoneSystem::buy(const std::string&id,int&money,std::vector<std::string>&inv){auto it=std::find_if(items_.begin(),items_.end(),[&](const ShopItem&i){return i.id==id;});if(it==items_.end()||money<it->price)return false;money-=it->price;inv.push_back(id);return true;}
bool PhoneSystem::internetBuy(const std::string&id,int&money,std::vector<std::string>&inv){auto it=std::find_if(listings_.begin(),listings_.end(),[&](const InternetListing&i){return i.id==id;});if(it==listings_.end()||money<it->price)return false;money-=it->price;inv.push_back(id);return true;}
bool PhoneSystem::deposit(int a,int&cash,int&bank){if(a<=0||cash<a)return false;cash-=a;bank+=a;return true;} bool PhoneSystem::withdraw(int a,int&cash,int&bank){if(a<=0||bank<a)return false;bank-=a;cash+=a;return true;} bool PhoneSystem::takeLoan(int a,int&bank,int&debt){if(a<=0)return false;bank+=a;debt+=a;return true;}
bool PhoneSystem::requestVehicle(const std::string&id,const std::vector<OwnedVehicle>&g,std::string&d)const{auto it=std::find_if(g.begin(),g.end(),[&](const OwnedVehicle&v){return v.id==id;});if(it==g.end()||it->missionLocked)return false;d=it->id;return true;}
bool PhoneSystem::callContact(const std::string&id,GameMode m,std::string&r)const{for(const auto&c:contacts_)if(c.id==id&&c.unlocked&&((m==GameMode::Story&&c.story)||(m==GameMode::Online&&c.online))){r=c.name;return true;}return false;}
bool PhoneSystem::canStartHeist(const std::string&id,GameMode m,const std::vector<OwnedVehicle>&g,bool room,std::string&r)const{for(const auto&h:heists_)if(h.id==id&&h.mode==m){if(h.setupsDone<h.setupsRequired){r="Setups Required";return false;}if(h.finalRequiresHeistRoom&&!room){r="Heist Room Required";return false;}if(h.requiresVehicle){for(const auto&v:g)if(v.heistCompatible&&!v.missionLocked){r="Ready";return true;}r="Vehicle Required";return false;}r="Ready";return true;}r="Heist Unavailable";return false;}
