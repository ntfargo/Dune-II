#pragma once

class cBarracks : public cAbstractStructure {
private:


public:
    cBarracks();

    ~cBarracks();

    // overloaded functions    
    void thinkFast() override;

    void thinkSlow() override {}

    void think_animation() override;

    void think_guard() override;

    void startAnimating() override {}

    void draw() override { drawWithShadow(); }

    int getType() const override;
};

