#pragma once
class WantedSystem {
public:
    int level{0}; float heat{0};
    void addHeat(float amount); void reduceHeat(float amount); void clear();
    void update(float dt,bool policeCanSeePlayer);
};
