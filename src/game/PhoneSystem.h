#pragma once
#include <string>
#include <vector>
#include "GameMode.h"

enum class PhoneApp { Home, SightShop, Properties, Garage, Bank, Businesses, Missions, Map, Internet, Mechanic, Contacts, Heists };
enum class ShopCategory { Furniture, Electronics, Kitchen, Bedroom, Bathroom, Lighting, Decorations, Gaming, GardenPool, Garage, Security, Luxury, Mansion, Office, Other };
enum class InternetCategory { Car, Motorcycle, Boat, Aircraft, House, Apartment, Garage, Business, Furniture };
struct ShopItem { std::string id,name; ShopCategory category; int price; };
struct InternetListing { std::string id,name; InternetCategory category; int price; };
struct OwnedVehicle { std::string id,name; bool heistCompatible=false; bool businessVehicle=false; bool missionLocked=false; };
struct ContactEntry { std::string id,name; bool story=true, online=true, unlocked=true; };
struct HeistEntry { std::string id,name; GameMode mode; int reward; int setupsRequired; int setupsDone; bool requiresVehicle; std::string vehicleTag; bool finalRequiresHeistRoom; };
struct PhoneState { bool open=false; PhoneApp app=PhoneApp::Home; int selection=0; };
class PhoneSystem {
public:
 PhoneSystem();
 void toggle(); void openApp(PhoneApp app); void close();
 const PhoneState& state() const{return state_;}
 const std::vector<ShopItem>& shopItems()const{return items_;}
 const std::vector<InternetListing>& internetListings()const{return listings_;}
 std::vector<ShopItem> itemsIn(ShopCategory c)const;
 std::vector<InternetListing> listingsIn(InternetCategory c)const;
 std::vector<ContactEntry> contactsFor(GameMode mode)const;
 std::vector<HeistEntry> heistsFor(GameMode mode)const;
 bool buy(const std::string&id,int& bank,std::vector<std::string>& inventory);
 bool internetBuy(const std::string&id,int& bank,std::vector<std::string>& inventory);
 bool deposit(int amount,int& cash,int& bank); bool withdraw(int amount,int& cash,int& bank);
 bool takeLoan(int amount,int& bank,int& debt);
 bool requestVehicle(const std::string&id,const std::vector<OwnedVehicle>& garage,std::string& delivered)const;
 bool callContact(const std::string&id,GameMode mode,std::string& result)const;
 bool canStartHeist(const std::string&id,GameMode mode,const std::vector<OwnedVehicle>& garage,bool hasHeistRoom,std::string& reason)const;
private:
 PhoneState state_; std::vector<ShopItem> items_; std::vector<InternetListing> listings_; std::vector<ContactEntry> contacts_; std::vector<HeistEntry> heists_;
};
